#pragma once
#include "compressed_buffers.h"
#include "defs.h"
#include "headers.h"
#include "prepare.h"
#include <vector>

namespace fqzcomp28 {
class EncodingContext {

  using header_fields_t = std::vector<headers::field_data_t>;

  /**
   * additional space to add when estimating compressed sizes
   * (required if input is very small)
   */
  static constexpr std::size_t extra_cbuffer_size = 1024;

  /* we assume that it's at least as good as bit-packing */
  static std::size_t compressBoundSequence(std::size_t original_size) {
    return original_size / 4 + extra_cbuffer_size;
  }

  static std::size_t compressBoundQuality(std::size_t original_size) {
    /* 94 different levels, so need 7 bits per symbol */
    return original_size / 8 * 7 + extra_cbuffer_size;
  }

public:
  EncodingContext(const DatasetMeta *meta);

  /**
   * encodes reads into cbs, allocating memory in cbs as needed
   */
  void encodeChunk(const FastqChunk &, CompressedBuffers &cbs);

  /**
   * decodes reads from cbs into chunk; space in chunk must be
   * we somehow must know number of reads ?
   * preallocated
   * // TODO: store stuff required for allocation in CompressedBuffers
   * // to make API symmetric
   */
  void decodeChunk(FastqChunk &chunk, CompressedBuffers &cbs);

  friend struct EncodingContextTester;

private:
  void encodeHeader(const std::string_view header, CompressedBuffers &cbs);

  /**
   * @return number of bytes written
   */
  unsigned decodeHeader(char *dst, CompressedBuffers &cbs);

  /**
   * must be called to reset context before encoding or
   * decoding a new chunk
   */
  void startNewChunk();

  const DatasetMeta *const meta_;

  headers::field_data_t current_field_;
  /**
   * context for header encoding
   */
  header_fields_t first_header_fields_;
  header_fields_t prev_header_fields_;

  static void
  initalizeHeaderFields(header_fields_t &fields,
                        const headers::HeaderFormatSpeciciation &fmt);

  /**
   * @brief
   */
  static void
  convertHeaderFieldFromAscii(std::string_view::iterator field_start,
                              std::string_view::iterator field_end,
                              headers::field_data_t &dst,
                              headers::FieldType typ);
};
} // namespace fqzcomp28
