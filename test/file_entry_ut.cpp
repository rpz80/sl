#include <assert.h>
#include <string.h>
#include <algorithm>
#include <iterator>

#include "catch.hh"
#include <log/file_entry.h>
#include <log/utils.h>
#include "file_utils.h"
#include "random_utils.h"

using namespace sl::detail;

class TestWriter {
public:
  TestWriter(IFileStream& stream) 
    : m_stream(stream),
      m_randomData(50, 100) {}

  void writeRandomData(size_t iterations = 500) {
    for (size_t i = 0; i < iterations; ++i) {
      auto data = m_randomData();
      m_stream.write(data.data(), data.size());
      std::copy(data.cbegin(), data.cend(), 
                std::back_inserter(m_expectedContent));
    }
  }

  std::vector<char>  expectedContent() const {
    return m_expectedContent;
  }
private:
  std::vector<char> m_expectedContent;
  IFileStream& m_stream;
  RandomData m_randomData;
};

FileEntryPtr createTestEntry(futils::TmpDir& tmpDir) {
  const std::string kFileName = fs::join(tmpDir.path(), "log_file");

  REQUIRE(futils::fileExists(kFileName) == false);
  FileEntryPtr result(new FileEntry(kFileName));

  REQUIRE(futils::fileExists(kFileName) == false);
  result->open();

  REQUIRE(futils::fileExists(kFileName) == true);
  return result;
}

TEST_CASE("FileStreamTest") {

  futils::TmpDir tmpDir;
  auto fname = fs::join(tmpDir.path(), "log_file");
  FileStream stream(fname);

  REQUIRE(futils::fileExists(fname));
  REQUIRE(stream.isOpened());

  SECTION("CloseTest") {
    stream.close();
    REQUIRE(futils::fileExists(fname));
    REQUIRE(stream.isOpened() == false);
  }

  SECTION("WriteTest") {
    TestWriter tw(stream);
    tw.writeRandomData();
    stream.close();
    auto content = futils::fileContent(fname);
    REQUIRE(content == tw.expectedContent());
  }
}

TEST_CASE("FileEntryGetEntriesTest", "[FileEntry, getEntries]") {
  const size_t kFileCount = 300;
  const std::string kFilePattern = "log_file";

  futils::TmpDir td(kFilePattern, kFileCount);
  FileEntryFactory factory;
  auto entries = factory.getExistent(td.path(), kFilePattern + "*");

  REQUIRE(entries.size() == kFileCount);
}

TEST_CASE("FileEntryGetEntriesFAILTest", "[FileEntry, getEntries]") {
  const std::string kFilePattern = "log_file";
  FileEntryFactory factory;
  REQUIRE_THROWS(factory.getExistent("/not/existing/path", kFilePattern + "*"));
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
