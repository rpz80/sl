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

}
}
