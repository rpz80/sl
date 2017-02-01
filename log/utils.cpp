#include <vector>
#include <array>
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include <log/utils.h>
#include <log/exception.h>
#include <log/format.h>

namespace sl {
namespace detail {

namespace fs {
int GlobMatch(const char *pattern, const char *mask)
{
  const char *patternCopy;
  char groupBuf[512];
  int groupSize;
  int notGroupFlag;
  int i;

  for (; *mask != 0; ++mask) {
    switch (*mask) {
    case '*': {
      if (*(mask + 1) == '\0')
        return 1;
      patternCopy = pattern;
      for (; *patternCopy != 0; ++patternCopy) {
        if (GlobMatch(patternCopy, mask + 1))
          return 1;
      }
      return 0;
    }
    case '?': {
      if (*pattern++ == '\0')
        return 0;
      break;
    }
    case '[': {
      memset(groupBuf, '\0', sizeof(groupBuf));
      groupSize = 0;
      notGroupFlag = 0;
      ++mask;
      if (*mask == 0)
        return 0;
      if (*mask == '!') {
        notGroupFlag = 1;
        ++mask;
      }
      while (1) {
        if (*mask == 0)
          return 0;
        if (*mask == ']') {
          break;
        }
        groupBuf[groupSize++] = *mask;
        ++mask;
      }
      for (i = 0; i < groupSize; ++i) {
        if (*pattern == groupBuf[i]) {
          if (notGroupFlag)
            return 0;
          break;
        }
      }
      ++pattern;
      break;
    }
    default: {
      if (*pattern++ != *mask)
        return 0;
    }
    }
  }

  if (*pattern == '\0')
    return 1;

  return 0;
}

std::string join(const std::string& subPath1, 
                 const std::string& subPath2) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  static const std::array<char, 1> kPathSeparators = {'/'};
#elif defined (_WIN32)
  static const std::array<char, 2> kPathSeparators = {'/', '\\'};
#endif

  bool path1EndsWithSeparator =
      std::find(kPathSeparators.cbegin(),
                kPathSeparators.cend(),
                subPath1[subPath1.size()-1]) != std::end(kPathSeparators);

  if (path1EndsWithSeparator) {
    return detail::str::join(subPath1, subPath2);
  }

  return detail::str::join(subPath1, "/", subPath2);
}

}

}
}

