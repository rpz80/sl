#include <stdexcept>
#include <log/rotation_limit_watcher.h>

namespace sl {
namespace detail {

RotationLimitWatcher::RotationLimitWatcher(
    int64_t totalLimit, 
    int64_t fileLimit,
    RotationLimitWatcherHandler* watcherHandler) :
  m_totalLimit(totalLimit),
  m_fileLimit(fileLimit),
  m_size(0),
  m_handler(watcherHandler) {
  if (m_totalLimit < m_fileLimit) {
    throw std::runtime_error("RotationLimitWatcher: totalLimit < fileLimit");
  } 
}

void RotationLimitWatcher::addWritten(int64_t bytesWritten) {
  int fileCount = m_size / m_fileLimit;
  int newFileCount = (m_size + bytesWritten) / m_fileLimit;

  if (newFileCount != fileCount) {
    m_handler->nextFile();
  }

  m_size += bytesWritten;

  if (m_size >= m_totalLimit) {
    auto clearedSize = m_handler->clearNeeded();
    if (clearedSize != -1) {
      m_size -= clearedSize;
    }
  }
}

void RotationLimitWatcher::setSize(int64_t size) {
  m_size = size;
}

}
}
