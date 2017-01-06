#include <stdexcept>
#include <cassert>
#include <thread>
#include <iomanip>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <sys/time.h>
#endif

#include "log.h"

namespace sl {

Logger::Logger() {}

Logger::SinkMapConstIterator Logger::getSinkById(int sinkId) const {
  return const_cast<Logger&>(*this).getSinkById(sinkId);
}

Logger::SinkMapIterator Logger::getSinkById(int sinkId) {
  auto sinkIt = m_sinks.find(sinkId);
  detail::assertThrow(sinkIt != m_sinks.cend(), 
                      fmt("sinkId % not found", sinkId));
  return sinkIt;
}

void Logger::setDefaultSink(Level level, 
                            const std::string& fileName, 
                            const OstreamPtr& sinkStream, 
                            bool duplicateToStdout) {
  detail::assertThrow(
      static_cast<bool>(sinkStream), 
      fmt("% sink stream is null. fileName = %", 
          __FUNCTION__,
          fileName));
  std::lock_guard<sm::shared_mutex> lock(m_defaultSinkMutex);
  m_defaultSink = Sink(level, 
                       fileName,
                       sinkStream, 
                       duplicateToStdout);
}

void Logger::addSink(int sinkId, 
                     Level level, 
                     const std::string& fileName, 
                     const OstreamPtr& sinkStream, 
                     bool duplicateToStdout) {
  detail::assertThrow(
      static_cast<bool>(sinkStream), 
      fmt("% sink stream is null. fileName = %. sinkId = %",
          __FUNCTION__,
          fileName, 
          sinkId));
  std::lock_guard<sm::shared_mutex> lock(m_sinksMutex);
  if (m_sinks.find(sinkId) != m_sinks.cend()) {
    throw std::runtime_error(
        fmt("% sink with this id (%) already exists", 
            __FUNCTION__, 
            sinkId));
  }
  bool emplaceResult = 
      m_sinks.emplace(sinkId, 
                      Sink(level, 
                           fileName, 
                           sinkStream, 
                           duplicateToStdout)).second;
  detail::assertThrow(
      emplaceResult, 
      fmt("% Emplace failed. fileName = %. sinkId = %",
          __FUNCTION__,
          fileName, 
          sinkId));
}

void Logger::setLevel(int sinkId, Level level) {
  std::lock_guard<sm::shared_mutex> lock(m_sinksMutex);
  auto sinkIt = getSinkById(sinkId);
  sinkIt->second.level = level;
}

void Logger::setDefaultLevel(Level level) {
  std::lock_guard<sm::shared_mutex> lock(m_defaultSinkMutex);
  detail::assertThrow(static_cast<bool>(m_defaultSink.out), 
                      "Default sink not set");
  m_defaultSink.level = level;
}

Level Logger::getLevel(int sinkId) const {
  std::shared_lock<sm::shared_mutex> lock(m_sinksMutex);
  auto sinkIt = getSinkById(sinkId);
  return sinkIt->second.level;
}

Level Logger::getDefaultLevel() const {
  std::shared_lock<sm::shared_mutex> lock(m_defaultSinkMutex);
  detail::assertThrow(static_cast<bool>(m_defaultSink.out), 
                      "Default sink not set");
  return m_defaultSink.level;
}

std::string Logger::getFileName(int sinkId) const {
  std::shared_lock<sm::shared_mutex> lock(m_sinksMutex);
  auto sinkIt = getSinkById(sinkId);
  return sinkIt->second.fileName;
}

std::string Logger::getDefaultFileName() const {
  std::shared_lock<sm::shared_mutex> lock(m_defaultSinkMutex);
  detail::assertThrow(static_cast<bool>(m_defaultSink.out), 
                      "Default sink not set");
  return m_defaultSink.fileName;
}

void Logger::addSink(int sinkId, 
                     const std::string& fileName,
                     Level level, 
                     bool duplicateToStdout) {
  addSink(sinkId, level, fileName, 
          detail::tryOpenFile(fileName),
          duplicateToStdout);
}

void Logger::removeSink(int sinkId) {
  std::lock_guard<sm::shared_mutex> lock(m_sinksMutex);
  m_sinks.erase(sinkId);
}

bool Logger::hasSink(int sinkId) const {
  std::shared_lock<sm::shared_mutex> lock(m_sinksMutex);
  return m_sinks.find(sinkId) != m_sinks.cend();
}

void Logger::setDefaultSink(const std::string& fileName, 
                            Level level,
                            bool duplicateToStdout) {
  setDefaultSink(level, fileName, 
                 detail::tryOpenFile(fileName),
                 duplicateToStdout);
}

void Logger::removeDefaultSink() {
  std::lock_guard<sm::shared_mutex> lock(m_defaultSinkMutex);
  m_defaultSink.out.reset();
}

bool Logger::hasDefaultSink() const {
  std::shared_lock<sm::shared_mutex> lock(m_defaultSinkMutex);
  return static_cast<bool>(m_defaultSink.out);
}

namespace detail {

Logger::OstreamPtr tryOpenFile(const std::string& fileName) {
  auto out = std::make_shared<std::ofstream>(fileName);
  detail::assertThrow(static_cast<bool>(out), 
                      sl::fmt("% Failed to open file %", 
                              __FUNCTION__,
                              fileName));
  return out;
}

void assertThrow(bool expr, const std::string& message) {
  //assert(expr);
  if (!expr) {
    throw LoggerException(message);
  }
}

void writeThreadId(std::stringstream& messageStream) {
  messageStream << std::hex << std::this_thread::get_id() << " ";
}

void writeLevel(std::stringstream& messageStream, Level level) {
  switch (level) {
    case Level::debug:    messageStream << "   DEBUG ";  break;
    case Level::info:     messageStream << "    INFO ";  break;
    case Level::warning:  messageStream << " WARNING ";  break;
    case Level::error:    messageStream << "   ERROR ";  break;
    case Level::critical: messageStream << "CRITICAL ";  break;
    default: detail::assertThrow(false, sl::fmt("% Unknown level: %",
                                                __FUNCTION__,
                                                (int)level));
  }
}

void writeTime(std::stringstream& messageStream) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  struct timeval tv;
  time_t timeNowSec;
  struct tm *timeLocal;
  char buf[64];

  gettimeofday(&tv, NULL);
  timeNowSec = tv.tv_sec;
  timeLocal = localtime(&timeNowSec);
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", timeLocal);
  messageStream << buf << "." << std::setw(3) << std::setfill('0')
                << (tv.tv_usec / 1000) << " ";
#elif defined (_WIN32)
#endif
}

void writeLogData(std::stringstream& messageStream, Level level) {
  writeTime(messageStream);
  writeLevel(messageStream, level);
  writeThreadId(messageStream);
}

void printTillSpecial(std::stringstream& out, 
                      const char** formatString) {
  for (; **formatString; ++*formatString) {
    if (**formatString == '\\') {
      if (*(*formatString + 1) && *(*formatString + 1) == '%') {
        ++*formatString;
        out << '%';
      } else {
        out << **formatString;
      }
    } else if (**formatString == '%') {
      break;
    } else {
      out << **formatString;
    }
  }
}

std::string fmt(std::stringstream& out,
                const char* formatString) {
  for (; *formatString; ++formatString) {
    out << *formatString;
  }
  return out.str();
}

}
}
