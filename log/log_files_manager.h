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
  LogFilesManager(const std::string& logDir, 
                 const std::string& baseName,
                 int64_t totalLimit,
                 int64_t fileLimit,
                 FileEntryFactoryPtr factory);

  void write(const char* data, int64_t size);
  std::string baseName() const;
  
private:
  virtual int64_t clearNeeded() override;
  virtual void nextFile() override;

private:
  RotationLimitWatcher m_limitWatcher;
  FileEntryFactoryPtr m_factory;
  FileEntryCatalog m_catalog;
  FileStreamPtr m_stream;
};

using LogFilesManagerPtr = std::unique_ptr<LogFilesManager>;
}
}
