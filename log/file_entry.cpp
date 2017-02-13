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

FileEntryList getFileEntriesUnix(const std::string& path, 
                                 const std::string& mask) {
  DIR* d;
  struct dirent* entry;
  FileEntryList result;

  if ((d = opendir(path.c_str())) == nullptr) {
    throw std::runtime_error(sl::fmt("%: path % doesn't exist",
                                     __FUNCTION__,
                                     path));
  }

  while ((entry = readdir(d)) != nullptr) {
    if (entry->d_type == DT_REG && fs::globMatch(entry->d_name, mask.data())) {
      std::string fullFilePath = fs::join(path, entry->d_name);
      result.emplace_back(
          std::unique_ptr<FileEntry>(new FileEntry(fullFilePath)));
    }
  }

  closedir(d);
  return result;
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
                                const std::string& mask) {
}

int64_t getFileSizeWin(const std::string& fullPath) {

}
#endif

FileEntryPtr FileEntryFactory::create(const std::string& path, 
                                      const std::string& mask,
                                      size_t index) {
  auto fullFileName =  FileNameComposer(path, mask, index)();
  return FileEntryPtr(new FileEntry(fullFileName));
}

FileNameComposer::FileNameComposer(const std::string& path,
                                   const std::string& mask,
                                   size_t index)
  : m_path(path),
    m_mask(mask),
    m_index(index) {}

std::string FileNameComposer::operator()() const {
  if (m_index == 0) {
    return str::join(fs::join(m_path, stripMask()), kLogFileExtension);
  }

  return str::join(fs::join(m_path, stripMask()), 
                   std::to_string(m_index),
                   kLogFileExtension);
}

std::string FileNameComposer::stripMask() const {
  return m_mask.substr(0, findLastSpecCharIndex());
}

size_t FileNameComposer::findLastSpecCharIndex() const {
  for (int i = m_mask.size() - 1; i >= 0; --i) {
    if (notSpecial(m_mask[i])) {
      return i + 1;
    }
  }
  
  return 0;
}

bool FileNameComposer::notSpecial(char c) const {
  return c != '[' && c != ']' && c != '*' && c != '?';
}

FileEntryList FileEntryFactory::getExistent(const std::string& path, 
                                            const std::string& mask) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  return getFileEntriesUnix(path, mask);
#elif defined (_WIN32)
  return getFileEntriesWin(path, mask);
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
