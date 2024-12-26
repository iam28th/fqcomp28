#pragma once
#include "prepare.h"
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

  std::size_t total() const {
    std::size_t tot = header + 2 * seq;
    tot += n_records * 5; /* 4 newlines and a '+' symbol per record */
    return tot;
  }
};

struct CompressedStats {
  std::size_t readlens = 0;
  std::size_t qual = 0;
  std::size_t seq = 0, n_count = 0, n_pos = 0;

  std::size_t n_blocks = 0;

  std::vector<std::size_t> header_fields;

  std::size_t sequence() const { return seq + readlens + n_count + n_pos; }

  std::size_t quality() const { return qual; }

  std::size_t headers() const {
    return std::accumulate(header_fields.begin(), header_fields.end(), 0ull);
  }

  /**
   * @param n_string_fields How many header fields are of type "STRING"
   */
  std::size_t total(const long n_string_fields) const {
    std::size_t ret = sequence() + quality() + headers();

    std::size_t sizes = 0;         // original and compressed sizes
    sizes += sizeof(uint64_t);     // total size
    sizes += sizeof(uint64_t);     // n_records
    sizes += 2 * sizeof(uint64_t); // readlens (original & compressed)
    sizes += 2 * sizeof(uint64_t); // npos
    sizes += 2 * sizeof(uint64_t); // ncount
    sizes += 2 * sizeof(uint64_t); // compressed sizes for sequence and quality

    auto n_string = static_cast<uint64_t>(n_string_fields);
    uint64_t n_numeric = header_fields.size() - n_string;
    sizes += n_string * 3 * 2 * sizeof(uint64_t);
    sizes += n_numeric * 2 * sizeof(uint64_t);

    return ret + n_blocks * sizes;
  }
};

void printReport(const InputStats &, const CompressedStats &,
                 const DatasetMeta &meta, std::ostream &);
} // namespace fqzcomp28
