#pragma once
#include <cstddef>
#include <iosfwd>
#include <numeric>
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
  std::size_t seq = 0;
  std::size_t header = 0;
  std::size_t n_records = 0;
};

struct CompressedSizes {
  std::size_t readlens = 0;
  std::size_t qual = 0;
  std::size_t seq = 0, n_count = 0, n_pos = 0;

  std::vector<std::size_t> header_fields;

#if 0
  std::size_t sequence() const {
    return seq + readlens + n_count + n_pos 
  }
#endif

  std::size_t total() const {
    const std::size_t headers =
        std::accumulate(header_fields.begin(), header_fields.end(), 0ull);
    return (seq + n_count + n_pos + readlens) + qual + headers;
  }
};

void printReport(const InputStats &, const CompressedSizes &, std::ostream &,
                 const int precision = 2);
} // namespace fqzcomp28
