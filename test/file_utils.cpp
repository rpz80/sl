#include <fstream>
#include <iterator>
#include <cstring>

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <log/format.h>
#include <log/utils.h>
#include "file_utils.h"

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

std::unordered_set<std::string> readAll(const std::string& path, const std::string& baseName) {
  std::unordered_set<std::string> result;
  return result;
}


TmpDir::TmpDir() {
  create();
}

TmpDir::TmpDir(const std::string& fileNamePattern, size_t count) 
{
  create();
  populate(fileNamePattern, count);
}

void TmpDir::create() {
  char dirTemplate[] = "/tmp/sl.XXXXXX";
  m_dirPath = mkdtemp(dirTemplate);
  if (m_dirPath.empty())
    throw std::runtime_error(sl::fmt("%: failed to create temporary dir",
                                     __FUNCTION__));
}

void TmpDir::populate(const std::string& fileNamePattern, size_t count) {
  using namespace sl::detail;

  for (size_t i = 0; i < count; ++i) {
    auto fullFileName = str::join(fs::join(m_dirPath, fileNamePattern), 
                                  std::to_string(i));
    std::ofstream ofs(fullFileName);
    detail::checkIfOpened(fullFileName, ofs);
  }
}

void TmpDir::forEachFile(FileHandler handler) {
  forEachFile(m_dirPath, handler);
}

void TmpDir::forEachFile(const std::string& dirName, FileHandler handler) {
  using namespace sl::detail;
  fs::PosixDir dir(dirName);

  dir.forEachEntry([this, &dirName, handler](struct dirent* entry) {
    auto fullName = fs::join(dirName, entry->d_name);

    processFile(fullName, handler);
  });
}

void TmpDir::processFile(const std::string& name, FileHandler handler) {
  struct stat st;
  if (stat(name.c_str(), &st) != 0) {
    return;
  }

  handler(name, S_ISDIR(st.st_mode) ? FileType::dir : FileType::file);
}

void TmpDir::removeHandler(const std::string& name, FileType type) {
  if (type == FileType::dir) {
    return forEachFile(name, [this](const std::string& name, FileType type) {
      removeHandler(name, type);
    });
  }

  if (std::remove(name.c_str()) != 0)
    throw std::runtime_error(sl::fmt("%: remove of % failed",
                                     __FUNCTION__, name));
}

TmpDir::~TmpDir() {
  TmpDir::remove();
}

void TmpDir::remove() {
  forEachFile([this](const std::string& name, FileType type) {
    removeHandler(name, type);
  });

  if (rmdir(m_dirPath.c_str()) != 0)
    throw std::runtime_error(sl::fmt("%: failed to remove dir %",
                                     __FUNCTION__, m_dirPath));
}

std::string TmpDir::path() const {
  return m_dirPath;
}
}