#include <log/file_entry_catalog.h>

namespace sl {
namespace detail {

FileEntryCatalog::FileEntryCatalog(FileEntryList entries)
  : m_entries(std::move(entries)) {}

FileEntry& FileEntryCatalog::first() {

}

void FileEntryCatalog::rotate() {

}

void FileEntryCatalog::removeLast() {
  
}

}
}