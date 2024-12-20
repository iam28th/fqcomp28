#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
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
using numeric_t = long long;
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
#if 0
  void storeNumeric(string_t::iterator field_start,
                    string_t::iterator field_end, numeric_t &prev_val);
#endif
};

struct FieldStorageOut : public FieldStorage {
  struct {
    std::size_t isDifferentPos = 0, contentPos = 0, contentLengthPos = 0;
  } index;

  void clear() override {
    FieldStorage::clear();
    index = {};
  }

  unsigned loadNextString(char *dst, string_t &prev_fal);
#if 0
  unsigned loadNumeric(char *dst, numeric_t &prev_val);
#endif
};

#if 0
/**
 * used for reading - holds current position in FieldStorage buffers
 */
struct FieldStorageIndex {};

/**
 * @return number of bytes written to `to`
 */
std::size_t loadString(char *to, string_t &prev_val, FieldStorage &,
                       FieldStorageIndex &);

/**
 * @return number of bytes written to `to`
 */
std::size_t loadString(char *to, string_t &prev_val, FieldStorage &,
                       FieldStorageIndex &);
#endif
} // namespace headers
} // namespace fqzcomp28
