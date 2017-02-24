#include "catch.hh"
#include <log/file_entry_catalog.h>
#include <log/utils.h>

using namespace sl::detail;

class TestFileEntry : public IFileEntry {
public:
  TestFileEntry(const std::string& name) 
    : m_name(name) {}

  virtual void remove() override {}
  virtual void rename(const std::string& newName) override {
    m_name = newName;
  }
  virtual std::string name() const override { return m_name; }
  virtual int64_t size() const override { return 0ll; }
  virtual bool exists() const override { return false; }
  virtual FileStreamPtr open() override { return nullptr; }

private:
  std::string m_name;
};

namespace {
const std::string kPath = "/test/path";
const std::string kBaseName = "test";
}

class TestFileEntryFactory : public IFileEntryFactory {
public:
  TestFileEntryFactory(size_t count = 0)
    : m_count(count) {}

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
                              ".log")));
  }
private:
  size_t m_count;
  std::vector<IFileEntry*> m_entries;
};

TEST_CASE("FileEntryCatalogEmptyTest", "[FileEntryCatalog, empty]") 
{
  TestFileEntryFactory factory;
  FileEntryCatalog catalog(&factory, kPath, kBaseName);

  REQUIRE(catalog.first().name() == "/test/path/test.log");
  catalog.removeLast();

  REQUIRE_THROWS(catalog.first());
  REQUIRE_THROWS(catalog.removeLast());
}

TEST_CASE("FileEntryCatalogTest", "[FileEntryCatalog]")
{
  const size_t kEntriesCount = 3;
  TestFileEntryFactory factory(kEntriesCount);
  FileEntryCatalog catalog(&factory, kPath, kBaseName);

  SECTION("rotate")
  {
    REQUIRE(catalog.first().name() == "/test/path/test.log");
    for (size_t i = 0; i < kEntriesCount; ++i) {
      auto newName = str::join(
          "/test/path/test", 
          (i == 0 ? "" : std::to_string(i)), 
          ".log");
      REQUIRE(factory.get()[i]->name() == newName);
    }

    REQUIRE_NOTHROW(catalog.rotate());

    /* entries names should've been +1'ed */
    for (size_t i = 0; i < kEntriesCount; ++i) {
      auto newName = str::join("/test/path/test", 
                               std::to_string(i + 1), 
                               ".log");
      REQUIRE(factory.get()[i]->name() == newName);
    }
  }

  SECTION("remove")
  {
    REQUIRE_NOTHROW(catalog.first());
    for (size_t i = 0; i < kEntriesCount; ++i) {
      REQUIRE_NOTHROW(catalog.removeLast());
    }
    REQUIRE_THROWS(catalog.removeLast());
  }
}
