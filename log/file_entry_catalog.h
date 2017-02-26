#pragma once

#include <string>
#include <memory>
#include <log/file_entry.h>

namespace sl {
namespace detail {

class FileEntryCatalog {
public:
  FileEntryCatalog(IFileEntryFactory* entryFactory, 
                   const std::string& path, 
                   const std::string& baseName);
  IFileEntry& first();
  void rotate();
  int64_t removeLast();
  std::string baseName() const;
  size_t size() const;
  bool empty() const;
  int64_t totalBytes() const;

protected:
  const FileEntryList& entries() const { return m_entries; }

private:
  void sortEntries();
  void addDefault();
  void rename(size_t index);

private:
  IFileEntryFactory* m_factory;
  FileEntryList m_entries;
  std::string m_path;
  std::string m_baseName;
};

using FileEntryCatalogPtr = std::unique_ptr<FileEntryCatalog>;

}
}