#pragma once
#include "compressed_buffers.h"
#include "defs.h"
#include "headers.h"
#include "prepare.h"
#include "report.h"
#include <vector>

namespace fqzcomp28 {
class EncodingContext {

  using header_fields_t = std::vector<headers::field_data_t>;

  /**
   * additional space to add when estimating compressed sizes
   * (required if input is very small)
   */
  static constexpr std::size_t extra_cbuffer_size = 1024;

  /** we assume that it's at least as good as bit-packing */
  static std::size_t compressBoundSequence(std::size_t original_size) {
    return original_size / 4 + extra_cbuffer_size;
  }

  static std::size_t compressBoundQuality(std::size_t original_size) {
    /* 94 different levels, so need 7 bits per symbol */
    return original_size / 8 * 7 + extra_cbuffer_size;
  }

  /** reserves enough space in `cbs` to encode `chunk`
   * & write stuff required for decompression */
  void prepareCompressedBuffers(const FastqChunk &chunk,
                                CompressedBuffersDst &cbs) {
    cbs.clear();
    cbs.original_size = chunk.raw_data.size();
    cbs.seq.resize(compressBoundSequence(chunk.tot_reads_length));
    cbs.qual.resize(compressBoundQuality(chunk.tot_reads_length));
    cbs.header_fields.resize(meta_->header_fmt.n_fields());
  }

  /** reserves enough space in `chunk` to decode `cbs` */
  static void prepareFastqChunk(FastqChunk &chunk,
                                const CompressedBuffersSrc &cbs) {
    chunk.clear();
    chunk.raw_data.resize(cbs.original_size);
    chunk.records.resize(cbs.n_records());
  }

public:
  EncodingContext(const DatasetMeta *meta);

  /** encodes reads into cbs, allocating memory in cbs as needed */
  void encodeChunk(const FastqChunk &, CompressedBuffersDst &cbs);

  /** decodes reads from cbs into chunk; resizes chunk as needed */
  void decodeChunk(FastqChunk &chunk, CompressedBuffersSrc &cbs);

  friend struct EncodingContextTester;

private:
  void encodeHeader(const std::string_view header, CompressedBuffersDst &cbs);

  /** @return number of bytes written to dst */
  unsigned decodeHeader(char *dst, CompressedBuffersSrc &cbs);

  void updateStats(const CompressedBuffersDst &cbs);

  /**
   * must be called to reset context before encoding or
   * decoding a new chunk
   */
  void startNewChunk();

  const DatasetMeta *const meta_;

  headers::field_data_t current_field_;

  /** context for header encoding */
  header_fields_t prev_header_fields_;
  header_fields_t first_header_fields_;

  /** accumulates statistics across encoded chunks */
  CompressedSizes comp_stats_;

  /**
   * put default-constructed value into each field according to header format
   */
  static void
  initalizeHeaderFields(header_fields_t &fields,
                        const headers::HeaderFormatSpeciciation &fmt);

  static void
  convertHeaderFieldFromAscii(std::string_view::iterator field_start,
                              std::string_view::iterator field_end,
                              headers::field_data_t &dst,
                              headers::FieldType typ);
};
} // namespace fqzcomp28
