#pragma once
#include "headers.h"
#include <vector>

namespace fqzcomp28 {
class EncodingContext {
public:
  // TODO: accept sequence & quality frequency tables ?
  EncodingContext(headers::HeaderFormatSpeciciation fmt) : fmt_(fmt) {}

  /**
   * populates prev_header_fields_ according to
   */
  void startNewChunk();

private:
  const headers::HeaderFormatSpeciciation fmt_;
  std::vector<headers::field_data_t> prev_header_fields_;
};
} // namespace fqzcomp28
