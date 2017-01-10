#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace sl {
namespace detail {

class FileEntry {
public:
  virtual void remove() = 0;
  virtual void rename(const std::string& newName) = 0;
  virtual int64_t size() const = 0;
};

using FileEntryPtr = std::unique_ptr<FileEntry>;
using FileEntryVector = std::vector<FileEntryPtr>;

FileEntryVector getFileEntries(const std::string& path, 
                               const std::string& mask);

class LogFileEntry : public FileEntry {
public:
  LogFileEntry(const std::string& path);

  virtual void remove() override;
  virtual void rename(const std::string& newName) override;
  virtual int64_t size() const override;

private:
  std::string m_fullPath;
};

}
}
