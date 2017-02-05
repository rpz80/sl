#pragma once

#include <sstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <iostream>
#include <cstdint>

#include <sm/shared_mutex.h>
#include <log/common_types.h>
#include <log/format.h>
#include <log/exception.h>
#include <log/log_files_manager.h>

namespace sl {

enum class Level {
  debug,
  info,
  warning,
  error,
  critical
};

namespace detail {

void writeLogData(std::stringstream& messageStream, 
                  Level level,
                  const std::string& timeFormat);
}

class Logger {
  struct Sink {
    detail::LogFilesManagerPtr fileManager;
    Level level;
    bool duplicateToStdout;
    std::unique_ptr<std::mutex> mutex;

    Sink() : level(Level::error),
             duplicateToStdout(false) {}

    Sink(Level level, 
         detail::LogFilesManagerPtr fileManager, 
         bool duplicateToStdout) :
      fileManager(std::move(fileManager)),
      level(level),
      duplicateToStdout(duplicateToStdout),
      mutex(new std::mutex) {}
  };

  using SinkMap = std::unordered_map<int, Sink>;
  using SinkMapConstIterator = SinkMap::const_iterator;
  using SinkMapIterator = SinkMap::iterator;

public:
  Logger();
  void setLevel(int sinkId, Level level);
  void setDefaultLevel(Level level);

  Level getLevel(int sinkId) const;
  Level getDefaultLevel() const;

  void addSink(int sinkId, 
               const std::string& logDir,
               const std::string& fileNamePattern,
               Level level,
               int64_t totalLimit,
               int64_t fileLimit,
               bool duplicateToStdout = false);

  bool hasSink(int sinkId) const;

  void setDefaultSink(const std::string& logDir, 
                      const std::string& fileNamePattern, 
                      Level level, 
                      int64_t totalLimit, 
                      int64_t fileLimit,
                      bool duplicateToStdout = false);

  bool hasDefaultSink() const;

  template<typename... Args>
  void log(int sinkId, Level level, 
           const char* formatString, 
           Args&&... args) {
    sm::shared_lock<sm::shared_mutex> lock(m_sinksMutex);
    auto sinkIt = getSinkById(sinkId);
    writeToSink(sinkIt->second, 
                level, 
                formatString, 
                std::forward<Args>(args)...);
  }


  template<typename... Args>
  void log(Level level, 
           const char* formatString, 
           Args&&... args) {
    sm::shared_lock<sm::shared_mutex> lock(m_defaultSinkMutex);
    detail::throwLoggerExceptionIfNot(
        static_cast<bool>(m_defaultSink.fileManager), 
        fmt("% No default sink", __FUNCTION__));
    writeToSink(m_defaultSink, 
                level, 
                formatString,
                std::forward<Args>(args)...);
  }

  void setTimeFormat(const std::string& timeFormatStr);
  static Logger& getLogger();

protected:
  std::string getFileNamePattern(int sinkId) const;
  std::string getDefaultFileNamePattern() const;
  std::string getTimeFormat() const;

private:
  SinkMapConstIterator getSinkById(int sinkId) const;
  SinkMapIterator getSinkById(int sinkId);
  void checkSinkWithPattern(const std::string& fileName) const;

  template<typename... Args>
  void writeToSink(Sink& sink, 
                   Level level,
                   const char* formatString, 
                   Args&&... args) {
    std::stringstream messageStream;
    detail::writeLogData(messageStream, level, m_timeFormat);
    detail::fmt(messageStream, 
                formatString, 
                std::forward<Args>(args)...);
    messageStream << std::endl << std::endl;
    std::lock_guard<std::mutex>(*sink.mutex);
    auto outString = messageStream.str();
    sink.fileManager->write(outString.data(), outString.size());
    if (sink.duplicateToStdout) {
      std::cout << outString;
    }
  }

private:
  Sink m_defaultSink;
  std::unordered_map<int, Sink> m_sinks;
  mutable sm::shared_mutex m_defaultSinkMutex;
  mutable sm::shared_mutex m_sinksMutex;
  std::string m_timeFormat;
};

}

#define ___LOG_EXPAND(...) __VA_ARGS__

#define LOG_S(___sinkId, ___level, ___formatStr, ...) \
  do { \
    if (sl::Logger::getLogger().getLevel(___sinkId) <= ___level) { \
      sl::Logger::getLogger().log(___sinkId,  \
                                  ___level,  \
                                  ___formatStr, \
                                  ___LOG_EXPAND(__VA_ARGS__)); \
    } \
  } while(0)
  

#define LOG(___level, ___formatStr, ...) \
  do { \
    if (sl::Logger::getLogger().getDefaultLevel() <= ___level) { \
      sl::Logger::getLogger().log(___level,  \
                                  ___formatStr, \
                                  ___LOG_EXPAND(__VA_ARGS__)); \
    } \
  } while(0)

