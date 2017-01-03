#define CATCH_CONFIG_MAIN
#include "catch.hh"
#include <log/log.h>

TEST_CASE("Format") {
  REQUIRE(sl::fmt("% %!", "Hello", "world") == "Hello world!");
  REQUIRE(sl::fmt("% % % \%", 1, std::string("2"), "three") == "1 2 three %");
  REQUIRE(sl::fmt("% % % \%", 1, std::string("2")) == "1 2 % %");
  REQUIRE(sl::fmt("% ", 1, std::string("2")) == "1 "); 
  REQUIRE(sl::fmt("", 1, 2.45) == "");
  REQUIRE(sl::fmt("%%%", 1, 2.45) == "12.45%");
}

class TestLogger : public sl::Logger {
public:
  void setDefaultSink(sl::Level level,
                      const std::string& fileName,
                      OstreamPtr sinkStream,
                      bool duplicateToStdout) {
    sl::Logger::setDefaultSink(level, 
                               fileName, 
                               std::move(sinkStream), 
                               duplicateToStdout);
  }

  void addSink(int sinkId, 
               sl::Level level,
               const std::string& fileName,
               OstreamPtr sinkStream,
               bool duplicateToStdout) {
    sl::Logger::addSink(sinkId,
                        level,
                        fileName,
                        std::move(sinkStream),
                        duplicateToStdout);
  }
};

TEST_CASE("Logger") {
  TestLogger logger;
  std::stringstream out;

  SECTION("Uninitialized test") {
    REQUIRE_THROWS(logger.getDefaultLevel());
    REQUIRE_THROWS(logger.getDefaultFileName());
    REQUIRE_THROWS(logger.setDefaultLevel(sl::Level::debug));
    REQUIRE_THROWS(logger.setLevel(1, sl::Level::warning));
    REQUIRE_THROWS(logger.getLevel(1));
    REQUIRE_NOTHROW(logger.removeDefaultSink());
    REQUIRE_NOTHROW(logger.removeSink(1));
    REQUIRE_THROWS(logger.getFileName(1));
    REQUIRE(logger.hasSink(1) == false);
    REQUIRE(logger.hasDefaultSink() == false);
  }

  SECTION("Default sink") {
  }
}

