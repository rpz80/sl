#include <algorithm>
#include <log/file_entry_catalog.h>
#include <log/utils.h>

namespace sl {
namespace detail {

FileEntryCatalog::FileEntryCatalog(IFileEntryFactory* entryFactory, 
                   const std::string& path, 
                   const std::string& mask)
  : m_factory(entryFactory),
    m_entries(m_factory->getExistent(path, mask)),
    m_path(path),
    m_mask(mask) 
{
  if (m_entries.empty()) {
    makeFirst();
  }

  sortEntries();
}

void FileEntryCatalog::sortEntries() {
  std::sort(m_entries.begin(), m_entries.end(), 
            [](const FileEntryPtr& lhs, const FileEntryPtr& rhs) {
              return lhs->name() < rhs->name();
            });
}

void FileEntryCatalog::makeFirst() {
  m_entries.emplace_back(m_factory->create(m_path, m_mask));
}

IFileEntry& FileEntryCatalog::first() {
  return *m_entries[0];
}

void FileEntryCatalog::rotate() {

}

void FileEntryCatalog::removeLast() {
  
}

}
}