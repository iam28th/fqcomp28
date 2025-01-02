#include <cassert>
#include <cstddef>
#include <cstring>

#include "libbsc/filters.h"
#include "libbsc/libbsc.h"

/// copy-paste from libbsc/bsc.cpp
int paramBlockSize = 64 * 1024 * 1024;
int paramBlockSorter = LIBBSC_BLOCKSORTER_BWT;
int paramCoder = LIBBSC_CODER_QLFC_STATIC;
int paramSortingContexts = LIBBSC_CONTEXTS_FOLLOWING;

int paramEnableParallelProcessing = 1;
int paramEnableMultiThreading = 1;
int paramEnableFastMode = 1;
int paramEnableLargePages = 0;
int paramEnableCUDA = 0;
int paramEnableSegmentation = 0;
int paramEnableReordering = 0;
int paramEnableLZP = 1;
int paramLZPHashSize = 15;
int paramLZPMinLen = 128;

int paramFeatures() {
  int features =
      (paramEnableFastMode ? LIBBSC_FEATURE_FASTMODE : LIBBSC_FEATURE_NONE) |
      (paramEnableMultiThreading ? LIBBSC_FEATURE_MULTITHREADING
                                 : LIBBSC_FEATURE_NONE) |
      (paramEnableLargePages ? LIBBSC_FEATURE_LARGEPAGES
                             : LIBBSC_FEATURE_NONE) |
      (paramEnableCUDA ? LIBBSC_FEATURE_CUDA : LIBBSC_FEATURE_NONE);

  return features;
}
/// end of copy-paste...

namespace fqzcomp28 {

std::size_t memcompress(std::byte *dst, const std::byte *src,
                        const std::size_t src_size) {
  int ret = bsc_init(paramFeatures());
  assert(ret == LIBBSC_NO_ERROR);
  int compressedSize =
      bsc_compress(reinterpret_cast<const unsigned char *>(src),
                   reinterpret_cast<unsigned char *>(dst),
                   static_cast<int>(src_size), paramLZPHashSize, paramLZPMinLen,
                   paramBlockSorter, paramCoder, paramFeatures());
  return static_cast<std::size_t>(compressedSize);
}

std::size_t memdecompress(std::byte *dst, const std::size_t dst_size,
                          const std::byte *src, const std::size_t src_size) {
  if (src_size == 0) [[unlikely]]
    return 0;

  int ret = bsc_init(paramFeatures());
  assert(ret == LIBBSC_NO_ERROR);
  ret = bsc_decompress(reinterpret_cast<const unsigned char *>(src),
                       static_cast<int>(src_size),
                       reinterpret_cast<unsigned char *>(dst),
                       static_cast<int>(dst_size), paramFeatures());
  assert(ret == LIBBSC_NO_ERROR);
  return dst_size;
}
} // namespace fqzcomp28
