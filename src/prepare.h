#pragma once
#include "defs.h"
#include "headers.h"

namespace fqzcomp28 {

/**
 * Contains information required for compression and decompression
 * (header format, frequency tables, etc.)
 */
struct DatasetMeta {
  /**
   * used for delta-ing first header in each chunk
   */
  const std::string first_header;

  const headers::HeaderFormatSpeciciation header_fmt;

  DatasetMeta(const FastqChunk &);
};

DatasetMeta analyzeDataset(path_t fastq_file, std::size_t sample_syte_bytes);

} // namespace fqzcomp28
