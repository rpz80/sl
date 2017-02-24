#include <algorithm>
#include <string.h>
#include <log/log_files_manager.h>
#include <log/utils.h>

namespace sl {
namespace detail {

LogFilesManager::LogFilesManager(const std::string& path, 
                               const std::string& baseName,
                               int64_t totalLimit,
                               int64_t fileLimit,
                               FileEntryFactoryPtr factory)
  : m_limitWatcher(totalLimit, fileLimit, this),
    m_factory(std::move(factory)),
    m_catalog(m_factory.get(), path, baseName)
{
  m_stream = m_catalog.first().open();
}

std::string LogFilesManager::baseName() const {
  return m_catalog.baseName();
}
/*

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
  fwrite(data, size, 1, m_stream);
  fflush(m_stream);
  m_limitWatcher.addWritten(size);
}

std::string LogFilesManager::fileNamePattern() const {
  return m_fileNamePattern.substr(0, m_fileNamePattern.size() - 1);
}
*/

}
}
