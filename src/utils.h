#pragma once
#include "defs.h"
#include <cassert>
#include <cstring>
#include <iosfwd>
#include <vector>

namespace fqzcomp28 {

/**
 * If the stream's state is good, do nothing; otherwise throw system_error
 * @param path - Path to the file associated with the stream
 */
void checkStreamState(std::ios &, path_t path);

/** (c) The C++ Programming Language, section 11.5 */
template <class Target, class Source> Target narrow_cast(Source v) {
  auto r = static_cast<Target>(v);
  // convert the value to the target type
  if (static_cast<Source>(r) != v) [[unlikely]]
    throw std::runtime_error("narrow_cast<>() failed");
  return r;
}

/**
 * TODO test
 * @brief appends bytes from val to storage
 */
template <TriviallyCopyable T>
void storeAsBytes(const T val, std::vector<std::byte> &storage) {
  const std::byte *p = reinterpret_cast<const std::byte *>(&val);
  const std::size_t old_size = storage.size();
  storage.resize(storage.size() + sizeof(T));
  std::memcpy(storage.data() + old_size, p, sizeof(val));
}

/**
 * @param i - Index in storage at which the object starts
 */
template <TriviallyCopyable T>
T loadFromBytes(const std::vector<std::byte> &storage, const std::size_t i) {
  T ret = 0;
  assert(i + sizeof(T) <= storage.size());
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

}; // namespace fqzcomp28
