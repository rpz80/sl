#define CATCH_CONFIG_MAIN
#include "catch.hh"
#include <log/log.h>
#include <log/utils.h>

TEST_CASE("Format") {
  REQUIRE(sl::fmt("% %!", "Hello", "world") == "Hello world!");
  REQUIRE(sl::fmt("% % % \%", 1, std::string("2"), "three") == "1 2 three %");
  REQUIRE(sl::fmt("% % % \%", 1, std::string("2")) == "1 2 % %");
  REQUIRE(sl::fmt("% ", 1, std::string("2")) == "1 "); 
  REQUIRE(sl::fmt("", 1, 2.45) == "");
  REQUIRE(sl::fmt("%%%", 1, 2.45) == "12.45%");
}

TEST_CASE("Utils", "join") {
  using namespace sl::detail;

  REQUIRE(str::join("ab", "cd") == "abcd");
  REQUIRE(str::join(std::string("ab"), std::string("cd"), std::string("ef")) == "abcdef");
  REQUIRE(str::join(std::string("ab"), "cd", "ef") == "abcdef");
  REQUIRE(str::join(std::string("ab"), "cd", "ef", std::string("gh")) == "abcdefgh");
}
/*
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

std::vector<std::string> splitBy(const std::string& source, char delim) {
  std::vector<std::string> result;
  std::string tmp;

  for (size_t i = 0; i < source.size(); ++i) {
    if (source[i] == delim) {
      if (!tmp.empty()) { 
        result.push_back(tmp);
        tmp.clear();
      }
    } else {
      tmp.push_back(source[i]);
    }
  }

  return result;
}

void checkLogLevel(sl::Level level, const std::string& levelLine) {
  switch (level) {
    case sl::Level::debug:
      REQUIRE(levelLine.find("DEBUG") != std::string::npos);
      break;
    case sl::Level::info:
      REQUIRE(levelLine.find("INFO") != std::string::npos);
      break;
    case sl::Level::warning:
      REQUIRE(levelLine.find("WARNING") != std::string::npos);
      break;
    case sl::Level::error:
      REQUIRE(levelLine.find("ERROR") != std::string::npos);
      break;
    case sl::Level::critical:
      REQUIRE(levelLine.find("CRITICAL") != std::string::npos);
      break;
  }
}

void checkMessage(const std::vector<std::string>& logParts, const std::string& message) {
  for (size_t i = 4; i < logParts.size(); ++i) {
    REQUIRE(message.find(logParts[i]) != std::string::npos);
  }
}

void checkLogOutput(const std::string& logLine, sl::Level level, const std::string& message) {
  auto logLineParts = splitBy(logLine, ' ');
  REQUIRE(logLineParts.size() >= 5);
  checkLogLevel(level, logLineParts[2]);
  checkMessage(logLineParts, message);
}

void assertUninitializedState(sl::Logger& logger) {
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

void assertDefaultSinkState(TestLogger& logger, 
                            const std::string& fileName, 
                            sl::Level level) {
  REQUIRE(logger.hasDefaultSink() == true);
  REQUIRE(logger.getDefaultFileName() == fileName);
  REQUIRE(logger.getDefaultLevel() == level);
}

void assertSinksState(TestLogger& logger,
                      size_t sinkCount,
                      const std::string& fileNamePattern,
                      sl::Level level) {
  for (size_t i = 0; i < sinkCount; ++i) {
    REQUIRE(logger.hasSink(i));
    REQUIRE(logger.getFileName(i) == fileNamePattern + std::to_string(i));
    REQUIRE(logger.getLevel(i) == level);
  }
  for (size_t i = 0; i < sinkCount; ++i) {
    REQUIRE_THROWS(logger.addSink(i, sl::Level::debug, 
                                  fileNamePattern + std::to_string(i),
                                  sl::Logger::OstreamPtr(new std::stringstream), 
                                  false));
  }
}

TEST_CASE("Logger") {
  TestLogger logger;

  SECTION("Uninitialized test") {
    assertUninitializedState(logger);
  }

  SECTION("Default sink") {
    assertUninitializedState(logger);

    const std::string kFileName("someFileName");
    sl::Logger::OstreamPtr streamPtr(new std::stringstream);
    logger.setDefaultSink(sl::Level::debug,
                          kFileName,
                          streamPtr,
                          true);

    assertDefaultSinkState(logger, kFileName, sl::Level::debug);
    logger.removeDefaultSink();
    assertUninitializedState(logger);
  }

  SECTION("Sinks with Id") {
    assertUninitializedState(logger);

    const size_t kSinksCount = 50;
    const std::string kFileNamePattern = "fileName";
    const sl::Level kSinkLevel = sl::Level::debug;

    for (size_t i = 0; i < kSinksCount; ++i) {
      logger.addSink(i, kSinkLevel, kFileNamePattern + std::to_string(i),
                     sl::Logger::OstreamPtr(new std::stringstream), false);
    }

    assertSinksState(logger, kSinksCount, kFileNamePattern, kSinkLevel);

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
    using SstreamPtr = std::shared_ptr<std::stringstream>;

    REQUIRE_THROWS(logger.log(sl::Level::debug, "% %!", "hello", "world"));
    REQUIRE_THROWS(logger.log(1, sl::Level::warning, "% %!", "hello", "world"));

    const std::string kFileName("someFileName");
    SstreamPtr defaultStream(new std::stringstream);
    logger.setDefaultSink(sl::Level::debug,
                          kFileName,
                          defaultStream,
                          true);

    const size_t kSinksCount = 50;
    std::vector<SstreamPtr> streams;
    streams.reserve(kSinksCount);

    for (size_t i = 0; i < kSinksCount; ++i) {
      streams.emplace_back(new std::stringstream);
      logger.addSink(i, sl::Level::debug, "fileName" + std::to_string(i),
                     streams[i], true);
    }

    const std::string kLogMessage("hello world!");

    SECTION("messages") {
      REQUIRE_NOTHROW(logger.log(sl::Level::debug, "% %!", "hello", "world"));
      checkLogOutput(defaultStream->str(), sl::Level::debug, kLogMessage);
      defaultStream->str("");

      REQUIRE_NOTHROW(logger.log(sl::Level::warning, "% %!", "hello", "world"));
      checkLogOutput(defaultStream->str(), sl::Level::warning, kLogMessage);
      defaultStream->str("");

      REQUIRE_NOTHROW(logger.log(sl::Level::critical, "% %!", "hello", "world"));
      checkLogOutput(defaultStream->str(), sl::Level::critical, kLogMessage);
    }
  }
}

TEST_CASE("Logging macros") {
}
*/

