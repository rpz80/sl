#pragma once 

#include <cstdint>

namespace sl {
namespace detail {

class RotationLimitWatcherHandler {
public:
  virtual int64_t clearNeeded() = 0;
  virtual void nextFile() = 0;
};

}
}
