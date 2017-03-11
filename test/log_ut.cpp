#include "catch.hh"
#include "file_utils.h"
#include <log/log.h>
#include <log/utils.h>
#include <unordered_map>
#include <thread>
#include <mutex>
#include "random_utils.h"

const int64_t kTotalLimit = 10000;
const int64_t kFileLimit = 300;

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

using LogDataMap = std::unordered_map<std::string, bool>;

void randomLogCheck(int threadCount, int messageCount, 
                    const std::string& dirPath, const std::string& baseName, 
                    sl::Level level, int64_t fileLimit, int64_t totalLimit)
{
  auto& logger = TestLogger::getLogger();

  LogDataMap memoryData;
  std::mutex mutex;
  std::vector<std::thread> threads;
  std::set<std::string> strippedLogData;

  auto sinkBaseName = [&baseName](int id) { return baseName + "_" + std::to_string(id); };

  /* init sinks */
  for (int i = 0; i < threadCount; ++i) {
    if (i != 0) {
      logger.addSink(i, dirPath, sinkBaseName(i), 
                    level, totalLimit, fileLimit, false);
    } else {
      logger.setDefaultSink(dirPath, sinkBaseName(i), level,
                            totalLimit, fileLimit, false);
    }
  }

  /* write random messages to log and to the logData from random thread */
  for (int i = 0; i < threadCount; ++i) {
    threads.emplace_back(std::thread([&] {
      RandomData randomDataGen(20, 100);
      thread_local std::random_device randDevice;
      thread_local std::mt19937 randGen(randDevice());
      std::uniform_int_distribution<> levelDist((int)sl::Level::debug, (int)sl::Level::critical);
      std::uniform_int_distribution<> sinkDist(0, threadCount - 1);

      for (int i = 0; i < messageCount; ++i) {
        auto writeLogLevel = levelDist(randGen);
        auto logMessage = randomDataGen();
        {
          std::lock_guard<std::mutex> lock(mutex);
          /* not all messages should be logged (based on the sink log level) */
          if ((int)level <= writeLogLevel) {
            memoryData.emplace(logMessage, true);
          } else {
            memoryData.emplace(logMessage, false);
          }
        }

        auto sinkId = sinkDist(randDevice);
        if (sinkId == 0) {
          LOG((sl::Level)writeLogLevel, "%", logMessage);
        } else {
          LOG_S(sinkId, (sl::Level)writeLogLevel, "%", logMessage);
        }
      }
    }));
  }

  for (int i = 0; i < threadCount; ++i) {
    threads[i].join();
  }

  /* read logged data */
  for (int i = 0; i < threadCount; ++i) {
    auto actualLoggedData = futils::readAll(dirPath, sinkBaseName(i));
    std::lock_guard<std::mutex> lock(mutex);
    for (auto& logDataString: actualLoggedData) {
      auto logStringSplits = futils::splitBy(logDataString, ' ');
      strippedLogData.emplace(logStringSplits[4]);
    }
  }

  /* Compare results */
  /* all actual logged messages should be found in memory data */
  for (auto it = strippedLogData.cbegin(); it != strippedLogData.cend(); ++it) {
    auto memoryDataIt = memoryData.find(*it);
    REQUIRE(memoryDataIt != memoryData.cend());
    REQUIRE(memoryDataIt->second == true);
    REQUIRE(memoryDataIt->first == *it);
  }

  /* but not all memory messages should have been actually logged */
  for (auto it = memoryData.cbegin(); it != memoryData.cend(); ++it) {
    auto strippedDataIt = strippedLogData.find(it->first);
    if (it->second) {
      REQUIRE(strippedDataIt != strippedLogData.cend());
      REQUIRE(*strippedDataIt == it->first);
    } else {
      REQUIRE(strippedDataIt == strippedLogData.cend());
    }
  }
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
  auto logLineParts = futils::splitBy(logLine, ' ');
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

  futils::TmpDir tmpDir;

  SECTION("Uninitialized test") {
    assertUninitializedState(logger);
  }

  SECTION("Default sink") {
    assertUninitializedState(logger);

    const std::string kFileName("someFileName");
    logger.setDefaultSink(tmpDir.path().c_str(), kFileName,sl::Level::debug,
                          kTotalLimit, kFileLimit, true);

    assertDefaultSinkState(logger, kFileName, sl::Level::debug);

    const auto filePath = fs::join(tmpDir.path(), str::join(kFileName, ".log"));
    logger.log(sl::Level::info, "% %", "hello", "world");

    REQUIRE(futils::fileExists(filePath));

    auto content = futils::fileContent(filePath);
    auto fileStrings = futils::splitBy(content, '\n');
    REQUIRE(!fileStrings.empty());
    checkLogOutput(fileStrings[0], sl::Level::info, "hello world");
  }

  SECTION("Sinks with Id") {
    assertUninitializedState(logger);

    const size_t kSinksCount = 50;
    const std::string kFileNamePattern = "fileName";
    const sl::Level kSinkLevel = sl::Level::debug;

    for (size_t i = 0; i < kSinksCount; ++i) {
      logger.addSink(i, tmpDir.path(), kFileNamePattern + std::to_string(i), 
                     kSinkLevel, kTotalLimit, kFileLimit, false);
    }

    assertSinksState(logger, tmpDir.path().c_str(), kSinksCount, 
                     kFileNamePattern, kSinkLevel);

  }
}

TEST_CASE("LogMacros") {
  futils::TmpDir tmpDir;

  SECTION("Contents check default sink") {
    randomLogCheck(1, 1000, tmpDir.path(), "log_file", 
                   sl::Level::debug, 5000, 1000 * 1000);
  }

  SECTION("Contents check multiple sinks, multiple threads") {
    randomLogCheck(3, 1000, tmpDir.path(), "log_file", 
                   sl::Level::debug, 5000, 1000 * 1000);
  }
}


