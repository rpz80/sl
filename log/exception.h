#pragma once

#include <stdexcept>

namespace sl {
namespace detail {

class LoggerException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class UtilsException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

template<typename Exception>
void throwIfNot(bool value, const std::string& message) {
  if (!value)
    throw Exception(message);
}

void throwLoggerExceptionIfNot(bool value, const std::string& message) {
  throwIfNot<LoggerException>(value, message);
}

}
}
