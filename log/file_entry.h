#pragma once

#include <cstdint>
#include <memory>
#include <deque>
#include <string>

namespace sl {
namespace detail {

class IFileStream {
public:
  virtual ~IFileStream() {}
  virtual void open() = 0;
  virtual void close() = 0;
  virtual void write(const void* data, size_t size) = 0;
  virtual bool isOpened() const = 0;
};

class FileStream : public IFileStream {
public:
  FileStream(const std::string& fileName);
  ~FileStream();
  virtual void open() override;
  virtual void close() override;
  virtual void write(const void* data, size_t size) override;
  virtual bool isOpened() const override;

private:
  std::string m_fileName;
  FILE* m_stream;
};

using FileStreamPtr = std::unique_ptr<IFileStream>;

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

}
}
