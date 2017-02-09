#include <string>
#include <vector>
#include <functional>

namespace futils {

bool fileExists(const std::string& name);
int64_t fileSize(const std::string& fileName);
std::vector<char> fileContent(const std::string& fileName);

enum class FileType {
  file,
  dir
};

class TmpDir {
  using FileHandler = std::function<void(const std::string&, FileType)>;

public:
  TmpDir();
  TmpDir(const std::string& fileNamePattern, size_t count);
  void forEachFile(FileHandler handler);
  ~TmpDir();

private:
  void create();
  void populate(size_t count);
  void remove();
  void forEachFile(const std::string& dirName, FileHandler handler);
  void processFile(const std::string& name, FileHandler handler);
  void removeHandler(const std::string& name, FileType type);

private:
  std::string m_dirPath;
  std::string m_fileNamePattern;
};

}