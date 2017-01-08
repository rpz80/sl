#include <log/utils.h>

namespace sl {
namespace detail {
namespace errh {

void assertThrow(bool expr, const std::string& message) {
  //assert(expr);
  if (!expr) {
    throw LoggerException(message);
  }
}

}
}
}

