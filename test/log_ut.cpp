#include "catch.hh"
#include "test_utils.h"
#include <log/log.h>
#include <log/utils.h>

const int64_t kTotalLimit = 30;
const int64_t kFileLimit = 10;

class TestLogger : public sl::Logger {
public:
  std::string getFileNamePattern(int sinkId) const {
    return sl::Logger::getFileNamePattern(sinkId);
  }

  std::string getDefaultFileNamePattern() const {
    return sl::Logger::getDefaultFileNamePattern();
  }

  std::string getTimeFormat() const {
    return sl::Logger::getTimeFormat();
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

void assertUninitializedState(TestLogger& logger) {
  REQUIRE_THROWS(logger.getDefaultLevel());
  REQUIRE_THROWS(logger.setDefaultLevel(sl::Level::debug));
  REQUIRE_THROWS(logger.setLevel(1, sl::Level::warning));
  REQUIRE_THROWS(logger.getLevel(1));
  REQUIRE_THROWS(logger.getFileNamePattern(1));
  REQUIRE(logger.hasSink(1) == false);
  REQUIRE(logger.hasDefaultSink() == false);
}

void assertDefaultSinkState(TestLogger& logger, 
                            const std::string& fileName, 
                            sl::Level level) {
  REQUIRE(logger.hasDefaultSink() == true);
  REQUIRE(logger.getDefaultFileNamePattern() == fileName);
  REQUIRE(logger.getDefaultLevel() == level);
}

void assertSinksState(TestLogger& logger,
                      const char* dirName,
                      size_t sinkCount,
                      const std::string& fileNamePattern,
                      sl::Level level) {
  for (size_t i = 0; i < sinkCount; ++i) {
    REQUIRE(logger.hasSink(i));
    REQUIRE(logger.getFileNamePattern(i) == fileNamePattern + std::to_string(i));
    REQUIRE(logger.getLevel(i) == level);
  }
  for (size_t i = 0; i < sinkCount; ++i) {
    REQUIRE_THROWS(logger.addSink(i, dirName, 
                                  fileNamePattern + std::to_string(i),
                                  sl::Level::debug, 
                                  kTotalLimit,
                                  kFileLimit,
                                  false));
  }
}

TEST_CASE("Logger") {
  TestLogger logger;
  using namespace sl::detail;

  char dirTemplate[] = "/tmp/sl.XXXXXX";
  const char* dirName = mkdtemp(dirTemplate);
  char buf[512];
  char contentBuf[4096];

  SECTION("Uninitialized test") {
    assertUninitializedState(logger);
  }

  SECTION("Default sink") {
    assertUninitializedState(logger);

    const std::string kFileName("someFileName");
    logger.setDefaultSink(dirName, kFileName,sl::Level::debug,
                          kTotalLimit, kFileLimit, true);

    assertDefaultSinkState(logger, kFileName, sl::Level::debug);

    const auto filePath = fs::join(dirName, str::join(kFileName, ".log"));
    logger.log(sl::Level::info, "% %", "hello world");

    REQUIRE(fileExists(filePath.data()));

    auto fileStrings = splitBy(fileContent(contentBuf, filePath.data()), '\n');
    REQUIRE(!fileStrings.empty());
    checkLogOutput(fileStrings[0], sl::Level::debug, "hello world");
  }

  SECTION("Sinks with Id") {
    assertUninitializedState(logger);

    const size_t kSinksCount = 50;
    const std::string kFileNamePattern = "fileName";
    const sl::Level kSinkLevel = sl::Level::debug;

    for (size_t i = 0; i < kSinksCount; ++i) {
      logger.addSink(i, dirName, kFileNamePattern + std::to_string(i), 
                     kSinkLevel, kTotalLimit, kFileLimit, false);
    }

    assertSinksState(logger, dirName, kSinksCount, 
                     kFileNamePattern, kSinkLevel);

  }

  REQUIRE(removeDir(dirName));
}
