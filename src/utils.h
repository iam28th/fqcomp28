#pragma once
#include "defs.h"
#include <cstring>
#include <vector>

namespace fqzcomp28 {

/**
 * @brief appends bytes from val to storage
 */
template <TriviallyCopyable T>
void storeAsBytes(const T val, std::vector<std::byte> &storage) {
  const std::byte *p = reinterpret_cast<const std::byte *>(&val);
  storage.insert(storage.end(), p, p + sizeof(val));
}

/**
 * @param i - Index in storage at which the object starts
 */
template <TriviallyCopyable T>
T loadFromBytes(const std::vector<std::byte> &storage, const std::size_t i) {
  T ret;
  std::memcpy(reinterpret_cast<std::byte *>(&ret), storage.data() + i,
              sizeof(T));
  return ret;
}

template <TriviallyCopyable T> auto to_byte_ptr(const T *p) {
  return reinterpret_cast<const std::byte *>(p);
}

template <TriviallyCopyable T> auto to_char_ptr(const T *p) {
  return reinterpret_cast<const char *>(p);
}
template <TriviallyCopyable T> auto to_char_ptr(T *p) {
  return reinterpret_cast<char *>(p);
}

#if 0
template <typename T>
concept signed_integral = std::integral<T> && std::is_signed_v<T>;

template <typename T>
concept unsigned_integral = std::integral<T> && std::is_unsigned_v<T>;

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
autoreadDeltaFromUnsigned(T_val prev, T_udelta udelta)
  requires std::is_same_v<T_udelta, std::make_unsigned_t<T_val>>
{
  T_val nxt = (udelta & 1u) ? -static_cast<T_val>(udelta >> 1u)
                            : static_cast<T_val>(udelta >> 1u);
  nxt += prev;
  return nxt;
}
#endif

}; // namespace fqzcomp28
