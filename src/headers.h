#pragma once
#include <cassert>
#include <string_view>
#include <vector>

namespace fqzcomp28 {

namespace headers {

/**
 * how we parse header segments
 * TODO: other special fields, e.g. for index sequence
 */
enum class FieldType { STRING, NUMERIC };

/**
 * describes the structure of a header
 */
struct HeaderFormatSpeciciation {
  unsigned n_fields;
  std::vector<FieldType> field_types;
  std::vector<char> separators;

  HeaderFormatSpeciciation(const std::string_view header) {
    assert(header[0] == '@');
    /* for now - treat entire header as a single STRING field */
    n_fields = 1;
    field_types.push_back(FieldType::STRING);

    assert(separators.size() == n_fields - 1);
  }
};

/**
 * holds a certain field of multiple headers
 */
struct FieldStorage {
  static constexpr unsigned char SEPARATOR = 0;
  /**
   * for STRING - if the field' value is the same as
   * in the previous header, store 0 in `isDifferent`
   * else - store 1 in `repeatFlag` and value + separator in `content`
   *
   * for NUMERIC - store delta with the previous header as 2 bytes
   * */
  std::vector<unsigned char> repeatFlag;
  std::vector<unsigned char> content;

  void clear() {
    repeatFlag.clear();
    content.clear();
  }
};

} // namespace headers
} // namespace fqzcomp28
