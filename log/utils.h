#pragma once

namespace sl {
namespace detail {

namespace errh {
void assertThrow(bool expr, const std::string& message); 
}

namespace fs {
std::string join(const std::string& subPath1, 
                 const std::string& subPath2);

bool maskFits(const std::string& fileName, const std::string& mask);
}

namespace str {
namespace detail {
template<typename... Args>
size_t calcSize(const Args&... args);

size_t calcSize(const std::string& head) {
  return head.size();
}

template<size_t N>
size_t calcSize(const char const(&) [N]) {
  return N - 1; 
}

template<typename... Tail>
size_t calcSize(const std::string& head, const Tail&... tail) {
  return head.size() + calcSize(tail...);
}

template<size_t N, typename... Tail>
size_t calcSize(const char const(&) [N], const Tail&... tail) {
  return N - 1 + calcSize(tail...);
}

template<typename... Args>
void fillResult(std::string& result, 
                size_t startIndex, 
                const Args&... args); 

void fillResult(std::string& result, 
                size_t startIndex,
                const std::string& head) {
  for (size_t i = 0; i < head.size(); ++i) {
    if (head[i] == '\0') {
      break;
    }
    result[startIndex++] = head[i];
  }
}

template<size_t N>
void fillResult(std::string& result,
                size_t startIndex,
                const char const(&head) [N]) {
  for (size_t i = 0; i < N; ++i) {
    if (head[i] == '\0') {
      break;
    }
    result[startIndex++] = head[i];
  }
}

template<typename... Tail>
void fillResult(std::string& result, 
                size_t startIndex,
                const std::string& head,
                const Tail&... tail) {
  for (size_t i = 0; i < head.size(); ++i) {
    if (head[i] == '\0') {
      break;
    }
    result[startIndex++] = head[i];
  }
  fillResult(result, startIndex, tail...);
}

template<size_t N, typename... Tail>
void fillResult(std::string& result,
                size_t startIndex,
                const char const(&head) [N],
                const Tail&... tail) {
  for (size_t i = 0; i < N; ++i) {
    if (head[i] == '\0') {
      break;
    }
    result[startIndex++] = head[i];
  }
  fillResult(result, startIndex, tail...);
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
