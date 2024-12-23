#pragma once
#include <cstddef>

namespace fqzcomp28 {
/**
 * Compresses src with generic compression algorithm, and writes results to dst
 * (which must be allocated by the caller)
 *
 * @param N - Number of from src to compress (i.e., original size)
 *
 * @return number of bytes written to dst (i.e., compressed size)
 */
std::size_t memcompress(std::byte *dst, const std::byte *src,
                        const std::size_t n);

/**
 * Decompresses src with generic compression algorithm, and writes results to
 * dst (which must be allocated by the caller)
 *
 * @param N - Number of bytes to decompress from src (i.e., compressed size)
 *
 * @return number of bytes written (i.e., decompressed size)
 */
std::size_t memdecompress(std::byte *dst, const std::byte *src,
                          const std::size_t n);

} // namespace fqzcomp28
