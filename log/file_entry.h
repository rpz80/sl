#pragma once

#include <cstdint>
#include <memory>
#include <deque>
#include <string>

namespace sl {
namespace detail {

class FileStream {
public:
  FileStream(const std::string& fileName);
  ~FileStream();
  void open();
  void close();
  void write(const void* data, size_t size);
  bool isOpened() const;

private:
  std::string m_fileName;
  FILE* m_stream;
};

using FileStreamPtr = std::unique_ptr<FileStream>;

class FileEntry;
using FileEntryPtr = std::unique_ptr<FileEntry>;

class FileEntry {
public:
  FileEntry(const std::string& fileName);

  void remove();
  void rename(const std::string& newName);
  std::string name() const;
  int64_t size() const;
  bool exists() const;
  FileStreamPtr open();

private:
  std::string m_fullPath;
};

using FileEntryList = std::deque<FileEntryPtr>;

FileEntryList getFileEntries(const std::string& path, 
                             const std::string& mask); 


}
}
