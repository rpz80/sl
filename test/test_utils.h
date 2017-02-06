#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

bool removeDir(const char* dirName);
bool fileExists(const char* name);
const char* catFileName(char* buffer, const char* dirName, const char* fname);
const char* fileContent(char* buf, const char* fileName);