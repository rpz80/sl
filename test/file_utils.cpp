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

class PosixDir {
  using EntryHandler = std::function<void(struct dirent*)>;
public:
  PosixDir(const std::string& name) : m_name(name) {}

  void forEachEntry(EntryHandler handler) {
    struct dirent* entry;

    open();
    while ((entry = readdir(m_dirHandle)) != nullptr) {
      processEntry(entry, handler);
    }
    close();
  }

  ~PosixDir() {
    close();
  }

private:
  void open() {
    m_dirHandle = opendir(m_name.c_str());
    if (!m_dirHandle)
      throw std::runtime_error(sl::fmt("%: open dir % failed",
                                       __FUNCTION__, m_name));
  }

  void close() {
    if (m_dirHandle)
      closedir(m_dirHandle);
  }

  void processEntry(struct dirent* entry, EntryHandler handler) {
    if (std::strcmp(entry->d_name, "..") == 0 || std::strcmp(entry->d_name, ".") == 0) {
      return;
    }

    handler(entry);
  }

private:
  std::string m_name;
  DIR* m_dirHandle;
};

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


TmpDir::TmpDir() {
  create();
}

TmpDir::TmpDir(const std::string& fileNamePattern, size_t count) 
  : m_fileNamePattern(fileNamePattern) 
{
  create();
  populate(count);
}

void TmpDir::create() {
  char dirTemplate[] = "/tmp/sl.XXXXXX";
  m_dirPath = mkdtemp(dirTemplate);
  if (m_dirPath.empty())
    throw std::runtime_error(sl::fmt("%: failed to create temporary dir",
                                     __FUNCTION__));
}

void TmpDir::populate(size_t count) {
  using namespace sl::detail;

  for (size_t i = 0; i < count; ++i) {
    auto fullFileName = str::join(fs::join(m_dirPath, "file"), std::to_string(i));
    std::ofstream ofs(fullFileName);
    detail::checkIfOpened(fullFileName, ofs);
  }
}

void TmpDir::forEachFile(FileHandler handler) {
  forEachFile(m_dirPath, handler);
}

void TmpDir::forEachFile(const std::string& dirName, FileHandler handler) {
  using namespace sl::detail;
  detail::PosixDir dir(dirName);

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

}