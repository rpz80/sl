#pragma once

#include <cstdint>
#include <log/abstract_types.h>

namespace sl {
namespace detail {

class RotationLimitWatcher {
public:
  RotationLimitWatcher(
    int64_t totalLimit, 
    int64_t fileLimit,
    RotationWatcherHandler* watcherHandler);

  void addWritten(int64_t bytesWritten);

private:
  bool processTotalLimitOverflow(int64_t bytesWritten);
  void processFileLimitOverflow(int64_t oldSize);

private:
  int64_t m_totalLimit;
  int64_t m_fileLimit;
  int64_t m_size;
  RotationWatcherHandler* m_watcherHandler;
};

}
}
