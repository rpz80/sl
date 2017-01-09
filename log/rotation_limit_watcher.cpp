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
  m_watcherHandler(watcherHandler) {}

bool RotationLimitWatcher::processTotalLimitOverflow(int64_t bytesWritten) {
  if (m_size > m_totalLimit) {
    auto clearedSize = 
        m_watcherHandler->clearNeeded(m_size - m_totalLimit);
    if (clearedSize != -1) {
      m_size -= clearedSize;
    }
    return true;
  }
  return false;
}

void RotationLimitWatcher::processFileLimitOverflow(int64_t oldSize) {
  if (oldSize / m_fileLimit != m_size / m_fileLimit) {
    m_watcherHandler->nextFile();
  }
}

void RotationLimitWatcher::addWritten(int64_t bytesWritten) {
  auto oldSize = m_size;
  m_size += bytesWritten;
  if (!processTotalLimitOverflow(bytesWritten)) {
    processFileLimitOverflow(oldSize);
  }
}

}
}
