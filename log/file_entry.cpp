#include <log/file_info.h>


namespace sl {
namespace detail {

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <dirent.h>
#include <sys/stat.h>

FileEntryVector getFileEntriesUnix(const std::string& path, 
                                   const std::string& mask) {
  DIR* d = opendir(path.c_str());
  if (d == nullptr) {
    return FileEntryVector();
  }
  
  FileEntryVector result;
  while ((auto entry = readdir(d)) != nulltpr) {
    std::string fullFileName = 
    result.emplace_back(new LogFileEntry());
  }

}
#elif defined (_WIN32)
FileEntryVector getFileEntriesWin(const std::string& path, 
                                  const std::string& mask) {
}
#endif

FileEntryVector getFileEntries(const std::string& path, 
                               const std::string& mask) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  return getFileEntriesUnix(path, mask);
#elif defined (_WIN32)
  return getFileEntriesWin(path, mask);
#endif
}

LogFileEntry::LogFileEntry(const std::string& path) {
}

void LogFileEntry::remove() {
}

void LogFileEntry::rename(const std::string& newName) {
}

int64_t LogFileEntry::size() const  {
}

  getFileEntriesUnix();
}
}
