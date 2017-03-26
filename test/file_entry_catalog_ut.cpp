#include "catch.hh"
#include <log/file_entry_catalog.h>
#include <log/utils.h>
#include "file_utils.h"
#include "test_common.h"

using namespace sl::detail;

TEST_CASE("FileEntryCatalogEmptyTest", "[FileEntryCatalog, empty]") 
{
  TestFileEntryFactory factory(100, 0);
  FileEntryCatalog catalog(&factory, kPath, kBaseName);

  REQUIRE(catalog.size() == 1);
  REQUIRE(catalog.first().name() == "/test/path/test.log");
}

TEST_CASE("FileEntryCatalogTest", "[FileEntryCatalog]")
{
  const size_t kEntriesCount = 3;
  const int64_t kFileSize = 100;
  TestFileEntryFactory factory(kFileSize, kEntriesCount);
  FileEntryCatalog catalog(&factory, kPath, kBaseName);

  REQUIRE(catalog.size() == kEntriesCount);
  REQUIRE(catalog.totalBytes() == kEntriesCount * kFileSize);
  REQUIRE(catalog.first().name() == "/test/path/test.log");
  for (size_t i = 0; i < kEntriesCount; ++i) {
    auto newName = str::join(
        "/test/path/test", 
        (i == 0 ? "" : std::to_string(i)), 
        ".log");
    REQUIRE(factory.get()[i]->name() == newName);
    REQUIRE(factory.get()[i]->size() == kFileSize);
  }

  SECTION("rotate")
  {
    REQUIRE_NOTHROW(catalog.rotate());
    REQUIRE(catalog.size() == kEntriesCount + 1);

    /* entries names should've been +1'ed */
    for (size_t i = 0; i < kEntriesCount; ++i) {
      auto newName = str::join("/test/path/test", 
                               std::to_string(i + 1), 
                               ".log");
      REQUIRE(factory.get()[i]->name() == newName);
    }
  }

  SECTION("remove")
  {
    REQUIRE_NOTHROW(catalog.first());
    for (size_t i = 0; i < kEntriesCount; ++i) {
      int64_t removeResult; 
      REQUIRE_NOTHROW(removeResult = catalog.removeLast());
      REQUIRE(removeResult == kFileSize);
    };
    REQUIRE(catalog.size() == 1);
    REQUIRE(catalog.totalBytes() == 0);
    REQUIRE(catalog.first().name() == "/test/path/test.log");
    REQUIRE(factory.get()[0]->size() == 0);
  }
}
