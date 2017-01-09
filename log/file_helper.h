#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace sl {
namespace detail {

class FileHelper {
public:
  virtual void remove() = 0;
  virtual void rename(const std::string& newName) = 0;
  virtual int64_t size() const = 0;
};

using FileHelperPtr = std::unique_ptr<FileHelper>;
using FileHelperVector = std::vector<FileHelperPtr>;

FileHelperVector getFileHelpers(const std::string& path, const std::string& mask);

class LogFileHelper : public FileHelper {
public:
  LogFileHelper(const std::string& path);

  virtual void remove() override;
  virtual void rename(const std::string& newName) override;
  virtual int64_t size() const override;

private:
  std::string m_fullPath;
};

}
}
