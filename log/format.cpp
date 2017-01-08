#include <log/format.h>

namespace sl {
namespace detail {

void printTillSpecial(std::stringstream& out, 
                      const char** formatString) {
  for (; **formatString; ++*formatString) {
    if (**formatString == '\\') {
      if (*(*formatString + 1) && *(*formatString + 1) == '%') {
        ++*formatString;
        out << '%';
      } else {
        out << **formatString;
      }
    } else if (**formatString == '%') {
      break;
    } else {
      out << **formatString;
    }
  }
}

std::string fmt(std::stringstream& out,
                const char* formatString) {
  for (; *formatString; ++formatString) {
    out << *formatString;
  }
  return out.str();
}

}
}

