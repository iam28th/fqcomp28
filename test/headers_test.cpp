#include "headers.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <random>

using namespace fqzcomp28;
using namespace fqzcomp28::headers;

/**
 * split a header into fields and deduce their types
 */
TEST_CASE("HeaderFormatSpec") {

  using enum FieldType;

  std::string header1 = "@SRR22543904.1 1 length=150";
  const auto fmt1 = HeaderFormatSpeciciation::fromHeader(header1);

  REQUIRE(fmt1.n_fields() == 5);
  REQUIRE(fmt1.field_types ==
          std::vector{STRING, NUMERIC, NUMERIC, STRING, NUMERIC});
  REQUIRE(fmt1.separators == std::vector{'.', ' ', ' ', '='});

  std::string header2 =
      "@SRR065390.1000 HWUSI-EAS687_61DAJ:8:1:1174:9158 length=100";
  const auto fmt2 = HeaderFormatSpeciciation::fromHeader(header2);
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

  FieldStorageDst in;
  std::string_view prev = first;
  for (const auto &s : values) {
    string_t val = s;
    in.storeString(val.begin(), val.end(), prev);
  }
  CHECK(in.isDifferentFlag.size() == values.size());

  FieldStorageSrc out = convertToSrcStorage(std::move(in));
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

  /* calling clear sets index to 0 */
  out.clear();
  CHECK(out.index.contentPos == 0);
}

/**
 * NUMERIC field storage
 */
TEST_CASE("{store,load}Numeric") {

  using random_engine_t = std::mt19937;
  using uniform_dist_t = std::uniform_int_distribution<numeric_t>;
  std::random_device rd;
  const unsigned seed = rd();
  random_engine_t engine(seed);

  constexpr numeric_t min_rn = 1, max_rn = 33808546, n_values = 1000;
  uniform_dist_t dist(min_rn, max_rn);

  std::vector<std::string> values(n_values);
  for (auto &val : values)
    val = std::to_string(dist(engine));
  const std::string first = values.front();

  FieldStorageDst in;
  numeric_t prev = std::stoi(first);
  for (const auto &field : values) {
    const std::string_view field_view = field;
    in.storeNumeric(field_view.begin(), field_view.end(), prev);
  }

  /* we know how many bytes should have been written to each buffer */
  CHECK(in.isDifferentFlag.size() == 0);
  CHECK(in.contentLength.size() == 0);
  CHECK(in.content.size() == n_values * sizeof(numeric_t));

  FieldStorageSrc out = convertToSrcStorage(std::move(in));
  std::vector<std::string> decoded_values{values.size()};
  prev = std::stoi(first);
  for (std::size_t i = 0; i < values.size(); ++i) {
    auto &dec_val = decoded_values[i];
    dec_val.resize(values[i].size());
    out.loadNextNumeric(dec_val.data(), prev);
  }

  /* all bytes were read and correctly decoded  */
  CHECK(out.index.contentPos == out.content.size());
  CHECK(values == decoded_values);
}
