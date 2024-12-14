#pragma once
#include "headers.h"
#include <vector>

namespace fqzcomp28 {

struct CompressedBuffers {

  CompressedBuffers(headers::HeaderFormatSpeciciation fmt) : fmt_(fmt) {}

  std::vector<unsigned char> seq, qual;
  std::vector<headers::FieldStorage> header_fields;

  void clear() {
    seq.clear();
    qual.clear();
    for (auto &hf : header_fields)
      hf.clear();
  }

private:
  headers::HeaderFormatSpeciciation fmt_;
};

} // namespace fqzcomp28
