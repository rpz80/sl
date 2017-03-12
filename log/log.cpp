#include <stdexcept>
#include <cassert>
#include <thread>
#include <iomanip>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <sys/time.h>
#endif

#include "log.h"

namespace sl {

using namespace detail;

Logger::Logger() 
    : m_timeFormat("%Y-%m-%d %H:%M:%S") {
}

Logger::SinkMapConstIterator Logger::getSinkById(int sinkId) const {
  return const_cast<Logger&>(*this).getSinkById(sinkId);
}

Logger::SinkMapIterator Logger::getSinkById(int sinkId) {
  auto sinkIt = m_sinks.find(sinkId);
  detail::throwLoggerExceptionIfNot(sinkIt != m_sinks.cend(),
                      fmt("sinkId % not found", sinkId));
  return sinkIt;
}

void Logger::setDefaultSink(const std::string& logDir, 
                    const std::string& fileNamePattern, 
                    Level level, 
                    int64_t totalLimit, 
                    int64_t fileLimit,
                    bool duplicateToStdout) {
  addSink(kDefaultSinkId, logDir, fileNamePattern, level, 
    totalLimit, fileLimit, duplicateToStdout);
}

void Logger::addSink(int sinkId, 
                     const std::string& logDir, 
                     const std::string& fileNamePattern, 
                     Level level, 
                     int64_t totalLimit, 
                     int64_t fileLimit,
                     bool duplicateToStdout) {
  std::lock_guard<sm::shared_mutex> lock(m_sinksMutex);
  if (m_sinks.find(sinkId) != m_sinks.cend()) {
    throw std::runtime_error(
        fmt("% sink with this id (%) already exists", 
            __FUNCTION__, 
            sinkId));
  }
  checkSinkWithPattern(fileNamePattern);
  bool emplaceResult = 
      m_sinks.emplace(
          sinkId, 
          Sink(level, 
               detail::LogFilesManagerPtr( 
                  new detail::LogFilesManager(
                      totalLimit, 
                      fileLimit,
                      FileEntryCatalogPtr(new FileEntryCatalog(
                          new FileEntryFactory(),
                          logDir,
                          fileNamePattern)))), 
               duplicateToStdout)).second;
  detail::throwLoggerExceptionIfNot(
      emplaceResult, 
      fmt("% Emplace failed. fileName pattern = %. sinkId = %",
          __FUNCTION__,
          fileNamePattern, 
          sinkId));
}

void Logger::checkSinkWithPattern(const std::string& fileNamePattern) const {
  for (auto sinkIt = m_sinks.cbegin(); sinkIt != m_sinks.cend(); ++ sinkIt) {
    if (sinkIt->second.fileManager->baseName() == fileNamePattern) {
      throw std::runtime_error(
          fmt("sink with this file name pattern (%) already exists", 
              fileNamePattern));
    }
  }
}

void Logger::setLevel(int sinkId, Level level) {
  std::lock_guard<sm::shared_mutex> lock(m_sinksMutex);
  auto sinkIt = getSinkById(sinkId);
  sinkIt->second.level = level;
}

void Logger::setDefaultLevel(Level level) {
  setLevel(kDefaultSinkId, level);
}

Level Logger::getLevel(int sinkId) const {
  sm::shared_lock<sm::shared_mutex> lock(m_sinksMutex);
  auto sinkIt = getSinkById(sinkId);
  return sinkIt->second.level;
}

Level Logger::getDefaultLevel() const {
  return getLevel(kDefaultSinkId);
}

std::string Logger::getFileNamePattern(int sinkId) const {
  sm::shared_lock<sm::shared_mutex> lock(m_sinksMutex);
  auto sinkIt = getSinkById(sinkId);
  return sinkIt->second.fileManager->baseName();
}

std::string Logger::getDefaultFileNamePattern() const {
  return getFileNamePattern(kDefaultSinkId);
}

bool Logger::hasSink(int sinkId) const {
  sm::shared_lock<sm::shared_mutex> lock(m_sinksMutex);
  return m_sinks.find(sinkId) != m_sinks.cend();
}

bool Logger::hasDefaultSink() const {
  return hasSink(kDefaultSinkId);
}

void Logger::setTimeFormat(const std::string& timeFormatStr) {
  std::lock_guard<sm::shared_mutex> lock(m_sinksMutex);
  m_timeFormat = timeFormatStr;
}

std::string Logger::getTimeFormat() const {
  std::lock_guard<sm::shared_mutex> lock(m_sinksMutex);
  return m_timeFormat;
}

Logger& Logger::getLogger() {
  static Logger logger;
  return logger;
}

namespace detail {

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
    default: detail::throwLoggerExceptionIfNot(false, sl::fmt("% Unknown level: %",
                                                __FUNCTION__,
                                                (int)level));
  }
}

void writeTime(std::stringstream& messageStream, 
               const std::string& timeFormat) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  struct timeval tv;
  time_t timeNowSec;
  struct tm *timeLocal;
  char buf[64];

  gettimeofday(&tv, NULL);
  timeNowSec = tv.tv_sec;
  timeLocal = localtime(&timeNowSec);
  strftime(buf, sizeof(buf), timeFormat.c_str(), timeLocal);
  messageStream << buf << "." << std::setw(3) << std::setfill('0')
                << (tv.tv_usec / 1000) << " ";
#elif defined (_WIN32)
#endif
}

void writeLogData(std::stringstream& messageStream, 
                  Level level,
                  const std::string& timeFormat) {
  writeTime(messageStream, timeFormat);
  writeLevel(messageStream, level);
  writeThreadId(messageStream);
}

} // detail
} // sl
