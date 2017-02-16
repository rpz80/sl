#pragma once

#include <memory>
#include <stdio.h>
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

}
}