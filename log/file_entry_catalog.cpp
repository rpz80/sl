#include <stdio.h>
#include <algorithm>
#include <log/file_entry_catalog.h>
#include <log/utils.h>
#include <log/format.h>

namespace sl {
namespace detail {

FileEntryCatalog::FileEntryCatalog(IFileEntryFactory* entryFactory, 
                   const std::string& path, 
                   const std::string& baseName)
  : m_factory(entryFactory),
    m_entries(m_factory->getExistent(path, baseName)),
    m_path(path),
    m_baseName(baseName) 
{
  if (m_entries.empty()) {
    addDefault();
  }

  sortEntries();
}

void FileEntryCatalog::sortEntries() {
  std::sort(m_entries.begin(), m_entries.end(), 
            [](const FileEntryPtr& lhs, const FileEntryPtr& rhs) {
              return lhs->name() < rhs->name();
            });
}

void FileEntryCatalog::addDefault() {
  m_entries.emplace_front(m_factory->create(m_path, m_baseName));
}

IFileEntry& FileEntryCatalog::first() {
  if (m_entries.empty()) {
    throw std::runtime_error(sl::fmt("%: no entries", __FUNCTION__));
  }

  return *m_entries[0];
}

void FileEntryCatalog::rotate() {
  for (int i = 0; i < m_entries.size(); ++i) {
    this->rename(i);
  }

  addDefault();
}

void FileEntryCatalog::rename(size_t index) {
  std::string newName = str::join(fs::join(m_path, m_baseName), 
                                  std::to_string(index + 1), 
                                  kLogFileExtension);
  m_entries[index]->rename(newName);
}

void FileEntryCatalog::removeLast() {
  if (m_entries.empty()) {
    throw std::runtime_error(sl::fmt("%: no entries", __FUNCTION__));
  }

  m_entries.back()->remove(); 
  m_entries.pop_back();
}

std::string FileEntryCatalog::baseName() const {
  return m_baseName;
}

}
}