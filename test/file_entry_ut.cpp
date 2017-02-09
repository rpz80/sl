#include <random>
#include <assert.h>
#include <string.h>

#include "catch.hh"
#include <log/file_entry.h>
#include <log/utils.h>
#include "file_utils.h"

using namespace sl::detail;

class WriterTest {
public:
  WriterTest(FileStream& stream) 
    : m_stream(stream),
      m_gen(m_device()) {}

  void writeRandomData(size_t iterations = 500) {

  }

  std::vector<char>  expectedContent() const {
    return m_expectedContent;
  }
private:
  std::vector<char> genRandomData() {

  }
private:
  std::vector<char> m_expectedContent;
  FileStream& m_stream;
  std::random_device m_device;
  std::mt19937 m_gen;
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
    stream.write("hello ", 5);
    stream.write("world", 4);
    stream.close();
    auto content = futils::fileContent(fname);
    REQUIRE(content.size() == 9);
  }
}
/*
TEST_CASE("FileEntryTest", "[file_entry]") {
  using namespace sl::detail;

  char nameBuffer[512];
  char contentBuf[256];
  const int fileCount = 10;
  char dirTemplate[] = "/tmp/sl.XXXXXX";
  const char* dirName = populateTestDir(dirTemplate, fileCount);

  auto fileEntries = getFileEntries(dirName, "file*");
  REQUIRE(fileEntries.size() == fileCount);

  for (int i = 0; i < fileCount; ++i) {
    printf("file: %s\n", fileEntries[i]->name().data());
    auto fileIt = std::find_if(fileEntries.cbegin(), fileEntries.cend(), [&] (const FileEntryPtr& fileEntry) {
      return fileEntry->name() == getFullFileName(nameBuffer, dirName, "file", i);
    });
    REQUIRE(fileIt != fileEntries.cend());
    REQUIRE((*fileIt)->size() == i + 1);
    REQUIRE(fileExists((*fileIt)->name().data()));
    REQUIRE((*fileIt)->exists());
  }

  const char* newName = catFileName(nameBuffer, dirName, "newName");
  fileEntries[0]->rename(newName);
  REQUIRE(fileEntries[0]->name() == newName);
  REQUIRE(fileExists(newName));
  REQUIRE(fileEntries[0]->exists());

  const char* stringToWrite = "xbcd";
  int stringToWriteLen = strlen(stringToWrite);
  auto stream = fileEntries[0]->stream();
  REQUIRE(stream);
  fwrite(stringToWrite, stringToWriteLen, 1, stream);
  fileEntries[0]->closeStream();

  FILE* f = fopen(fileEntries[0]->name().data(), "r");
  REQUIRE(f);
  fread(contentBuf, 5, 1, f);
  int contentLen = strlen(contentBuf);
  for (int i = stringToWriteLen - 1; i >= 0; --i) {
    REQUIRE(contentBuf[i + (contentLen - stringToWriteLen)] == stringToWrite[i]);
  }
  fclose(f);

  fileEntries[0]->remove();
  REQUIRE(!fileExists(fileEntries[0]->name().data()));
  REQUIRE(!fileEntries[0]->exists());

  const char* createdEntry = catFileName(nameBuffer, dirName, "createdEntry");
  REQUIRE(!fileExists(createdEntry));
  auto newFileEntry = FileEntry::create(createdEntry);
  REQUIRE(newFileEntry->stream());
  REQUIRE(newFileEntry->exists());
  REQUIRE(newFileEntry->name() == createdEntry);
  REQUIRE(newFileEntry->size() == 0);

  REQUIRE(removeDir(dirName));
}
*/