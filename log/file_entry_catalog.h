#pragma once

#include <log/file_entry.h>

namespace sl {
namespace detail {

class FileEntryCatalog {
public:
  FileEntryCatalog(const std::string& path, const std::string& mask);
  FileEntry& first();
  void rotate();
private:
  FileEntryList m_entries;
};

}
}