#include "catch.hh"
#include <log/rotation_limit_watcher.h>
#include <log/rotation_limit_watcher_handler.h>

class TestWatcherHandler : public sl::detail::RotationLimitWatcherHandler {
public:
  virtual int64_t clearNeeded() override {
    clearNeededCalled = true;
    return 10;
  }
  

  virtual void nextFile() override {
    nextFileCalled = true;
  }

  bool clearNeededCalled = false;
  bool nextFileCalled = false;
};

TEST_CASE("LimitWatcherTest", "[limit_watcher_test]") {
  using namespace sl::detail;

  TestWatcherHandler handler;
  REQUIRE_THROWS(RotationLimitWatcher(10, 30, &handler));
  RotationLimitWatcher watcher(30, 10, &handler);

  watcher.addWritten(5);
  REQUIRE(handler.nextFileCalled == false);
  REQUIRE(handler.clearNeededCalled == false);

  watcher.addWritten(4);
  REQUIRE(handler.nextFileCalled == false);
  REQUIRE(handler.clearNeededCalled == false);

  watcher.addWritten(5);
  REQUIRE(handler.nextFileCalled == true);
  REQUIRE(handler.clearNeededCalled == false);
  handler.nextFileCalled = false;

  watcher.addWritten(6);
  REQUIRE(handler.nextFileCalled == true);
  REQUIRE(handler.clearNeededCalled == false);
  handler.nextFileCalled = false;

  watcher.addWritten(11);
  REQUIRE(handler.nextFileCalled == true);
  REQUIRE(handler.clearNeededCalled == true);
  handler.nextFileCalled = false;
  handler.clearNeededCalled = false;

  watcher.addWritten(4);
  REQUIRE(handler.nextFileCalled == false);
  REQUIRE(handler.clearNeededCalled == false);

  watcher.addWritten(6);
  REQUIRE(handler.nextFileCalled == true);
  REQUIRE(handler.clearNeededCalled == true);
}