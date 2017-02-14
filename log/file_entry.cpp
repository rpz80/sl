#include <stdio.h>
#include <stdexcept>
#include <memory>

#include <errno.h>

#include <log/format.h>
#include <log/file_entry.h>
#include <log/utils.h>

namespace sl {
namespace detail {

const std::string kLogFileExtension = ".log";

FileStream::FileStream(const std::string& fileName) 
  : m_fileName(fileName),
    m_stream(nullptr) 
{
  open();
}

FileStream::~FileStream() {
  close();
}

void FileStream::open() {
  if (m_stream != nullptr) {
    throw std::runtime_error(sl::fmt("FileStream: file % already opened", 
                                     m_fileName));
  }
  m_stream = fopen(m_fileName.c_str(), "ab");
  if (m_stream == nullptr) {
    throw std::runtime_error(sl::fmt("FileStream: file % open failed", 
                                     m_fileName));
  }
}

void FileStream::close() {
  if (m_stream) {
    fflush(m_stream);
    fclose(m_stream);
    m_stream = nullptr;
  }
}

void FileStream::write(const void* data, size_t size) {
  auto bytesWritten = fwrite(data, size, 1, m_stream) * size;
  if (bytesWritten != size) {
    throw std::runtime_error(sl::fmt("FileStream: file % write failed", 
                                     m_fileName));
  }
}

bool FileStream::isOpened() const {
  return m_stream != nullptr;
}

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <dirent.h>
#include <sys/stat.h>

class FileEntriesPosix {
public:
  FileEntriesPosix(const std::string& path, const std::string& baseName)
    : m_posixDir(path),
      m_baseName(baseName) {}

  FileEntryList operator()() {
    FileEntryList result;

    m_posixDir.forEachEntry([&result, this](struct ::dirent* entry){
      addIfMatches(entry, result);
    });

    return result;
  }

private:
  void addIfMatches(struct dirent* entry, FileEntryList& result) {
    if (!entryMatches(entry))
      return;

    auto newEntry = m_factory.create(m_posixDir.name(), m_baseName);
    result.push_back(newEntry);
  }

  bool entryMatches(struct dirent* entry) {
    return entry->d_type == DT_REG && 
           fs::globMatch(entry->d_name, (m_baseName + '*').data());
  }

private:
  fs::PosixDir m_posixDir;
  std::string m_baseName;
  FileEntryFactory m_factory;
};

FileEntryList getFileEntriesUnix(const std::string& path, 
                                 const std::string& baseName) {
  return FileEntriesPosix(path, baseName)();
}

int64_t getFileSizeUnix(const std::string& fullPath) {
  struct stat st;
  if (stat(fullPath.data(), &st) != 0)
    throw std::runtime_error(
        sl::fmt("Get file size failed for the file %", fullPath));
  return st.st_size;
}

#elif defined (_WIN32)
FileEntryList getFileEntriesWin(const std::string& path, 
                                const std::string& baseName) {
}

int64_t getFileSizeWin(const std::string& fullPath) {

}
#endif

FileEntryPtr FileEntryFactory::create(const std::string& path, 
                                      const std::string& baseName,
                                      size_t index) {
  std::string fullFileName = getFullFileName(path, baseName, index);
  return FileEntryPtr(new FileEntry(fullFileName));
}

std::string FileEntryFactory::getFullFileName(const std::string& path,
                                              const std::string& baseName,
                                              size_t index) {
  if (m_index == 0) {
    return str::join(fs::join(m_path, baseName), kLogFileExtension);
  }

  return str::join(fs::join(m_path, baseName), 
                   std::to_string(m_index),
                   kLogFileExtension);
}

FileEntryList FileEntryFactory::getExistent(const std::string& path, 
                                            const std::string& baseName) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  return getFileEntriesUnix(path, baseName);
#elif defined (_WIN32)
  return getFileEntriesWin(path, baseName);
#endif
}

FileEntry::FileEntry(const std::string& fullPath) : 
    m_fullPath(fullPath) {
}

void FileEntry::remove() {
  if (::remove(m_fullPath.data()) != 0) {
    throw std::runtime_error(sl::fmt("Delete file % failed", m_fullPath));
  }
}

void FileEntry::rename(const std::string& newName) {
  if (::rename(m_fullPath.data(), newName.data()) != 0) {
    throw std::runtime_error(sl::fmt("Rename file % to % failed", 
                                     m_fullPath, 
                                     newName));
  }
  m_fullPath = newName;
}

std::string FileEntry::name() const  {
  return m_fullPath;
}

int64_t FileEntry::size() const {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  return getFileSizeUnix(m_fullPath);
#elif defined (_WIN32)
  return getFileSizeWin(m_fullPath);
#endif
}

bool FileEntry::exists() const {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  struct stat st; 
  if (stat(m_fullPath.c_str(), &st) == 0)
    return true;
#elif defined(_WIN32)
#endif
  return false;
}

FileStreamPtr FileEntry::open() {
  return FileStreamPtr(new FileStream(m_fullPath));
}

} // detail
} // sl
