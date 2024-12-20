#include "headers.h"
#include <catch2/catch_test_macros.hpp>

using namespace fqzcomp28::headers;

FieldStorageOut convertToOutStorage(FieldStorageIn &&in) {
  FieldStorageOut out;
  out.isDifferentFlag = std::move(in.isDifferentFlag);
  out.content = std::move(in.content);
  out.contentLength = std::move(in.contentLength);
  return out;
}

/**
 * split a header into fields and deduce their types
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

/**
 * STRING field storage
 */
TEST_CASE("{store,load}String") {
  const std::string first = "EAS687";

  const std::vector<std::string> values = {
      "EAS687", "EAS688", "EAS688", "TI OBDUREN", "TI OBDUREN", "BEZNOGIM"};

  FieldStorageIn in;
  std::string_view prev = first;
  for (const auto &s : values) {
    std::string_view val = s;
    in.storeString(val.begin(), val.end(), prev);
  }

  FieldStorageOut out = convertToOutStorage(std::move(in));
  std::vector<std::string> decoded_values{values.size()};
  prev = first;
  for (std::size_t i = 0; i < values.size(); ++i) {
    auto &dec_val = decoded_values[i];
    dec_val.resize(values[i].size());
    out.loadNextString(dec_val.data(), prev);
  }

  /* all values were read */
  CHECK(out.index.isDifferentPos == out.isDifferentFlag.size());
  CHECK(out.index.contentPos == out.content.size());
  CHECK(out.index.contentLengthPos == out.contentLength.size());
  /* ...and corectly decoded */
  CHECK(values == decoded_values);
}
