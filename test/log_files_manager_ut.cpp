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
  const size_t kEntriesCount = 0;
  const int64_t kTotalLimit = 300;
  const int64_t kFileLimit = 100;

  TestFileEntryFactory factory(0, 0);
  FileEntryCatalogPtr catalog(new TestFileEntryCatalog(&factory, kPath, kBaseName));
  TestFileEntryCatalog* catalogPtr = static_cast<TestFileEntryCatalog*>(catalog.get());
  TestLogFilesManager manager(kTotalLimit, kFileLimit, std::move(catalog));

  REQUIRE(manager.baseName() == kBaseName);
  /* default entry should be created */
  REQUIRE(catalogPtr->entries().size() == 1);

  SECTION("Write below file limit") {
    manager.write(nullptr, 70);
    REQUIRE(catalogPtr->entries().size() == 1);
    REQUIRE(static_cast<TestFileStream*>(manager.stream().get())->written == 70);
  }

  SECTION("Write beyond file limit") {
    manager.write(nullptr, 101);
    REQUIRE(catalogPtr->entries().size() == 2);
  }

  SECTION("Write beyond total limit") {
    manager.write(nullptr, 50);
    REQUIRE(catalogPtr->entries().size() == 1);
    
    manager.write(nullptr, 101);
    REQUIRE(catalogPtr->entries().size() == 2);

    manager.write(nullptr, 101);
    REQUIRE(catalogPtr->entries().size() == 3);

    manager.write(nullptr, 101);
    REQUIRE(catalogPtr->entries().size() == 3);
  }
}