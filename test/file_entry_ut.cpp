#include <assert.h>
#include <string.h>
#include <algorithm>
#include <iterator>

#include "catch.hh"
#include <log/file_entry.h>
#include <log/utils.h>
#include "file_utils.h"

using namespace sl::detail;

FileEntryPtr createTestEntry(futils::TmpDir& tmpDir) {
  const std::string kFileName = fs::join(tmpDir.path(), "log_file");

  REQUIRE(futils::fileExists(kFileName) == false);
  FileEntryPtr result(new FileEntry(kFileName));

  REQUIRE(futils::fileExists(kFileName) == false);
  result->open();

  REQUIRE(futils::fileExists(kFileName) == true);
  return result;
}

TEST_CASE("FileEntryFactoryCreateTest", "[FileEntry, createEntry]") {
  FileEntryFactory fef;
  auto entry = fef.create("/some/path/", "log_file", 32);
  REQUIRE(entry->name() == "/some/path/log_file32.log");

  entry = fef.create("/some/path/", "log_file");
  REQUIRE(entry->name() == "/some/path/log_file.log");

  entry = fef.create("/some/path", "log_file", 42);
  REQUIRE(entry->name() == "/some/path/log_file42.log");

  entry = fef.create("/some/path", "log_file");
  REQUIRE(entry->name() == "/some/path/log_file.log");
}

TEST_CASE("FileEntryFactoryGetEntriesTest", "[FileEntry, getEntries]") {
  const size_t kFileCount = 300;
  const std::string kFilePattern = "log_file";

  futils::TmpDir td(kFilePattern, kFileCount);
  FileEntryFactory factory;
  auto entries = factory.getExistent(td.path(), kFilePattern);

  REQUIRE(entries.size() == kFileCount);
}

TEST_CASE("FileEntryFactoryGetEntriesFAILTest", "[FileEntry, getEntries]") {
  const std::string kFilePattern = "log_file";
  FileEntryFactory factory;
  REQUIRE_THROWS(factory.getExistent("/not/existing/path", kFilePattern));
}

TEST_CASE("FileEntryRemoveTest", "[FileEntry, remove]") {
  futils::TmpDir tmpDir;
  auto testEntry = createTestEntry(tmpDir);
  testEntry->remove();
  REQUIRE(futils::fileExists(testEntry->name()) == false);
}

TEST_CASE("FileEntryRenameTest", "[FileEntry, rename]") {
  futils::TmpDir tmpDir;
  auto testEntry = createTestEntry(tmpDir);
  const std::string kNewName = testEntry->name() + "_new";

  testEntry->rename(kNewName);
  REQUIRE(testEntry->name() == kNewName);
  REQUIRE(futils::fileExists(testEntry->name()) == true);
}

TEST_CASE("FileEntryNameTest", "[FileEntry, name]") {
  futils::TmpDir tmpDir;
  auto testEntry = createTestEntry(tmpDir);
  REQUIRE(futils::fileExists(testEntry->name()) == true);
}

TEST_CASE("FileEntrySizeTest", "[FileEntry, size]") {
  futils::TmpDir tmpDir;
  auto testEntry = createTestEntry(tmpDir);
  auto stream = testEntry->open();

  TestWriter tw(*stream);
  tw.writeRandomData();
  stream->close();

  REQUIRE(futils::fileExists(testEntry->name()) == true);
  REQUIRE(futils::fileSize(testEntry->name()) == tw.expectedContent().size());
  REQUIRE(testEntry->size() == tw.expectedContent().size());
}

TEST_CASE("FileEntryExistsTest", "[FileEntry, exists]") {
  futils::TmpDir tmpDir;
  auto testEntry = createTestEntry(tmpDir);

  REQUIRE(testEntry->exists() == true);
  testEntry->remove();
  REQUIRE(testEntry->exists() == false);
}
