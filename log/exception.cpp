#include <log/exception.h>

namespace sl {
namespace detail {

LoggerException(const std::string& message) : m_message(message) {}

virtual const char* what() const noexcept override {
  return m_message.c_str();
}

}
}
