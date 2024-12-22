#pragma once
#include "defs.h"
#include "headers.h"
#include <vector>

namespace fqzcomp28 {

template <typename T>
  requires std::is_base_of_v<headers::FieldStorage, T>
struct CompressedBuffers {
  CompressedBuffers() = default;
  std::vector<std::byte> seq, qual;
  std::vector<readlen_t> readlens;

  std::vector<T> header_fields;

  /** original size of reads encoded in this chunk */
  uint64_t original_size;

  auto n_records() const { return readlens.size(); }

  void clear() {
    seq.clear();
    qual.clear();
    for (auto &hf : header_fields)
      hf.clear();
  }
};

using CompressedBuffersSrc = CompressedBuffers<headers::FieldStorageSrc>;
using CompressedBuffersDst = CompressedBuffers<headers::FieldStorageDst>;

} // namespace fqzcomp28
