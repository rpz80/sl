#define CATCH_CONFIG_MAIN
#include "catch.hh"
#include <log/log.h>

TEST_CASE("Format") {
  REQUIRE(sl::fmt("% %!", "Hello", "world") == "Hello world!");
  REQUIRE(sl::fmt("% % % \%", 1, std::string("2"), "three") == "1 2 three %");
  REQUIRE(sl::fmt("% % % \%", 1, std::string("2")) == "1 2 % %");
  REQUIRE(sl::fmt("% ", 1, std::string("2")) == "1 "); 
}

