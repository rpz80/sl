#include <algorithm>
#include <string.h>
#include <log/log_files_manager.h>
#include <log/utils.h>
#include <log/format.h>

namespace sl {
namespace detail {

LogFilesManager::LogFilesManager(int64_t totalLimit,
                                 int64_t fileLimit,
                                 FileEntryCatalogPtr catalog)
  : m_limitWatcher(totalLimit, fileLimit, this),
    m_catalog(std::move(catalog))
{
  m_limitWatcher.setSize(m_catalog->totalBytes());
  m_stream = m_catalog->first().open();
}

std::string LogFilesManager::baseName() const {
  return m_catalog->baseName();
}

int64_t LogFilesManager::clearNeeded() {
  if (m_catalog->empty())
    return 0;

  if (m_stream)
    m_stream->close();

  auto result = m_catalog->removeLast();
  m_stream = m_catalog->first().open();

  return result;
}

void LogFilesManager::nextFile() {
  m_catalog->rotate();
  m_stream->close();
  m_stream = m_catalog->first().open();
}

void LogFilesManager::write(const void* data, size_t size) {
  if (!m_stream || !m_stream->isOpened()) 
    throw std::runtime_error(sl::fmt("% no stream", __FUNCTION__));

  m_stream->write(data, size);
  m_limitWatcher.addWritten(size);
}

}
}
