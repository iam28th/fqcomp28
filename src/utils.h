#pragma once
#include <cstring>
#include <string_view>
#include <vector>

namespace fqzcomp28 {

/**
 * @brief appends bytes from val to storage
 */
template <typename T>
void storeAsBytes(const T val, std::vector<std::byte> &storage)
  requires std::is_trivially_copyable_v<T>
{
  const std::byte *p = reinterpret_cast<const std::byte *>(&val);
  storage.insert(storage.end(), p, p + sizeof(val));
}

/**
 * @param i - Index in storage at which the object starts
 */
template <typename T>
  requires std::is_trivially_copyable_v<T>
T loadFromBytes(const std::vector<std::byte> &storage, const std::size_t i) {
  T ret;
  std::memcpy(reinterpret_cast<std::byte *>(&ret), storage.data() + i,
              sizeof(T));
  return ret;
}

template <std::random_access_iterator T> auto to_byte_ptr(T it) {
  return reinterpret_cast<std::byte *>(it);
}

template <std::random_access_iterator T>
auto to_byte_ptr(T it)
  requires std::is_same_v<T, std::string_view::iterator> ||
           std::is_same_v<T, const char *>
{
  return reinterpret_cast<const std::byte *>(it);
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
