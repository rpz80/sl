#pragma once

namespace sl {
namespace detail {

namespace errh {
void assertThrow(bool expr, const std::string& message); 
}

namespace fs {
std::string join(const std::string& subPath1, 
                 const std::string& subPath2);
}

namespace str {
namespace detail {
template<typename... Args>
size_t calcSize(const Args&... args);

template<typename... Tail>
size_t calcSize(const std::string& head, const Tail&... tail) {
  return head.size() + calcSize(tail...);
}

template<size_t N, typename... Tail>
size_t calcSize(const char const(&) [N], const Tail&... tail) {
  return N - 1 + calcSize(tail...);
}
}

template<typename... Args>
std::string join(const Args&... args) {
  std::string result;
  result.reserve(detail::calcSize(args...));
  str::detail::fillResult(result, tail...);
}

}

}
}
