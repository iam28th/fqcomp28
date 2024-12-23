#include <cstddef>
#include <cstring>

namespace fqzcomp28 {
std::size_t memcompress(std::byte *dst, const std::byte *src,
                        const std::size_t n) {
  // TODO use something like bsc
  std::memcpy(dst, src, n);
  return n;
}

std::size_t memdecompress(std::byte *dst, const std::byte *src,
                          const std::size_t n) {
  std::memcpy(dst, src, n);
  return n;
}
} // namespace fqzcomp28
