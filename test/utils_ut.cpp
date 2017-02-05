#include "catch.hh"
#include <log/log.h>
#include <log/utils.h>

TEST_CASE("FormatTest", "[utils, format]") {
  REQUIRE(sl::fmt("% %!", "Hello", "world") == "Hello world!");
  REQUIRE(sl::fmt("% % % \%", 1, std::string("2"), "three") == "1 2 three %");
  REQUIRE(sl::fmt("% % % \%", 1, std::string("2")) == "1 2 % %");
  REQUIRE(sl::fmt("% ", 1, std::string("2")) == "1 "); 
  REQUIRE(sl::fmt("", 1, 2.45) == "");
  REQUIRE(sl::fmt("%%%", 1, 2.45) == "12.45%");
}

TEST_CASE("StrJoinTest", "[utils, join]") {
  using namespace sl::detail;

  REQUIRE(str::join("ab", "cd") == "abcd");
  REQUIRE(str::join(std::string("ab"), std::string("cd"), std::string("ef")) == "abcdef");
  REQUIRE(str::join(std::string("ab"), "cd", "ef") == "abcdef");
  REQUIRE(str::join(std::string("ab"), "cd", "ef", std::string("gh")) == "abcdefgh");
  REQUIRE(str::join(std::string("ab"), "cd", "ef", std::string("")) == "abcdef");
  REQUIRE(str::join(std::string("ab"), "", "ef", std::string("")) == "abef");
}

TEST_CASE("FsGlobTest", "[utils, glob]") {
  using namespace sl::detail;

  REQUIRE(fs::globMatch("file.txt", "*"));
  REQUIRE(fs::globMatch("", "*"));
  REQUIRE(fs::globMatch("file.txt", "*.txt"));
  REQUIRE(fs::globMatch("file.txt", "f[oiu]l?.txt"));
  REQUIRE(fs::globMatch("/some/path/file.txt", "/s*/p??h/[uioef][krri]??.*"));
  REQUIRE(fs::globMatch("/some/path/file.txt", "/s*/p??h/[uioef][krri]???.*") == false);
}

TEST_CASE("FsJoin", "[utils, fs_join]") {
  using namespace sl::detail;

  REQUIRE(fs::join("/home", "user") == "/home/user");
  REQUIRE(fs::join("/home/", "user") == "/home/user");
}
