#pragma once
#include <cstddef>

namespace fqzcomp28 {

/**
 * Compresses src with generic compression algorithm, and writes results to dst
 * (which must be allocated by the caller)
 *
 * @param src_size - Number of bytes from src to compress (i.e., original size)
 *
 * @return number of bytes written to dst (i.e., compressed size)
 */
std::size_t memcompress(std::byte *dst, const std::byte *src,
                        const std::size_t src_size);

/**
 * Decompresses src with generic compression algorithm, and writes results to
 * dst (which must be allocated by the caller)
 *
 * @param dst_size - Number of bytes allocated at src (i.e., original size)
 * @param src_size - Number of bytes to decompress from src (i.e., compressed
 * size)
 *
 * @return number of bytes written (i.e., decompressed size)
 */
std::size_t memdecompress(std::byte *dst, const std::size_t dst_size,
                          const std::byte *src, const std::size_t src_size);

} // namespace fqzcomp28
