#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <log/common_types.h>

namespace sl {
namespace detail {

class FileEntry {
public:
  virtual void remove() = 0;
  virtual void rename(const std::string& newName) = 0;
  virtual std::string name() const = 0;
  virtual int64_t size() const = 0;
  virtual bool exists() const = 0;
  virtual OstreamPtr stream() = 0;
  virtual void closeStream() = 0;
};

using FileEntryPtr = std::unique_ptr<FileEntry>;
using FileEntryList = std::vector<FileEntryPtr>;

FileEntryList getFileEntries(const std::string& path, 
                             const std::string& mask); 

class LogFileEntry : public FileEntry {
public:
  LogFileEntry(const std::string& fullPath);

  virtual void remove() override;
  virtual void rename(const std::string& newName) override;
  virtual std::string name() const override;
  virtual int64_t size() const override;
  virtual bool exists() const override;
  virtual OstreamPtr stream() override;
  virtual void closeStream() override;

  static FileEntryPtr create(const std::string& fullPath);

private:
  std::string m_fullPath;
  OstreamPtr m_stream;
};

}
}
