#include <log/file_entry_catalog.h>

namespace sl {
namespace detail {

FileEntryCatalog::FileEntryCatalog(const std::string& path, 
                                   const std::string& mask)
  : m_entries(getFileEntries(path, mask)) {}

FileEntry& FileEntryCatalog::first() {

}

void FileEntryCatalog::rotate() {
  
}

}
}