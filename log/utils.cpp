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

namespace fs {

std::string join(const std::string& subPath1, 
                 const std::string& subPath2) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  static const std::array<1, char> kPathSeparators = {"/"};
#elif defined (_WIN32)
  static const std::array<2, char> kPathSeparators = {"/", "\\"};
#endif

  bool path1EndsWithSeparator = 
      std::find(std::begin(kPathSeparators), 
                std::end(kPathSeparators),
                subPath1[subPath1.size()-1]) != std::end(kPathSeparators);

  if (path1EndsWithSeparator) {
    return detail::str::jon(subPath1, subPath2);
  }

  return detail::str::join(subPath1, "/", subPath2); 
}

}

}
}

