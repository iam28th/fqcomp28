#pragma once
#include "defs.h"

namespace fqzcomp28 {

void loadFileContents(path_t const path, std::vector<char> &data);
std::vector<char> loadFileContents(path_t const path);

template <typename T>
void storeAsBytes(T val, std::vector<unsigned char> &storage) {
  unsigned char *p = reinterpret_cast<unsigned char *>(&val);
  storage.insert(storage.end(), p, p + sizeof(val));
}

inline auto *to_byte_ptr(std::string_view::iterator it) {
  return reinterpret_cast<const std::byte *>(it);
}

}; // namespace fqzcomp28
