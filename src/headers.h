#pragma once
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <variant>
#include <vector>

namespace fqcomp28::headers {

constexpr std::size_t FIELDLEN_MAX = 255;

/**
 * How we parse & store header segments
 * TODO: other special fields, e.g. for index sequence, length, ...
 */
enum class FieldType { NUMERIC = 0, STRING };

using string_t = std::string_view;
using numeric_t = int32_t;

using field_data_t = std::variant<numeric_t, string_t>;
field_data_t fieldFromAscii(std::string_view::iterator s,
                            std::string_view::iterator e, FieldType);

/** Describes the structure of headers */
struct HeaderFormatSpeciciation {
  HeaderFormatSpeciciation() = default;

  std::vector<FieldType> field_types;
  std::vector<char> separators;
  [[nodiscard]] auto n_fields() const { return field_types.size(); }

  /**
   * Fills specification (i.e., number and types of fields) from
   * example header
   */
  static HeaderFormatSpeciciation fromHeader(std::string_view header);
};

using header_fields_t = std::vector<headers::field_data_t>;

/** @return Header fields constructed from `header` according to `fmt */
header_fields_t fromHeader(std::string_view header,
                           const HeaderFormatSpeciciation &fmt);

/** holds a certain field of multiple headers */
struct FieldStorage {
public:
  /**
   * for STRING - if the field' value is the same as
   * in the previous header, store 0 in `isDifferent`
   * else - store 1 in `isDifferent`, value in `content`
   * and value's length (as 1 byte) in `contentLength`
   *
   * for NUMERIC - store delta with the previous
   * header as sizeof(numeric_t) bytes in `content`
   * TODO: try encoding sign and magnitude of delta separately
   * */
  std::vector<std::byte> isDifferentFlag;
  std::vector<std::byte> content;
  std::vector<std::byte> contentLength;

  /** helper struct to load original sizes into during decompression */
  struct sizes {
    uint32_t isDifferentFlag, content, contentLength;

    bool operator==(const sizes &) const = default;
  };

  bool operator==(const FieldStorage &) const = default;

  virtual ~FieldStorage() = default;

  virtual void clear() {
    isDifferentFlag.clear();
    content.clear();
    contentLength.clear();
  }
};

struct FieldStorageDst : public FieldStorage {
  void storeString(string_t::iterator field_start, string_t::iterator field_end,
                   string_t &prev_val);
  void storeNumeric(string_t::iterator field_start,
                    string_t::iterator field_end, numeric_t &prev_val);
};

struct FieldStorageSrc : public FieldStorage {
  struct {
    // TODO: track as std::byte* instead
    std::size_t isDifferentPos = 0, contentPos = 0, contentLengthPos = 0;
  } index;
  /**
   * @return Number of bytes written to `dst`
   */
  unsigned loadNextString(char *dst, string_t &prev_val);
  unsigned loadNextNumeric(char *dst, numeric_t &prev_val);

  void clear() override {
    FieldStorage::clear();
    index = {};
  }
};

/** Holds a certain field of multiple headers after general compression */
using CompressedFieldStorage = FieldStorage;

} // namespace fqcomp28::headers
