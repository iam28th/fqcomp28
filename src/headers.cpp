#include "headers.h"
#include <algorithm>
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
} // namespace headers
} // namespace fqzcomp28
