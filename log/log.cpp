#include <stdexcept>
#include <cassert>
#include <shared_mutex>
#include "log.h"

namespace sl {

namespace aux {
class LoggerException : public std::exception {
public:
  LoggerException(const std::string& message) : m_message(message) {}
  virtual const char* what() const noexcept override {
    return m_message.c_str();
  }
private:
  std::string m_message;
};

void assertThrow(bool expr, const std::string& message) {
  //assert(expr);
  if (!expr) {
    throw LoggerException(message);
  }
}

Logger::OstreamPtr tryOpenFile(const std::string& fileName) {
  auto out = std::make_shared<std::ofstream>(fileName);
  aux::assertThrow(static_cast<bool>(out), 
                   fmt("% Failed to open file %", 
                       __FUNCTION__,
                       fileName));
  return out;
}

}

Logger::Logger() {}

Logger::SinkMapConstIterator Logger::getSinkById(int sinkId) const {
  return const_cast<Logger&>(*this).getSinkById(sinkId);
}

Logger::SinkMapIterator Logger::getSinkById(int sinkId) {
  auto sinkIt = m_sinks.find(sinkId);
  aux::assertThrow(sinkIt != m_sinks.cend(), 
                   fmt("sinkId % not found", sinkId));
  return sinkIt;
}

void Logger::setDefaultSink(Level level, 
                            const std::string& fileName, 
                            const OstreamPtr& sinkStream, 
                            bool duplicateToStdout) {
  aux::assertThrow(
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
  aux::assertThrow(
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
  aux::assertThrow(
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
  aux::assertThrow(static_cast<bool>(m_defaultSink.out), 
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
  aux::assertThrow(static_cast<bool>(m_defaultSink.out), 
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
  aux::assertThrow(static_cast<bool>(m_defaultSink.out), 
                   "Default sink not set");
  return m_defaultSink.fileName;
}

void Logger::addSink(int sinkId, 
                     const std::string& fileName,
                     Level level, 
                     bool duplicateToStdout) {
  addSink(sinkId, level, fileName, 
          aux::tryOpenFile(fileName),
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
                 aux::tryOpenFile(fileName),
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
