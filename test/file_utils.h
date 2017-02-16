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
  ~TmpDir();
  
  void forEachFile(FileHandler handler);
  std::string path() const;

private:
  void create();
  void populate(const std::string& fileNamePattern, size_t count);
  void remove();
  void forEachFile(const std::string& dirName, FileHandler handler);
  void processFile(const std::string& name, FileHandler handler);
  void removeHandler(const std::string& name, FileType type);

private:
  std::string m_dirPath;
};

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

}