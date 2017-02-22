#include <vector>
#include <log/file_stream.h>
#include <log/utils.h>
#include "random_utils.h"
#include "file_utils.h"
#include "catch.hh"

using namespace sl::detail;

TEST_CASE("FileStreamTest") {

  futils::TmpDir tmpDir;
  auto fname = fs::join(tmpDir.path(), "log_file");
  FileStream stream(fname);

  REQUIRE(futils::fileExists(fname));
  REQUIRE(stream.isOpened());

  SECTION("CloseTest") {
    stream.close();
    REQUIRE(futils::fileExists(fname));
    REQUIRE(stream.isOpened() == false);
  }

  SECTION("WriteTest") {
    futils::TestWriter tw(stream);
    tw.writeRandomData();
    stream.close();
    auto content = futils::fileContent(fname);
    REQUIRE(content == tw.expectedContent());
  }
}
