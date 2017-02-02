#include "catch.hh"
#include <log/file_entry.h>

#include <unistd.h>

struct TestFile {
  const char* name;
  int64_t size;
};

void populateTestDir(const struct TestFile* testFiles, int testFileCount) {
} 

TEST_CASE("FileEntryTest", "[file_entry, get_entries]") {

}