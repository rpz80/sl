#include <string>
#include <vector>
#include <functional>
#include <unordered_set>
#include <iterator>
#include <log/utils.h>
#include <log/file_stream.h>
#include "random_utils.h"

using namespace sl::detail;

namespace futils {

template<typename Source>
std::vector<std::string> splitBy(const Source& source, char delim) {
  std::vector<std::string> result;
  std::string tmp;

  for (size_t i = 0; i < source.size(); ++i) {
    if (source[i] == delim) {
      if (!tmp.empty()) { 
        result.push_back(tmp);
        tmp.clear();
      }
    } else {
      tmp.push_back(source[i]);
    }
  }

  if (!tmp.empty())
    result.push_back(tmp);

  return result;
}

bool fileExists(const std::string& name);
int64_t fileSize(const std::string& fileName);
std::vector<char> fileContent(const std::string& fileName);
std::unordered_set<std::string> readAll(const std::string& path, const std::string& baseName);

class TmpDir : public fs::Dir {
  using BaseType = fs::Dir;
public:
  TmpDir();
  TmpDir(const std::string& fileNamePattern, size_t count);
  ~TmpDir();
  
private:
  std::string create();
  void populate(const std::string& fileNamePattern, size_t count);
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
