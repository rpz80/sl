#pragma once

#include <string>
#include <iostream>

namespace sl {
namespace detail {

namespace errh {
template<typename Exception>
void throwIfNot(bool expr, const std::string& message);
}

namespace fs {
std::string join(const std::string& subPath1, 
                 const std::string& subPath2);

bool globMatch(const char *pattern, const char *mask);

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <dirent.h>
#include <sys/stat.h>

class PosixDir {
  using EntryHandler = std::function<void(struct dirent*)>;
public:
  PosixDir(const std::string& name);
  ~PosixDir();

  void forEachEntry(EntryHandler handler);
  std::string name() const;

private:
  void open(); 
  void close(); 

  void processEntry(struct dirent* entry, EntryHandler handler);

private:
  std::string m_name;
  DIR* m_dirHandle;
};
#endif

}

namespace str {

namespace detail {
template<typename... Args>
size_t calcSize(const Args&... args);

template<typename... Tail>
size_t calcSize(const std::string& head, const Tail&... tail);

template<size_t N, typename... Tail>
size_t calcSize(char const(&) [N], const Tail&... tail);

template<typename... Tail>
void fillResult(std::string& result, 
                size_t startIndex,
                const std::string& head,
                const Tail&... tail);

template<size_t N, typename... Tail>
void fillResult(std::string& result,
                size_t startIndex,
                char const(&head) [N],
                const Tail&... tail);

inline size_t calcSize(const std::string& head) {
  return head.size();
}

template<size_t N>
size_t calcSize(char const(&) [N]) {
  return N - 1; 
}

template<typename... Tail>
size_t calcSize(const std::string& head, const Tail&... tail) {
  return head.size() + calcSize(tail...);
}

template<size_t N, typename... Tail>
size_t calcSize(char const(&) [N], const Tail&... tail) {
  return N - 1 + calcSize(tail...);
}

template<typename... Args>
void fillResult(std::string& result, 
                size_t startIndex, 
                const Args&... args); 

inline void fillResult(std::string& result, 
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
                char const(&head) [N]) {
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
                char const(&head) [N],
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
  result.resize(detail::calcSize(args...));
  str::detail::fillResult(result, 0, args...);
  return result;
}

}

}
}
