#include <string>
#include <vector>
#include <functional>
#include <unordered_set>
#include <log/file_stream.h>
#include <log/file_entry.h>
#include <log/utils.h>
#include "random_utils.h"

using namespace sl::detail;

const std::string kPath = "/test/path";
const std::string kBaseName = "test";

class TestFileStream : public IFileStream {
public:
  TestFileStream(int64_t size) : fileSize(size) {}

  virtual void open() override {
    opened = true;
  }

  virtual void close() override {
    opened = false;
  }

  virtual void write(const void* /*data*/, size_t size) override {
    written += size;
  }

  virtual bool isOpened() const override {
    return opened;
  }

  bool opened = true;
  int64_t written = 0;
  int64_t fileSize;
};

class TestFileEntry : public IFileEntry {
public:
  TestFileEntry(const std::string& name, int64_t fileSize) 
    : m_name(name),
      m_fileSize(fileSize) {}

  virtual void remove() override { m_removed = true; }
  virtual void rename(const std::string& newName) override {
    m_name = newName;
  }
  virtual std::string name() const override { return m_name; }
  virtual int64_t size() const override { return m_removed ? 0ll : m_fileSize; }

  virtual bool exists() const override { return false; }
  virtual FileStreamPtr open() override { return FileStreamPtr(new TestFileStream(m_fileSize)); }

private:
  std::string m_name;
  int64_t m_fileSize;
  bool m_removed = false;
};

class TestFileEntryFactory : public IFileEntryFactory {
public:
  TestFileEntryFactory(int64_t entrySize, size_t count)
    : m_count(count),
      m_entrySize(entrySize) {}

  virtual FileEntryPtr create(const std::string& path, 
                              const std::string& baseName,
                              size_t index = 0) override {
    return createEntry(0);
  }

  virtual FileEntryList getExistent(
      const std::string& path, 
      const std::string& baseName) override 
  {
    FileEntryList result;
    for (size_t i = 0; i < m_count; ++i) {
      auto newEntry = createEntry(i);
      m_entries.push_back(newEntry.get());
      result.push_back(std::move(newEntry));
    }

    return result;
  }

std::vector<IFileEntry*> get() const {
  return m_entries;
}

private:
  FileEntryPtr createEntry(size_t i) {
    return FileEntryPtr(
                new TestFileEntry(
                    str::join(fs::join(kPath, kBaseName), 
                              (i == 0 ? "" : std::to_string(i)),
                              ".log"),
                    m_entrySize));
  }
private:
  size_t m_count;
  int64_t m_entrySize;
  std::vector<IFileEntry*> m_entries;
};



namespace futils {

bool fileExists(const std::string& name);
int64_t fileSize(const std::string& fileName);
std::vector<char> fileContent(const std::string& fileName);
std::unordered_set<std::string> readAll(const std::string& path, const std::string& baseName);

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