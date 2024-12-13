#pragma once
#include "headers.h"
#include <vector>

namespace fqzcomp28 {

struct CompressedBuffers {

  std::vector<unsigned char> seq, qual;
  std::vector<headers::FieldStorage> header_fields;

  void clear() {
    seq.clear();
    qual.clear();
    for (auto &hf : header_fields)
      hf.clear();
  }
};

} // namespace fqzcomp28
