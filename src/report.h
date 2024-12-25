#pragma once
#include <cstddef>
#include <iosfwd>
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
  std::size_t seq;
  std::size_t header;
  std::size_t n_records;
};

struct CompressedSizes {
  std::size_t readlens;
  std::size_t qual;
  std::size_t seq, n_count, n_pos;

  std::vector<std::size_t> header_fields;
};

void printReport(const InputStats &, const CompressedSizes &, std::ostream &);
} // namespace fqzcomp28
