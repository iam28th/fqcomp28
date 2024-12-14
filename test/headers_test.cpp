#include "headers.h"
#include <catch2/catch_test_macros.hpp>

using namespace fqzcomp28::headers;
/**
 * headers fields are parsed as expected
 */
TEST_CASE("HeaderFormatSpec") {

  using enum FieldType;

  std::string header1 = "@SRR22543904.1 1 length=150";
  HeaderFormatSpeciciation fmt1(header1);

  REQUIRE(fmt1.n_fields() == 5);
  REQUIRE(fmt1.field_types ==
          std::vector{STRING, NUMERIC, NUMERIC, STRING, NUMERIC});
  REQUIRE(fmt1.separators == std::vector{'.', ' ', ' ', '='});

  std::string header2 =
      "@SRR065390.1000 HWUSI-EAS687_61DAJ:8:1:1174:9158 length=100";
  HeaderFormatSpeciciation fmt2(header2);
  REQUIRE(fmt2.n_fields() == 11);
  REQUIRE(fmt2.field_types == std::vector{STRING, NUMERIC, STRING, STRING,
                                          STRING, NUMERIC, NUMERIC, NUMERIC,
                                          NUMERIC, STRING, NUMERIC});
  REQUIRE(fmt2.separators ==
          std::vector{'.', ' ', '-', '_', ':', ':', ':', ':', ' ', '='});
}
