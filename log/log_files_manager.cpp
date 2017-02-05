#include <algorithm>
#include <string.h>
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
    m_fileEntries.emplace_back(FileEntry::create(fileEntryName));
  } else {
    std::sort(m_fileEntries.begin(),
              m_fileEntries.end(), 
              [](const FileEntryPtr& lhs, const FileEntryPtr& rhs) {
      return lhs->name() < rhs->name();
    });
  }
  m_stream = m_fileEntries[0]->stream();
}

int64_t LogFilesManager::clearNeeded() {
  int64_t result = 0;

  if (m_fileEntries.size() == 1) {
    m_fileEntries[0]->closeStream();
    result = m_fileEntries[0]->size();
    m_fileEntries[0]->remove();
    m_stream = m_fileEntries[0]->stream();
    return result;
  }

  auto entryToRemove = std::move(m_fileEntries[m_fileEntries.size() - 1]);
  result = entryToRemove->size();
  entryToRemove->closeStream();
  entryToRemove->remove();
  m_fileEntries.pop_back();

  return result;
}

void LogFilesManager::nextFile() {
  char buf[512];
  auto baseNamePattern = fs::join(m_logDir, m_fileNamePattern);
  auto baseNameSize = baseNamePattern.size() - 1;

  m_fileEntries[0]->closeStream();
  for (int i = m_fileEntries.size() - 1; i >= 0; --i) {
    auto& fileEntry = m_fileEntries[i];
    strncpy(buf, baseNamePattern.data(), baseNameSize);
    buf[baseNameSize] = '\0';
    if (fileEntry->name()[baseNameSize] == '.') {
      strcat(buf, "1");
    } else {
      long fileNumber = strtol(fileEntry->name().data() + baseNameSize, 
                               nullptr, 10);
      sprintf(buf + baseNameSize, "%ld", fileNumber + 1);
    }
    strcat(buf, kLogFilesManagerExtension.data());
    fileEntry->rename(buf);
  }

  strncpy(buf, baseNamePattern.data(), baseNameSize);
  buf[baseNameSize] = '\0';
  strcat(buf, kLogFilesManagerExtension.data());
  m_fileEntries.push_front(FileEntry::create(buf));
  m_stream = m_fileEntries[0]->stream();
}

void LogFilesManager::write(const char* data, int64_t size) {
  m_stream->write(data, size);
  m_stream->flush();
  m_limitWatcher.addWritten(size);
}

std::string LogFilesManager::fileNamePattern() const {
  return m_fileNamePattern.substr(0, m_fileNamePattern.size() - 1);
}

}
}
