#include "catch.hh"
#include <log/rotation_limit_watcher.h>
#include <log/rotation_limit_watcher_handler.h>

class TestWatcherHandler : public sl::detail::RotationLimitWatcherHandler {
public:
  virtual int64_t clearNeeded(int64_t spaceToClear) override {

  }

  virtual bool nextFile() override {

  }

  bool clearNeededCalled = false;
  bool nextFileCalled = false;
}

TEST("LimitWatcherTest", "[limit_watcher_test]") {
  using namespace sl::detail;

  TestWatcherHandler handler;
  RotationLimitWatcher watcher(10, 30, &handler);
  watcher.addWritten(5);
  REQUIRE(handler.nextFileCalled == false);
  REQUIRE(handler.clearNeededCalled == false);
}