#pragma once

#include <sstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <sm/shared_mutex.h>

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

void assertThrow(bool expr, const std::string& message); 
std::string 

}

class Logger {
  struct Sink {
    Level level;
    std::string path;
    std::string fileNamePattern;
    OstreamPtr out;
    bool duplicateToStdout;
    std::unique_ptr<std::mutex> mutex;

    Sink() : level(Level::error), 
             duplicateToStdout(false) {}

    Sink(Level level, 
         const std::string& path,
         const std::string& fileNamePattern,
         OstreamPtr out, 
         bool duplicateToStdout) :
      level(level),
      path(path),
      fileNamePattern(fileNamePattern),
      out(std::move(out)),
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
               const std::string& fileNamePattern,
               Level level,
               bool duplicateToStdout = false);

  void removeSink(int sinkId);
  bool hasSink(int sinkId) const;

  void setDefaultSink(const std::string& fileNamePattern,
                      Level level,
                      bool duplicateToStdout = false);

  void removeDefaultSink();
  bool hasDefaultSink() const;

  template<typename... Args>
  void log(int sinkId, Level level, 
           const char* formatString, 
           Args&&... args) {
    std::shared_lock<sm::shared_mutex> lock(m_sinksMutex);
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
    std::shared_lock<sm::shared_mutex> lock(m_defaultSinkMutex);
    detail::assertThrow(static_cast<bool>(m_defaultSink.out),
                        fmt("% No default sink", __FUNCTION__));
    writeToSink(m_defaultSink, 
                level, 
                formatString,
                std::forward<Args>(args)...);
  }

  void setTimeFormat(const std::string& timeFormatStr);
  std::string getTimeFormat() const;

  static Logger& getLogger();

protected:
  void setDefaultSink(Level level,
                      const std::string& fileName,
                      const OstreamPtr& sinkStream,
                      bool duplicateToStdout);

  void addSink(int sinkId, 
               Level level,
               const std::string& fileName,
               const OstreamPtr& sinkStream,
               bool duplicateToStdout);

  std::string getFileNamePattern(int sinkId) const;
  std::string getDefaultFileNamePattern() const;


private:
  SinkMapConstIterator getSinkById(int sinkId) const;
  SinkMapIterator getSinkById(int sinkId);

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
    *sink.out << messageStream.str();
    if (sink.duplicateToStdout) {
      std::cout << messageStream.str();
    }
  }

private:
  Sink m_defaultSink;
  std::unordered_map<int, Sink> m_sinks;
  mutable sm::shared_mutex m_defaultSinkMutex;
  mutable sm::shared_mutex m_sinksMutex;
  std::string m_timeFormat;
};

namespace detail {
Logger::OstreamPtr tryOpenFile(const std::string& fileName); 
}

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

