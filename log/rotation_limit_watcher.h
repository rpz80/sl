#pragma once

#include <cstdint>
#include <log/rotation_limit_watcher_handler.h>

namespace sl {
namespace detail {

class RotationLimitWatcher {
public:
  RotationLimitWatcher(
    int64_t totalLimit, 
    int64_t fileLimit,
    RotationLimitWatcherHandler* watcherHandler);

  void addWritten(int64_t bytesWritten);
  void setSize(int64_t);

private:
  int64_t m_totalLimit;
  int64_t m_fileLimit;
  int64_t m_size;
  RotationLimitWatcherHandler* m_handler;
};

}
}
