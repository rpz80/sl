#include <stdio.h>
#include <stdexcept>
#include <memory>
#include <log/format.h>
#include <log/file_entry.h>
#include <log/utils.h>

namespace sl {
namespace detail {

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <dirent.h>
#include <sys/stat.h>

FileEntryList getFileEntriesUnix(const std::string& path, 
                                 const std::string& mask) {
  DIR* d;
  struct dirent* entry;
  FileEntryList result;

  if ((d = opendir(path.c_str())) == nullptr) {
    return result;
  }

  while ((entry = readdir(d)) != nullptr) {
    if (entry->d_type == DT_REG && fs::globMatch(entry->d_name, mask.data())) {
      std::string fullFilePath = fs::join(path, entry->d_name);
      result.emplace_back(std::unique_ptr<LogFileEntry>(new LogFileEntry(fullFilePath)));
    }
  }

  closedir(d);
  return result;
}

int64_t getFileSizeUnix(const std::string& fullPath) {
  struct stat st;
  if (stat(fullPath.data(), &st) != 0)
    throw std::runtime_error(sl::fmt("Get file size failed for the file %", fullPath));
  return st.st_size;
}

#elif defined (_WIN32)
FileEntryList getFileEntriesWin(const std::string& path, 
                                  const std::string& mask) {
}

int64_t getFileSizeWin(const std::string& fullPath) {

}
#endif

FileEntryList getFileEntries(const std::string& path, 
                             const std::string& mask) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  return getFileEntriesUnix(path, mask);
#elif defined (_WIN32)
  return getFileEntriesWin(path, mask);
#endif
}

LogFileEntry::LogFileEntry(const std::string& fullPath) : 
    m_fullPath(fullPath) {
}

void LogFileEntry::remove() {
  if (::remove(m_fullPath.data()) != 0) {
    throw std::runtime_error(sl::fmt("Delete file % failed", m_fullPath));
  }
}

void LogFileEntry::rename(const std::string& newName) {
  if (::rename(m_fullPath.data(), newName.data()) != 0) {
    throw std::runtime_error(sl::fmt("Rename file % to % failed", m_fullPath, newName));
  }
  m_fullPath = newName;
}

std::string LogFileEntry::name() const  {
  return m_fullPath;
}

int64_t LogFileEntry::size() const {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  return getFileSizeUnix(m_fullPath);
#elif defined (_WIN32)
  return getFileSizeWin(m_fullPath);
#endif
}

bool LogFileEntry::exists() const {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  struct stat st; 
  if (stat(m_fullPath.c_str(), &st) == 0)
    return true;
#elif defined(_WIN32)
#endif
  return false;
}

OstreamPtr LogFileEntry::stream() {
  if (m_stream) {
    return m_stream;
  }
  m_stream = OstreamPtr(new std::ofstream(m_fullPath));
  if (!std::static_pointer_cast<std::ofstream>(m_stream)->is_open()) {
    m_stream.reset();
    throw std::runtime_error(sl::fmt("Failed to open file % for write", m_fullPath));
  }
  return m_stream;
}

void LogFileEntry::closeStream() {
  if (m_stream)
    std::static_pointer_cast<std::ofstream>(m_stream)->close();
}

}

}
