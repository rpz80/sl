#pragma once

#include <string>
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
  void removeLast();

private:
  void sortEntries();
  void makeFirst();

private:
  IFileEntryFactory* m_factory;
  FileEntryList m_entries;
  std::string m_path;
  std::string m_baseName;
};

}
}