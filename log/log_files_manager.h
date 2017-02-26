#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <log/rotation_limit_watcher_handler.h>
#include <log/rotation_limit_watcher.h>
#include <log/file_entry.h>
#include <log/file_entry_catalog.h>
#include <log/file_stream.h>

namespace sl {
namespace detail {

class LogFilesManager : public RotationLimitWatcherHandler {
  static const std::string kLogFilesManagerExtension;
public:
  LogFilesManager(int64_t totalLimit,
                  int64_t fileLimit,
                  FileEntryCatalogPtr catalog);

  void write(const void* data, size_t size);
  std::string baseName() const;

protected:
  const FileStreamPtr& stream() const { return m_stream; }
  
private:
  virtual int64_t clearNeeded() override;
  virtual void nextFile() override;

private:
  RotationLimitWatcher m_limitWatcher;
  FileEntryCatalogPtr m_catalog;
  FileStreamPtr m_stream;
};

using LogFilesManagerPtr = std::unique_ptr<LogFilesManager>;
}
}
