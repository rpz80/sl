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
class PosixDir {
  using EntryHandler = std::function<void(struct dirent*)>;
public:
  Dir(const std::string& name);
  ~Dir();

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


#include <dirent.h>
#include <sys/stat.h>

Dir::Dir(const std::string& name) : m_name(name) {}

void Dir::forEachEntry(EntryHandler handler) {
  struct dirent* entry;

  open();
  while ((entry = readdir(m_dirHandle)) != nullptr) {
    processEntry(entry, handler);
  }
  close();
}

Dir::~Dir() {
  close();
}

std::string Dir::name() const {
  return m_name;
}

void Dir::open() {
  m_dirHandle = opendir(m_name.c_str());
  if (!m_dirHandle)
    throw std::runtime_error(sl::fmt("%: open dir % failed",
                                     __FUNCTION__, m_name));
}

void Dir::close() {
  if (m_dirHandle) {
    closedir(m_dirHandle);
    m_dirHandle = nullptr;
  }
}

void Dir::processEntry(struct dirent* entry, EntryHandler handler) {
  if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) {
    return;
  }

  handler(entry);
}

#elif defined(_WIN32)
#include <windows.h>

class DirImpl {
public:
  DirImpl(const std::string& name)
    : m_dirHandle(INVALID_HANDLE_VALUE),
      m_name(name)
  {
    open();
  }

  ~DirImpl() {
    close();
  }

  void forEachEntry(Dir::EntryHandler handler) {
    open();
    while (FindNextFile(m_dirHandle, &m_ffd) != 0) {
      Dir::Entry entry(
        m_ffd.cFileName,
        m_ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? Dir::dir : Dir::file);
      handler(entry);
    }
    close();
  }

  std::string name() const { return m_name; }

private:
  void open()  {
    close();

    std::string findMask = fs::join(m_name, "/*");
    TCHAR* szDir = nullptr;

#if defined (_UNICODE)
    TCHAR buf[2048];
    MultiByteToWideChar(CP_UTF8, 0, findMask.c_str(), -1, buf, 2048);
    szDir = buf;
#else
    szDir = (TCHAR*)findMask.c_str();
#endif

    m_dirHandle = FindFirstFile(szDir, &m_ffd);
    if (m_dirHandle == INVALID_HANDLE_VALUE) {
      throw std::runtime_error(sl::fmt("% Failed to open dir %", __FUNCTION__, m_name));
    }
  }

  void close() {
    if (m_dirHandle != INVALID_HANDLE_VALUE) {
      FindClose(m_dirHandle);
      m_dirHandle = INVALID_HANDLE_VALUE;
    }
  }

private:
  WIN32_FIND_DATA m_ffd;
  HANDLE m_dirHandle;
  std::string m_name;
};

#endif

Dir::Dir(const std::string& name) : m_impl(new DirImpl(name)) {}
Dir::~Dir() {}

void Dir::forEachEntry(EntryHandler handler) {
  m_impl->forEachEntry(handler);
}

std::string Dir::name() const { return m_impl->name(); }

}

}
}

