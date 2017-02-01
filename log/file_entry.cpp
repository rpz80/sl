#include <log/file_entry.h>
#include <log/utils.h>


namespace sl {
namespace detail {
/*
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

  while ((entry = readdir(d)) != nullptr && entry->d_type == DT_REG) {
    std::string fullFilePath = fs::join(path, entry->d_name);
    result.emplace_back(std::make_unique<LogFileEntry>(fullFilePath));
  }

  closedir(d);
  return result;
}

#elif defined (_WIN32)
FileEntryList getFileEntriesWin(const std::string& path, 
                                  const std::string& mask) {
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

LogFileEntry::LogFileEntry(const std::string& path) {
}

void LogFileEntry::remove() {
}

void LogFileEntry::rename(const std::string& newName) {
}

int64_t LogFileEntry::size() const  {
}

  getFileEntriesUnix();

*/
}

}
