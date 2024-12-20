#pragma once
#include "defs.h"
#include "headers.h"
#include <vector>

namespace fqzcomp28 {

struct CompressedBuffers {

#if 0
  std::vector<unsigned char> seq, qual;
#endif
  std::vector<std::byte> seq, qual;
  std::vector<headers::FieldStorage> header_fields;

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
    for (auto &hf : header_fields)
      hf.clear();
  }

  // TODO: methods putHeader, putLength, putSequence, ...
  // nextHeader, nextLength ...
};

} // namespace fqzcomp28
