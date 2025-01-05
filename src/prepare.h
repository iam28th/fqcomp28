#pragma once
#include "defs.h"
#include "fse_quality.h"
#include "fse_sequence.h"
#include "headers.h"
#include <algorithm>

namespace fqzcomp28 {

/**
 * Contains information required for compression and decompression
 * (header format, frequency tables, etc.)
 */
struct DatasetMeta {

  DatasetMeta() = default;

  explicit DatasetMeta(const std::string_view header)
      : first_header(header),
        header_fmt(
            headers::HeaderFormatSpeciciation::fromHeader(first_header)) {}

  explicit DatasetMeta(const FastqChunk &chunk)
      : first_header(chunk.records.front().header()),
        header_fmt(headers::HeaderFormatSpeciciation::fromHeader(first_header)),
        ft_seq(FSE_Sequence::calculateFreqTable(chunk)),
        ft_qual(FSE_Quality::calculateFreqTable(chunk)) {}

  /** used for delta-ing the first header in each chunk */
  std::string first_header;
  headers::HeaderFormatSpeciciation header_fmt;
  std::unique_ptr<FSE_Sequence::FreqTableT> ft_seq;
  std::unique_ptr<FSE_Quality::FreqTableT> ft_qual;

  [[nodiscard]] auto n_fields_of_type(headers::FieldType typ) const {
    const auto &types = header_fmt.field_types;
    return std::count(types.begin(), types.end(), typ);
  }

  /** @return how many bytes are needed to store metadata in archive */
  [[nodiscard]] std::size_t size() const {
    return headers() + sequence() + quality();
  };

  static void storeToStream(const DatasetMeta &, std::ostream &);
  static DatasetMeta loadFromStream(std::istream &);

  [[nodiscard]] std::size_t headers() const {
    return sizeof(readlen_t) + first_header.length();
  }
  [[nodiscard]] std::size_t sequence() const { return sizeof(*ft_seq); }
  [[nodiscard]] std::size_t quality() const { return sizeof(*ft_qual); }
};

bool operator==(const DatasetMeta &lhs, const DatasetMeta &rhs);

DatasetMeta analyzeDataset(path_t fastq_file, std::size_t sample_size_bytes);

} // namespace fqzcomp28
