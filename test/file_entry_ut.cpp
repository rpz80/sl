#include <assert.h>
#include <string.h>

#include "catch.hh"
#include <log/file_entry.h>
#include "test_utils.h"

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

bool fileExists(const char* name) {
  struct stat st;
  if (stat(name, &st) != 0)
    return false;
  return true;
}

TEST_CASE("FileEntryTest", "[file_entry]") {
  using namespace sl::detail;

  char nameBuffer[512];
  char contentBuf[256];
  const int fileCount = 10;
  char dirTemplate[] = "/tmp/sl.XXXXXX";
  const char* dirName = populateTestDir(dirTemplate, fileCount);

  auto fileEntries = getFileEntries(dirName, "file*");
  REQUIRE(fileEntries.size() == fileCount);

  /* contents */
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

  /* rename */
  const char* newName = catFileName(nameBuffer, dirName, "newName");
  fileEntries[0]->rename(newName);
  REQUIRE(fileEntries[0]->name() == newName);
  REQUIRE(fileExists(newName));
  REQUIRE(fileEntries[0]->exists());

  /* stream && write */
  const char* stringToWrite = "abcd";
  auto stream = fileEntries[0]->stream();
  stream->write(stringToWrite, 5);
  fileEntries[0]->closeStream();
  FILE* f = fopen(fileEntries[0]->name().data(), "r");
  REQUIRE(f);
  fread(contentBuf, 1, 5, f);
  REQUIRE(strcmp(stringToWrite, contentBuf) == 0);
  fclose(f);

  /* remove */
  fileEntries[0]->remove();
  REQUIRE(!fileExists(fileEntries[0]->name().data()));
  REQUIRE(!fileEntries[0]->exists());

  /* create */
  const char* createdEntry = catFileName(nameBuffer, dirName, "createdEntry");
  REQUIRE(!fileExists(createdEntry));
  auto newFileEntry = FileEntry::create(createdEntry);
  REQUIRE(newFileEntry->exists());
  REQUIRE(newFileEntry->name() == createdEntry);
  REQUIRE(newFileEntry->size() == 0);

  REQUIRE(removeDir(dirName));
}
