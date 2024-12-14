#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace fqzcomp28 {

namespace headers {

/**
 * how we parse header segments
 * TODO: other special fields, e.g. for index sequence
 */
enum class FieldType { NUMERIC, STRING };

using field_data_t = std::variant<uint64_t, std::string>;

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
   * else - store 1 in `repeatFlag`, value in `content`
   * and value's length in `contentLength`
   *
   * for NUMERIC - store delta with the previous
   * header as 2 bytes in `content`
   * */
  std::vector<unsigned char> repeatFlag;
  std::vector<unsigned char> content;
  std::vector<unsigned char> contentLength;

  void clear() {
    repeatFlag.clear();
    content.clear();
  }
};

} // namespace headers
} // namespace fqzcomp28
