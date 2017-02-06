#pragma once

#include <cstdint>
#include <memory>
#include <deque>
#include <string>
#include <log/common_types.h>

namespace sl {
namespace detail {

class FileEntry;
using FileEntryPtr = std::unique_ptr<FileEntry>;

class FileEntry {
public:
  FileEntry(const std::string& fullPath);

  void remove();
  void rename(const std::string& newName);
  std::string name() const;
  int64_t size() const;
  bool exists() const;
  FILE* stream();
  void closeStream();

  static FileEntryPtr create(const std::string& fullPath);

private:
  std::string m_fullPath;
  FILE* m_stream;
};

using FileEntryList = std::deque<FileEntryPtr>;

FileEntryList getFileEntries(const std::string& path, 
                             const std::string& mask); 


}
}
