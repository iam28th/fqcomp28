#pragma once
#include "defs.h"
#

namespace fqzcomp28 {

void loadFileContents(path_t const path, std::vector<char> &data);
std::vector<char> loadFileContents(path_t const path);

template <typename T>
void storeAsBytes(T val, std::vector<std::byte> &storage) {
  std::byte *p = reinterpret_cast<std::byte *>(&val);
  storage.insert(storage.end(), p, p + sizeof(val));
}

inline auto *to_byte_ptr(std::string_view::iterator it) {
  return reinterpret_cast<const std::byte *>(it);
}

template <typename T>
concept signed_integral = std::integral<T> && std::is_signed_v<T>;

template <typename T>
concept unsigned_integral = std::integral<T> && std::is_unsigned_v<T>;

#if 0
/* last bit is 0 if value if larger than the previous */
// TODO: not sure if this is a good encoding strategy,
// maybe better to just store delta in signed T as is ...
template <signed_integral T> auto storeDeltaInUnsigned(T prev, T nxt) {
  using ret_t = std::make_unsigned_t<T>;
  T delta = std::abs(nxt - prev);
  assert(static_cast<ret_t>(delta) < (std::numeric_limits<ret_t>::max() >> 1u));
  ret_t ret = (static_cast<ret_t>(delta) << 1u) + (nxt < prev);
  return ret;
};

template <signed_integral T_val, unsigned_integral T_udelta>
auto readDeltaFromUnsigned(T_val prev, T_udelta udelta)
  requires std::is_same_v<T_udelta, std::make_unsigned_t<T_val>>
{
  T_val nxt = (udelta & 1u) ? -static_cast<T_val>(udelta >> 1u)
                            : static_cast<T_val>(udelta >> 1u);
  nxt += prev;
  return nxt;
}
#endif

}; // namespace fqzcomp28
