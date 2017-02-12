#pragma once

#include <log/file_entry.h>

namespace sl {
namespace detail {

class FileEntryCatalog {
public:
  FileEntryCatalog(FileEntryList entries);
  FileEntry& first();
  void rotate();
  void removeLast();
private:
  FileEntryList m_entries;
};

}
}