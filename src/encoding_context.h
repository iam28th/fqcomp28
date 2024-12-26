#pragma once
#include "compressed_buffers.h"
#include "defs.h"
#include "headers.h"
#include "prepare.h"
#include "report.h"
#include <vector>

namespace fqzcomp28 {
class EncodingContext {
  /**
   * additional space to add when estimating compressed sizes
   * (required if input is very small)
   */
  static constexpr std::size_t extra_cbuffer_size = 1024;

  /** we assume that it's at least as good as bit-packing */
  static std::size_t compressBoundSequence(std::size_t original_size) {
    return original_size;
    // TODO: once we have fse in place...
    return original_size / 4 + extra_cbuffer_size;
  }

  static std::size_t compressBoundQuality(std::size_t original_size) {
    return original_size;
    // TODO: once we have fse in place...
    /* 94 different levels, so need 7 bits per symbol */
    return original_size / 8 * 7 + extra_cbuffer_size;
  }

  /** reserves enough space in `chunk` to decode `cbs` */
  static void prepareFastqChunk(FastqChunk &chunk,
                                const CompressedBuffersSrc &cbs) {
    chunk.clear();
    chunk.raw_data.resize(cbs.original_size.total);
    chunk.records.resize(cbs.original_size.n_records);
  }

public:
  EncodingContext(const DatasetMeta *meta);

  /** encodes reads into cbs, allocating memory in cbs as needed */
  void encodeChunk(const FastqChunk &, CompressedBuffersDst &cbs);
  /** decodes reads from cbs into chunk; resizes chunk as needed */
  void decodeChunk(FastqChunk &chunk, CompressedBuffersSrc &cbs);

  const CompressedStats &stats() const { return comp_stats_; }

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

  const headers::HeaderFormatSpeciciation fmt_;

  /** context for header encoding */
  const headers::header_fields_t first_header_fields_;
  headers::header_fields_t prev_header_fields_;
  headers::field_data_t current_field_;

  /** accumulates statistics across encoded chunks */
  CompressedStats comp_stats_;

  /**
   * reserves enough space in `cbs` to encode `chunk`
   */
  void prepareBuffersForEncoding(const FastqChunk &chunk,
                                 CompressedBuffersDst &cbs);

  /**
   * Runs additional generic compression on header buffers, readlens, etc.
   * updates `comp_stats_`.
   */
  void compressMiscBuffers(CompressedBuffersDst &);

  /** Decompresses buffers that were compressed with generic algorithm */
  void decompressMiscBuffers(CompressedBuffersSrc &);

  /**
   * Compresses src to dst, resizing dst as needed
   * @return Resulting dst.size
   * */
  static std::size_t compressBuffer(std::vector<std::byte> &dst,
                                    const std::vector<std::byte> &src);
};
} // namespace fqzcomp28
