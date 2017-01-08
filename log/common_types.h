#pragma once

#include <fstream>
#include <memory>
#include <stdexcept>

namespace sl {

enum class Level {
  debug,
  info,
  warning,
  error,
  critical
};

namespace detail {

class LoggerException : public std::exception {
public:
  LoggerException(const std::string& message) : m_message(message) {}
  virtual const char* what() const noexcept override {
    return m_message.c_str();
  }
private:
  std::string m_message;
};


using OstreamPtr = std::shared_ptr<std::ostream>;

class RotationWatcherHandler {
public:
  virtual int64_t clearNeeded(int64_t spaceToClear) = 0;
  virtual bool nextFile() = 0;
};

}
}
