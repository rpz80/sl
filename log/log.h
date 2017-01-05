#pragma once

#include <sstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include <fstream>
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

void printTillSpecial(std::stringstream& out, 
                      const char** formatString);

template<typename... Args>
std::string fmt(std::stringstream& out,
                const char* formatString,
                Args&&... args); 

std::string fmt(std::stringstream& out,
                const char* formatString); 

template<typename Head, typename... Tail>
std::string fmt(std::stringstream& out,
                const char* formatString,
                Head&& head,
                Tail&&... tail) {
  printTillSpecial(out, &formatString);
  if (*formatString) {
    out << std::forward<Head>(head);
    ++formatString;
    return fmt(out, formatString, std::forward<Tail>(tail)...);
  } else {
    return out.str();
  }
}

void writeLogData(std::stringstream& messageStream, Level level);

class LoggerException : public std::exception {
public:
  LoggerException(const std::string& message) : m_message(message) {}
  virtual const char* what() const noexcept override {
    return m_message.c_str();
  }
private:
  std::string m_message;
};

void assertThrow(bool expr, const std::string& message); 

}

template<typename... Args>
std::string fmt(const char* formatString, Args&&... args) {
  std::stringstream out;
  return detail::fmt(out, formatString, std::forward<Args>(args)...);
}

class Logger {
public:
  using OstreamPtr = std::shared_ptr<std::ostream>;

private:
  struct Sink {
    Level level;
    std::string fileName;
    OstreamPtr out;
    bool duplicateToStdout;
    std::unique_ptr<std::mutex> mutex;

    Sink() : level(Level::error), 
             duplicateToStdout(false) {}

    Sink(Level level, 
         const std::string& fileName,
         OstreamPtr out, 
         bool duplicateToStdout) :
      level(level),
      fileName(fileName),
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

  std::string getFileName(int sinkId) const;
  std::string getDefaultFileName() const;

  void addSink(int sinkId, 
               const std::string& fileName,
               Level level,
               bool duplicateToStdout = false);

  void removeSink(int sinkId);
  bool hasSink(int sinkId) const;

  void setDefaultSink(const std::string& fileName,
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

private:
  SinkMapConstIterator getSinkById(int sinkId) const;
  SinkMapIterator getSinkById(int sinkId);

  template<typename... Args>
  void writeToSink(Sink& sink, 
                   Level level,
                   const char* formatString, 
                   Args&&... args) {
    std::stringstream messageStream;
    detail::writeLogData(messageStream, level);
    detail::fmt(messageStream, 
                formatString, 
                std::forward<Args>(args)...);
    messageStream << std::endl;
    std::lock_guard<std::mutex>(*sink.mutex);
    *sink.out << messageStream.str();
  }

private:
  Sink m_defaultSink;
  std::unordered_map<int, Sink> m_sinks;
  mutable sm::shared_mutex m_defaultSinkMutex;
  mutable sm::shared_mutex m_sinksMutex;
};

namespace detail {
Logger::OstreamPtr tryOpenFile(const std::string& fileName); 
}

}
