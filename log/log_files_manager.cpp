#include <algorithm>
#include <log/log_files_manager.h>
#include <log/utils.h>

namespace sl {
namespace detail {

const std::string LogFilesManager::kLogFilesManagerExtension = ".log";

LogFilesManager::LogFilesManager(const std::string& logDir, 
                               const std::string& fileNamePattern,
                               int64_t totalLimit,
                               int64_t fileLimit)
  : m_limitWatcher(totalLimit, fileLimit, this),
    m_logDir(logDir),
    m_fileNamePattern(fileNamePattern + "*"),
    m_fileEntries(getFileEntries(logDir, m_fileNamePattern))
{
  if (m_fileEntries.empty()) {
    auto fileEntryName = fs::join(logDir, str::join(fileNamePattern, ".log"));
    m_fileEntries.emplace_back(LogFileEntry::create(fileEntryName));
    return;
  }
  std::sort(m_fileEntries.begin(),
            m_fileEntries.end(), 
            [](const FileEntryPtr& lhs, const FileEntryPtr& rhs) {
    return lhs->name() < rhs->name();
  });
  m_stream = m_fileEntries[0]->stream();
}

int64_t LogFilesManager::clearNeeded() {

}

void LogFilesManager::nextFile() {

}

void LogFilesManager::write(const char* data, int64_t size) {
  m_stream->write(data, size);
  m_limitWatcher.addWritten(size);
}

/*
std::ostream& LogFilesManager::getCurrentFileStream() {
}

std::string LogFilesManager::getFullPath() const {
}

void LogFilesManager::combineFullPath() {

}

void LogFilesManager::openLogFilesManager() {

}

int64_t LogFilesManager::clearNeeded(int64_t spaceToClear) {
}

bool LogFilesManager::nextFile() {
}
*/
}
}
