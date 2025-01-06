#pragma once
#include "archive.h"
#include <cstddef>
#include <iosfwd>
#include <numeric>
#include <vector>

namespace fqzcomp28 {

struct InputStats {
  std::size_t seq = 0;
  std::size_t header = 0;
  std::size_t n_records = 0;

  [[nodiscard]] std::size_t total() const {
    std::size_t tot = header + 2 * seq;
    tot += n_records * 5; /* 4 newlines and a '+' symbol per record */
    return tot;
  }

  InputStats &operator+=(const InputStats &other) {
    seq += other.seq;
    header += other.header;
    n_records += other.n_records;
    return *this;
  }
};

struct CompressedStats {
  std::size_t readlens = 0;
  std::size_t qual = 0;
  std::size_t seq = 0, n_count = 0, n_pos = 0;

  std::size_t n_blocks = 0;

  std::vector<std::size_t> header_fields;

  CompressedStats &operator+=(const CompressedStats &other) {
    if (other.n_blocks == 0)
      return *this;

    readlens += other.readlens;
    qual += other.qual;
    seq += other.seq;
    n_count += other.n_count;
    n_pos += other.n_pos;
    n_blocks += other.n_blocks;

    assert(header_fields.size() == other.header_fields.size());
    for (unsigned i = 0; i < header_fields.size(); ++i)
      header_fields[i] += other.header_fields[i];

    return *this;
  }

  [[nodiscard]] std::size_t sequence() const {
    return seq + readlens + n_count + n_pos;
  }

  [[nodiscard]] std::size_t quality() const { return qual; }

  [[nodiscard]] std::size_t headers() const {
    return std::accumulate(header_fields.begin(), header_fields.end(), 0ull);
  }

  /** @param n_string_fields How many header fields are of type "STRING" */
  [[nodiscard]] std::size_t data_section_size(const long n_string_fields) const;
};

void printReport(const InputStats &, const CompressedStats &, const Archive &,
                 std::ostream &);
} // namespace fqzcomp28
