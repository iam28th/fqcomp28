#include "headers.h"
#include "utils.h"
#include <algorithm>
#include <charconv>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace fqzcomp28 {
namespace headers {

HeaderFormatSpeciciation::HeaderFormatSpeciciation(
    const std::string_view header) {
  assert(header[0] == '@');

  auto field_start = header.begin() + 1;

  for (;;) {
    // TODO: also start a new field when switching between numeric and string ?
    const auto sep = std::find_if_not(field_start, header.end(),
                                      [](char c) { return std::isalnum(c); });
    const std::string_view field(field_start, sep);

    if (std::all_of(field.begin(), field.end(),
                    [](char c) { return std::isdigit(c); }))
      field_types.push_back(FieldType::NUMERIC);
    else
      field_types.push_back(FieldType::STRING);

    if (sep == header.end())
      break;
    if (sep == header.end() - 1) /* this should be generally the case */
      throw std::invalid_argument(std::string(header) +
                                  ": header should end in alnum char");
    separators.push_back(*sep);

    field_start = sep + 1;
  }
}

void FieldStorageIn::storeString(string_t::iterator field_start,
                                 string_t::iterator field_end,
                                 string_t &prev_val) {
  const string_t val(field_start, field_end);
  if (val == prev_val) {
    isDifferentFlag.push_back(BYTE0);
  } else {
    isDifferentFlag.push_back(BYTE1);
    content.insert(content.end(), to_byte_ptr(val.begin()),
                   to_byte_ptr(val.end()));
    assert(val.length() < std::numeric_limits<unsigned char>::max());
    contentLength.push_back(static_cast<std::byte>(val.length()));
    prev_val = val;
  }
}

unsigned FieldStorageOut::loadNextString(char *dst, string_t &prev_val) {
  const auto is_different = isDifferentFlag[index.isDifferentPos++];
  if (is_different == BYTE0) {
    std::memcpy(dst, prev_val.begin(), prev_val.length());
    return static_cast<unsigned>(prev_val.length());
  }

  const auto field_length =
      static_cast<unsigned char>(contentLength[index.contentLengthPos++]);

  std::memcpy(dst, content.data() + index.contentPos, field_length);
  index.contentPos += field_length;
  prev_val = string_t{dst, field_length};

  return field_length;
}

#if 0
void storeNumeric(string_t::iterator field_start, string_t::iterator field_end,
                  numeric_t &prev_val, FieldStorage &storage) {
  numeric_t val;
  [[maybe_unused]] auto [_, ec] = std::from_chars(field_start, field_end, val);
  assert(ec == std::errc()); /* no error */

  /* last bit is 0 if value if larger than the previous */
  // TODO: convertion to/from delta functions
  numeric_t delta = std::abs(val - prev_val);
  assert(delta < (std::numeric_limits<udelta_t>::max() >> 1u));
  udelta_t udelta = (static_cast<udelta_t>(delta) << 1u) + (val < prev_val);

  storeAsBytes(udelta, storage.content);

  prev_val = val;
}

std::size_t loadNumeric(char *to, numeric_t &prev_val, FieldStorage &storage,
                        FieldStorageIndex &index) {
  assert(index.contentPos < storage.content.size());

  udelta_t udelta;
  std::memcpy(reinterpret_cast<unsigned char *>(&udelta),
              storage.content.data() + index.contentPos, sizeof(udelta));
  index.contentPos += sizeof(udelta);

  numeric_t delta = (udelta & 1u) ? -static_cast<numeric_t>(udelta >> 1u)
                                  : static_cast<numeric_t>(udelta >> 1u);
  numeric_t val = prev_val + delta;

  prev_val = val;

  auto res = std::to_chars(to, to + FIELDLEN_MAX, val);
  assert(res.ec == std::errc());
  return static_cast<std::size_t>(res.ptr - to);
}
#endif
} // namespace headers
} // namespace fqzcomp28
