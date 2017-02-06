#include <assert.h>
#include <string.h>
#include "catch.hh"
#include <log/log_files_manager.h>
#include "test_utils.h"

const char* fileContent(char* buf, const char* fileName) {
  FILE* f;
  int i = 0;

  f = fopen(fileName, "rb");
  assert(f);
  while (fread(buf + i, 1, 1, f) != 0) {
    ++i;
  }
  buf[i] = '\0';
  fclose(f);

  return buf;
}

int64_t fileSize(const char* fileName) {
  struct stat st;
  int result;

  result = stat(fileName, &st);
  assert(result == 0);

  return st.st_size;
}

TEST_CASE("LogFilesManagerTest") {
  using namespace sl::detail;

  char dirTemplate[] = "/tmp/sl.XXXXXX";
  const char* dirName = mkdtemp(dirTemplate);
  const char* fileNamePattern = "log_file";
  const char* testString = "0123456789";
  const int testStringSize = 10;
  char buf[512];

  assert(dirName);
  LogFilesManager logManager(dirName, fileNamePattern, 30, 10);
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file.log")));

  logManager.write(testString, 10);
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file.log")));
  REQUIRE(strcmp(testString, fileContent(buf, catFileName(buf, dirName, "log_file1.log"))) == 0);
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file1.log")));
  REQUIRE(fileSize(catFileName(buf, dirName, "log_file.log")) == 0);

  logManager.write(testString, 10);
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file.log")));
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file1.log")));
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file2.log")));
  REQUIRE(strcmp(testString, fileContent(buf, catFileName(buf, dirName, "log_file1.log"))) == 0);
  REQUIRE(strcmp(testString, fileContent(buf, catFileName(buf, dirName, "log_file2.log"))) == 0);
  REQUIRE(fileSize(catFileName(buf, dirName, "log_file.log")) == 0);

  logManager.write(testString, 10);
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file.log")));
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file1.log")));
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file2.log")));
  REQUIRE(!fileExists(catFileName(buf, dirName, "log_file3.log")));
  REQUIRE(strcmp(testString, fileContent(buf, catFileName(buf, dirName, "log_file1.log"))) == 0);
  REQUIRE(strcmp(testString, fileContent(buf, catFileName(buf, dirName, "log_file2.log"))) == 0);
  REQUIRE(fileSize(catFileName(buf, dirName, "log_file.log")) == 0);

  logManager.write(testString, 5);
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file.log")));
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file1.log")));
  REQUIRE(fileExists(catFileName(buf, dirName, "log_file2.log")));
  REQUIRE(!fileExists(catFileName(buf, dirName, "log_file3.log")));
  REQUIRE(strcmp(testString, fileContent(buf, catFileName(buf, dirName, "log_file1.log"))) == 0);
  REQUIRE(strcmp(testString, fileContent(buf, catFileName(buf, dirName, "log_file2.log"))) == 0);
  REQUIRE(fileSize(catFileName(buf, dirName, "log_file.log")) == 5);
  
  REQUIRE(removeDir(dirName));
}