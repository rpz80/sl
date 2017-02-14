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
bool globMatch(const char *pattern, const char *mask)
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
        return true;
      patternCopy = pattern;
      for (; *patternCopy != 0; ++patternCopy) {
        if (globMatch(patternCopy, mask + 1))
          return true;
      }
      return false;
    }
    case '?': {
      if (*pattern++ == '\0')
        return false;
      break;
    }
    case '[': {
      memset(groupBuf, '\0', sizeof(groupBuf));
      groupSize = 0;
      notGroupFlag = 0;
      ++mask;
      if (*mask == 0)
        return false;
      if (*mask == '!') {
        notGroupFlag = 1;
        ++mask;
      }
      while (1) {
        if (*mask == 0)
          return false;
        if (*mask == ']') {
          break;
        }
        groupBuf[groupSize++] = *mask;
        ++mask;
      }
      for (i = 0; i < groupSize; ++i) {
        if (*pattern == groupBuf[i]) {
          if (notGroupFlag)
            return false;
          break;
        }
      }
      ++pattern;
      break;
    }
    default: {
      if (*pattern++ != *mask)
        return false;
    }
    }
  }

  if (*pattern == '\0')
    return true;

  return false;
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

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <dirent.h>
#include <sys/stat.h>

PosixDir::PosixDir(const std::string& name) : m_name(name) {}

void PosixDir::forEachEntry(EntryHandler handler) {
  struct dirent* entry;

  open();
  while ((entry = readdir(m_dirHandle)) != nullptr) {
    processEntry(entry, handler);
  }
  close();
}

PosixDir::~PosixDir() {
  close();
}

std::string PosixDir::name() const {
  return m_name;
}

void PosixDir::open() {
  m_dirHandle = opendir(m_name.c_str());
  if (!m_dirHandle)
    throw std::runtime_error(sl::fmt("%: open dir % failed",
                                     __FUNCTION__, m_name));
}

void PosixDir::close() {
  if (m_dirHandle) {
    closedir(m_dirHandle);
    m_dirHandle = nullptr;
  }
}

void PosixDir::processEntry(struct dirent* entry, EntryHandler handler) {
  if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) {
    return;
  }

  handler(entry);
}

#endif

}

}
}

