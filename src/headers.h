#pragma once
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string_view>
#include <variant>
#include <vector>

namespace fqzcomp28 {

namespace headers {

/**
 * how we parse & store header segments
 * TODO: other special fields, e.g. for index sequence
 */
enum class FieldType { NUMERIC = 0, STRING };

/* string_t is a view into either the first or the previous header */
using numeric_t = int32_t;
using string_t = std::string_view;
using field_data_t = std::variant<numeric_t, string_t>;

constexpr std::size_t FIELDLEN_MAX = 255;

/**
 * a type used to store delta between numeric field values
 * of adjacent headers
 */
using udelta_t = uint32_t;

/**
 * describes the structure of a header
 */
struct HeaderFormatSpeciciation {
  /**
   * fills specification (i.e., number and types of fields) from
   * example header
   */
  HeaderFormatSpeciciation(const std::string_view header);

  std::vector<FieldType> field_types;
  std::vector<char> separators;
  auto n_fields() const { return field_types.size(); }
};

/**
 * holds a certain field of multiple headers
 */
struct FieldStorage {
  /**
   * for STRING - if the field' value is the same as
   * in the previous header, store 0 in `isDifferent`
   * else - store 1 in `isDifferent`, value in `content`
   * and value's length (as 1 byte) in `contentLength`
   *
   * for NUMERIC - store delta with the previous
   * header as 2 bytes in `content`
   * */
  std::vector<std::byte> isDifferentFlag;
  std::vector<std::byte> content;
  std::vector<std::byte> contentLength;

  virtual void clear() {
    isDifferentFlag.clear();
    content.clear();
    contentLength.clear();
  }
};

struct FieldStorageIn : public FieldStorage {
  void storeString(string_t::iterator field_start, string_t::iterator field_end,
                   string_t &prev_val);
  void storeNumeric(string_t::iterator field_start,
                    string_t::iterator field_end, numeric_t &prev_val);
};

struct FieldStorageOut : public FieldStorage {
  struct {
    // TODO: track as std::byte* instead
    std::size_t isDifferentPos = 0, contentPos = 0, contentLengthPos = 0;
  } index;
  /**
   * @return number of bytes written to `dst`
   */
  unsigned loadNextString(char *dst, string_t &prev_fal);
  unsigned loadNextNumeric(char *dst, numeric_t &prev_val);

  void clear() override {
    FieldStorage::clear();
    index = {};
  }
};

inline udelta_t storeDeltaInUnsigned(numeric_t prev, numeric_t nxt) {
  numeric_t delta = std::abs(nxt - prev);
  assert(static_cast<udelta_t>(delta) <=
         (std::numeric_limits<udelta_t>::max() >> 1u));
  return (static_cast<udelta_t>(delta) << 1u) + (nxt < prev);
}

inline numeric_t readDeltaFromUnsigned(numeric_t prev, udelta_t udelta) {
  numeric_t delta = (udelta & 1u) ? -static_cast<numeric_t>(udelta >> 1u)
                                  : static_cast<numeric_t>(udelta >> 1u);
  return prev + delta;
}

} // namespace headers
} // namespace fqzcomp28
