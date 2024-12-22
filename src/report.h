#pragma once
#include <cstddef>
#include <vector>

namespace fqzcomp28 {
#if 0
struct CompressionStats {
  struct {
    std::size_t seq, header;
  } input_sizes;

  struct {
    std::size_t seq, qual, header;
  } compressed_sizes;

  std::vector<std::size_t> header_field_sizes;
};
#endif

struct InputStats {
  std::size_t seq, header;
  std::size_t n_records;
};

struct CompressedSizes {
  std::size_t seq, qual;
  std::vector<std::size_t> header_fields;
};
} // namespace fqzcomp28
