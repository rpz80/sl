#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include <assert.h>
#include <string.h>

#include "catch.hh"
#include <log/file_entry.h>

const char* catFileName(char* buffer, const char* dirName, const char* fname) {
  strcpy(buffer, dirName);
  strcat(buffer, "/");
  strcat(buffer, fname);

  return buffer;
}

const char* getFullFileName(char* buffer, const char* dirName, const char* fname, int index) {
  catFileName(buffer, dirName, fname);
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

    for (int j = 0; j < i + 1; ++j) {
      fwrite("a", 1, 1, f);
    }

    fclose(f);
  }

  return dirName;
}

bool removeDir(const char* dirName) {
  DIR* d;
  struct dirent* entry;
  struct stat st;
  char nameBuf[512];

  if ((d = opendir(dirName)) == nullptr) {
    printf("open dir failed %s\n", dirName);
    return false;
  }

  while ((entry = readdir(d)) != nullptr) {
    if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) {
      continue;
    }
    if (stat(catFileName(nameBuf, dirName, entry->d_name), &st) != 0) {
      printf("stat failed %s\n", nameBuf);
      return false;
    }
    if (S_ISDIR(st.st_mode)) {
      return removeDir(catFileName(nameBuf, dirName, entry->d_name));
    }
    if (remove(catFileName(nameBuf, dirName, entry->d_name)) != 0) {
      printf("remove failed %s\n", nameBuf);
      return false;
    }
  }

  closedir(d);

  return rmdir(dirName) == 0;
}

bool hasFile(const char* dirName, const char* fileName) {
  DIR* d;
  struct dirent* entry;
  struct stat st;  
  char nameBuf[512];

  if ((d = opendir(dirName)) == nullptr) {
    printf("open dir failed %s\n", dirName);
    return false;
  }

  while ((entry = readdir(d)) != nullptr) {
    if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) {
      continue;
    }
    if (strcmp(catFileName(nameBuf, dirName, entry->d_name), fileName) == 0) {
      return true;
    }
  }

  return false;
}

TEST_CASE("FileEntryTest", "[file_entry]") {
  using namespace sl::detail;

  char nameBuffer[512];
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
    REQUIRE(hasFile(dirName, (*fileIt)->name().data()));
  }

  REQUIRE(removeDir(dirName));
}