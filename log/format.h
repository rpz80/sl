#pragma once

#include <sstream>

namespace sl {
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

}

template<typename... Args>
std::string fmt(const char* formatString, Args&&... args) {
  std::stringstream out;
  return detail::fmt(out, formatString, std::forward<Args>(args)...);
}

}
