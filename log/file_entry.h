#pragma once

#include <cstdint>
#include <memory>
#include <deque>
#include <string>
#include <log/file_stream.h>

namespace sl {
namespace detail {

const std::string kLogFileExtension = ".log";

class IFileEntry {
public:
  virtual ~IFileEntry() {}
  virtual void remove() = 0;
  virtual void rename(const std::string& newName) = 0;
  virtual std::string name() const = 0;
  virtual int64_t size() const = 0;
  virtual bool exists() const = 0;
  virtual FileStreamPtr open() = 0;
};

using FileEntryPtr = std::unique_ptr<IFileEntry>;

class FileEntry : public IFileEntry {
public:
  FileEntry(const std::string& fileName);

  virtual void remove() override;
  virtual void rename(const std::string& newName) override;
  virtual std::string name() const override;
  virtual int64_t size() const override;
  virtual bool exists() const override;
  virtual FileStreamPtr open() override;

private:
  std::string m_fullPath;
};

using FileEntryList = std::deque<FileEntryPtr>;

class IFileEntryFactory {
public:
  virtual FileEntryPtr create(const std::string& path, 
                              const std::string& baseName,
                              size_t index = 0) = 0;
  virtual FileEntryList getExistent(const std::string& path, 
                                    const std::string& baseName) = 0;
};

class FileEntryFactory : public IFileEntryFactory {
public:
  virtual FileEntryPtr create(const std::string& path, 
                              const std::string& baseName,
                              size_t index = 0) override;
  virtual FileEntryList getExistent(const std::string& path, 
                                    const std::string& baseName) override;

private:
  static std::string getFullFileName(const std::string& path,
                                     const std::string& baseName,
                                     size_t index);
};

using FileEntryFactoryPtr = std::unique_ptr<IFileEntryFactory>;

}
}
