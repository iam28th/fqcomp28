#pragma once
#include "defs.h"
#include "headers.h"
#include <vector>

namespace fqzcomp28 {

// TODO: separate classes for compression and decompression
struct CompressedBuffers {
  std::vector<std::byte> seq, qual;
  std::vector<headers::FieldStorageIn> header_fields_in;
  std::vector<headers::FieldStorageOut> header_fields_out;

  // TODO: now it's stored as plain values;
  // change to delta
  std::vector<readlen_t> readlens;

#if 0
  std::vector<readlen_t> n_counts;
  std::vector<readlen_t> n_positions;
#endif

  auto n_records() const { return readlens.size(); }

  void clear() {
    seq.clear();
    qual.clear();
    for (auto &hf : header_fields_in)
      hf.clear();
  }

  // TODO: methods putHeader, putLength, putSequence, ...
  // nextHeader, nextLength ...
};

} // namespace fqzcomp28
