#include <assert.h>
#include <string.h>
#include "catch.hh"
#include <log/log_files_manager.h>
#include <log/file_entry.h>
#include "file_utils.h"

using namespace sl::detail;

class TestLogFilesManager : public LogFilesManager {
public:
  using LogFilesManager::LogFilesManager;
  const FileStreamPtr& stream() const { return LogFilesManager::stream(); }
};

class TestFileEntryCatalog : public FileEntryCatalog {
public:
  using FileEntryCatalog::FileEntryCatalog;
  const FileEntryList& entries() const { return FileEntryCatalog::entries(); }
};

TEST_CASE("LogFilesManager") {
  const size_t kEntriesCount = 3;
  const int64_t kTotalLimit = 100;
  const int64_t kFileLimit = 50;

  TestFileEntryFactory factory(kEntriesCount);
  FileEntryCatalogPtr catalog(new TestFileEntryCatalog(&factory, kPath, kBaseName));
  TestFileEntryCatalog* catalogPtr = static_cast<TestFileEntryCatalog*>(catalog.get());
  TestLogFilesManager manager(kTotalLimit, kFileLimit, std::move(catalog));

  REQUIRE(manager.baseName() == kBaseName);

  SECTION("Write below file limit") {
    manager.write(nullptr, 30);
    REQUIRE(catalogPtr->entries().size() == kEntriesCount);
    REQUIRE(static_cast<TestFileStream*>(manager.stream().get())->written == 30);
  }

  SECTION("Write beyond file limit") {
    manager.write(nullptr, 51);
    REQUIRE(catalogPtr->entries().size() == kEntriesCount + 1);
  }
}