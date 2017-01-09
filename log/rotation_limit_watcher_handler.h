#pragma once 

namespace sl {
namespace detail {

class RotationLimitWatcherHandler {
public:
  virtual int64_t clearNeeded(int64_t spaceToClear) = 0;
  virtual bool nextFile() = 0;
};

}
}
