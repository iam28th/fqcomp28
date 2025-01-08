#include "headers.h"
#include "defs.h"
#include "utils.h"
#include <algorithm>
#include <charconv>
#include <cstring>
#include <stdexcept>

namespace fqcomp28::headers {

field_data_t fieldFromAscii(std::string_view::iterator s,
                            std::string_view::iterator e, FieldType typ) {
  field_data_t ret;
  if (typ == FieldType::STRING) {
    ret = string_t{};
    std::get<headers::string_t>(ret) = {s, e};
  } else {
    headers::numeric_t &val = std::get<headers::numeric_t>(ret);
    [[maybe_unused]] auto [_, ec] = std::from_chars(s, e, val);
    assert(ec == std::errc()); /* no error */
  }
  return ret;
}

header_fields_t fromHeader(const std::string_view header,
                           const HeaderFormatSpeciciation &fmt) {
  header_fields_t fields(fmt.n_fields());

  const auto *field_start = header.begin() + 1; /* skip '@' */
  for (std::size_t i = 0, E = fmt.n_fields() - 1; i < E; ++i) {
    const auto *field_end =
        std::find(field_start + 1, header.end(), fmt.separators[i]);

    fields[i] = fieldFromAscii(field_start, field_end, fmt.field_types[i]);
    field_start = field_end + 1; /* skip separator */
  }
  fields.back() =
      fieldFromAscii(field_start, header.end(), fmt.field_types.back());

  return fields;
}

HeaderFormatSpeciciation
HeaderFormatSpeciciation::fromHeader(const std::string_view header) {
  assert(header[0] == '@');

  HeaderFormatSpeciciation fmt;
  const auto *field_start = header.begin() + 1;

  for (;;) {
    // TODO: also start a new field when switching between numeric and string ?
    const auto *sep = std::find_if_not(field_start, header.end(),
                                       [](char c) { return std::isalnum(c); });
    const std::string_view field(field_start, sep);

    if (std::all_of(field.begin(), field.end(),
                    [](char c) { return std::isdigit(c); }))
      fmt.field_types.push_back(FieldType::NUMERIC);
    else
      fmt.field_types.push_back(FieldType::STRING);

    if (sep == header.end())
      break;
    if (sep == header.end() - 1) /* this should be generally the case */
      throw std::invalid_argument(std::string(header) +
                                  ": header should end in alnum char");
    fmt.separators.push_back(*sep);

    field_start = sep + 1;
  }

  return fmt;
}

void FieldStorageDst::storeString(string_t::iterator field_start,
                                  string_t::iterator field_end,
                                  string_t &prev_val) {
  const string_t val(field_start, field_end);
  if (val == prev_val) {
    isDifferentFlag.push_back(BYTE0);
  } else {
    isDifferentFlag.push_back(BYTE1);
    assert(val.length() < FIELDLEN_MAX);
    content.insert(content.end(), to_byte_ptr(val.begin()),
                   to_byte_ptr(val.end()));
    contentLength.push_back(static_cast<std::byte>(val.length()));
    prev_val = val;
  }
}

unsigned FieldStorageSrc::loadNextString(char *dst, string_t &prev_val) {
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

void FieldStorageDst::storeNumeric(string_t::iterator field_start,
                                   string_t::iterator field_end,
                                   numeric_t &prev_val) {
  numeric_t val;
  [[maybe_unused]] auto [_, ec] = std::from_chars(field_start, field_end, val);
  assert(ec == std::errc()); /* no error */

  const numeric_t delta = val - prev_val;
  storeAsBytes(delta, content);
  prev_val = val;
}

unsigned FieldStorageSrc::loadNextNumeric(char *dst, numeric_t &prev_val) {

  numeric_t delta;
  std::memcpy(reinterpret_cast<std::byte *>(&delta),
              content.data() + index.contentPos, sizeof(delta));
  index.contentPos += sizeof(delta);

  const numeric_t val = prev_val + delta;
  prev_val = val;

  const auto res = std::to_chars(dst, dst + FIELDLEN_MAX, val);
  assert(res.ec == std::errc());
  return static_cast<unsigned>(res.ptr - dst);
}
} // namespace fqcomp28::headers
