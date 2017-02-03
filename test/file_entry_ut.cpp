#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include <assert.h>
#include <string.h>

#include "catch.hh"
#include <log/file_entry.h>


const char* getFullFileName(char* buffer, const char* dirName, const char* fname, int index) {
  strcpy(buffer, dirName);
  strcat(buffer, "/");
  strcat(buffer, "file");
  sprintf(buffer + strlen(buffer), "%d", index);

  return buffer;
}

const char* populateTestDir(char* dirTemplate, int testFileCount) {
  const char* dirName = mkdtemp(dirTemplate);
  char nameBuffer[512];
  const char* fullFileName;
  FILE* f;

  assert(dirName);
  for (int i = 0; i < testFileCount; ++i) {
    fullFileName = getFullFileName(nameBuffer, dirName, "file", i);
    f = fopen(fullFileName, "wb");
    assert(f);

    for (int j = 0; j <= i + 1; ++j)
      fwrite("a", 1, 1, f);

    fclose(f);
  }

  return dirName;
}

bool removeDir(const char* dirName) {
  DIR* d;
  struct dirent* entry;
  struct stat st;

  if ((d = opendir(dirName)) == nullptr) {
    return false;
  }

  while ((entry = readdir(d)) != nullptr) {
    if (!stat(entry->d_name, &st))
      return false;
    if (S_ISDIR(st.st_mode))
      return removeDir(entry->d_name);
    if (!remove(entry->d_name))
      return false;
  }

  closedir(d);

  return rmdir(dirName) == 0;
}

TEST_CASE("FileEntryTest", "[file_entry, get_entries]") {
  using namespace sl::detail;

  char nameBuffer[512];
  const int fileCount = 10;
  char dirTemplate[] = "/tmp/sl.XXXXXX";
  const char* dirName = populateTestDir(dirTemplate, fileCount);

  auto fileEntries = getFileEntries(dirName, "file*");
  REQUIRE(fileEntries.size() == fileCount);

  for (int i = 0; i < fileCount; ++i) {
    printf("file: %s\n", fileEntries[i]->name().data());
    // REQUIRE(fileEntries[i]->name() == getFullFileName(nameBuffer, dirName, "file", i));
  }

  REQUIRE(removeDir(dirName));
}