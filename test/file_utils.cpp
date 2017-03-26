#include <fstream>
#include <iterator>
#include <cstring>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  #include <sys/stat.h>
  #include <unistd.h>
  #include <dirent.h>
#elif defined(_WIN32)
  #include <windows.h>
#endif

#include <log/format.h>
#include <log/utils.h>
#include "file_utils.h"

using namespace sl::detail;

namespace futils {
namespace detail {

template<typename Stream>
void checkIfOpened(const std::string& name, const Stream& stream) {
  if (!stream.is_open())
    throw std::runtime_error(sl::fmt("%: failed to open file %", 
                                     __FUNCTION__, 
                                     name));
}

}

bool fileExists(const std::string& name) {
  struct stat st;
  if (stat(name.c_str(), &st) != 0)
    return false;
  return true;
}

std::vector<char> fileContent(const std::string& fileName) {
  std::ifstream ifs(fileName);
  detail::checkIfOpened(fileName, ifs);
  return std::vector<char>((std::istreambuf_iterator<char>(ifs)), 
                           std::istreambuf_iterator<char>());

}

int64_t fileSize(const std::string& fileName) {
  struct stat st;
  if (stat(fileName.c_str(), &st) != 0)
    throw std::runtime_error(sl::fmt("%: failed to get file % size", 
                                     __FUNCTION__, fileName));

  return st.st_size;
}

std::unordered_set<std::string> readAll(const std::string& path, 
                                        const std::string& baseName) {
  std::unordered_set<std::string> result;
  fs::Dir dir(path);
  auto mask= baseName + '*';

  dir.forEachEntry([&result, &mask, &path](const fs::Dir::Entry& entry) {
    if (fs::globMatch(entry.name.c_str(), mask.c_str())) {
      auto fullFileName = fs::join(path, entry.name);
      for (const auto& s: splitBy(fileContent(fullFileName), '\n')) {
        result.insert(s);
      }
    }
  });

  return result;
}


TmpDir::TmpDir() : BaseType(create()) {
}

TmpDir::~TmpDir() {}

TmpDir::TmpDir(const std::string& fileNamePattern, size_t count)
  : BaseType(create())
{
  populate(fileNamePattern, count);
}

std::string TmpDir::create() {
  std::string result;
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  char dirTemplate[] = "/tmp/sl.XXXXXX";
  result.assign(mkdtemp(dirTemplate));
#elif defined (_WIN32)
  TCHAR buf[MAX_PATH + 1];
  int bufLen = GetTempPath(MAX_PATH, buf);
  result.resize(bufLen);
  #if defined(_UNICODE)
    WideCharToMultiByte(CP_UTF8, 0, buf, MAX_PATH, bufLen, m_dirPath.data(), 0, 0);
  #else
    result.assign(buf);
  #endif
#endif
  if (result.empty())
    throw std::runtime_error(sl::fmt("%: failed to create temporary dir",
                                     __FUNCTION__));
  return result;
}

void TmpDir::populate(const std::string& fileNamePattern, size_t count) {
  using namespace sl::detail;

  for (size_t i = 0; i < count; ++i) {
    auto fullFileName = str::join(fs::join(BaseType::name(), fileNamePattern),
                                  std::to_string(i));
    std::ofstream ofs(fullFileName);
    detail::checkIfOpened(fullFileName, ofs);
  }
}

}
