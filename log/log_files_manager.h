#pragma once

#include <string>
#include <cstdint>
#include <log/common_types.h>
#include <log/rotation_limit_watcher_handler.h>
#include <log/rotation_limit_watcher.h>
#include <log/file_entry.h>

namespace sl {
namespace detail {

class LogFilesManager : public RotationLimitWatcherHandler {
  static const std::string kLogFilesManagerExtension;
public:
  LogFilesManager(const std::string& logDir, 
                 const std::string& fileNamePattern,
                 int64_t totalLimit,
                 int64_t fileLimit);

  void write(const char* data, int64_t size);
  
private:
  virtual int64_t clearNeeded() override;
  virtual void nextFile() override;

private:
  RotationLimitWatcher m_limitWatcher;
  std::string m_logDir;
  std::string m_fileNamePattern;
  FileEntryList m_fileEntries;
  OstreamPtr m_stream;
};

}
}
