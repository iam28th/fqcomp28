#pragma once
#include "defs.h"
#include "headers.h"

namespace fqzcomp28 {

/**
 * Contains information required for compression and decompression
 * (header format, frequency tables, etc.)
 */
struct DatasetMeta {

  DatasetMeta() = default;

  DatasetMeta(const std::string_view header)
      : first_header(header),
        header_fmt(
            headers::HeaderFormatSpeciciation::fromHeader(first_header)) {}

  DatasetMeta(const FastqChunk &chunk)
      : first_header(chunk.records.front().header()),
        header_fmt(
            headers::HeaderFormatSpeciciation::fromHeader(first_header)) {}

  /** used for delta-ing the first header in each chunk */
  std::string first_header;
  headers::HeaderFormatSpeciciation header_fmt;

  static void storeToStream(const DatasetMeta &, std::ostream &);
  static DatasetMeta loadFromStream(std::istream &);

  std::size_t sizeInArchive() const {
    // TODO: account for frequency tables and stuff
    return sizeof(readlen_t) + first_header.size();
  }
};

bool operator==(const DatasetMeta &lhs, const DatasetMeta &rhs);

DatasetMeta analyzeDataset(path_t fastq_file, std::size_t sample_syte_bytes);

} // namespace fqzcomp28
