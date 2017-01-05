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
                               sinkStream, 
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
                        sinkStream,
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
    const std::string kFileName("someFileName");
    sl::Logger::OstreamPtr streamPtr(new std::stringstream);
    logger.setDefaultSink(sl::Level::debug,
                          kFileName,
                          streamPtr,
                          true);
    REQUIRE(logger.hasDefaultSink() == true);
    REQUIRE(logger.getDefaultFileName() == kFileName);
    REQUIRE(logger.getDefaultLevel() == sl::Level::debug);
    logger.removeDefaultSink();
    REQUIRE(logger.hasDefaultSink() == false);
    REQUIRE_THROWS(logger.getDefaultFileName());
    REQUIRE_THROWS(logger.getDefaultLevel());
  }

  SECTION("Sinks with Id") {
    const size_t kSinksCount = 50;
    for (size_t i = 0; i < kSinksCount; ++i) {
      logger.addSink(i, sl::Level::debug, "fileName" + std::to_string(i),
                     sl::Logger::OstreamPtr(new std::stringstream), false);
    }
    for (size_t i = 0; i < kSinksCount; ++i) {
      REQUIRE(logger.hasSink(i));
      REQUIRE(logger.getFileName(i) == "fileName" + std::to_string(i));
      REQUIRE(logger.getLevel(i) == sl::Level::debug);
    }
    for (size_t i = 0; i < kSinksCount; ++i) {
      REQUIRE_THROWS(logger.addSink(i, sl::Level::debug, 
                                    "fileName" + std::to_string(i),
                                    sl::Logger::OstreamPtr(new std::stringstream), 
                                    false));
    }
    for (size_t i = 0; i < kSinksCount; ++i) {
      logger.removeSink(i);
      REQUIRE_NOTHROW(logger.removeSink(i));
    }
    for (size_t i = 0; i < kSinksCount; ++i) {
      REQUIRE(logger.hasSink(i) == false);
      REQUIRE_THROWS(logger.getFileName(i));
      REQUIRE_THROWS(logger.getLevel(i));
    }
  }

  SECTION("logging") {
    REQUIRE_THROWS(logger.log(sl::Level::debug, "% %!", "hello", "world"));
    REQUIRE_THROWS(logger.log(1, sl::Level::warning, "% %!", "hello", "world"));

    const std::string kFileName("someFileName");
    sl::Logger::OstreamPtr defaultStream(new std::stringstream);
    logger.setDefaultSink(sl::Level::debug,
                          kFileName,
                          defaultStream,
                          true);
    const size_t kSinksCount = 50;
    for (size_t i = 0; i < kSinksCount; ++i) {
      logger.addSink(i, sl::Level::debug, "fileName" + std::to_string(i),
                     sl::Logger::OstreamPtr(new std::stringstream), false);
    }
  }
}

