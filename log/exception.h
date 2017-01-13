#pragma once

#include <stdexcept>

namespace sl {
namespace detail {

class LoggerException : public std::exception {
public:
  LoggerException(const std::string& message);
  virtual const char* what() const noexcept override; 

private:
  std::string m_message;
};

class UtilsException : public std::runtime_exception {
public:
  using std::runtime_exception::runtime_exception;
};

}
}
