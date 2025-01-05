#pragma once
#include "compressed_buffers.h"
#include "defs.h"
#include "headers.h"
#include "prepare.h"
#include "report.h"
#include <vector>

namespace fqzcomp28 {
class Workspace {
protected:
  /**
   * additional space to add when estimating compressed sizes
   * (required if input is very small for FSE, and _always_ for
   * general-compressed buffers)
   */
  static constexpr std::size_t extra_csize_fse = 1024;
  static constexpr std::size_t extra_csize_misc = 28; // LIBBSC_HEADER_SIZE

public:
  static std::size_t compressBoundSequence(std::size_t original_size) {
    /* we can normally assume that the compression at least as good as
     * bit-packing
     * ...but also for very small inputs (e.g., 1 base) compressed size
     * is much larger than original size so I'm using a heuristic... */
    if (original_size < extra_csize_fse)
      return extra_csize_fse * FSE_Sequence::N_MODELS;
    return original_size / 4 + extra_csize_fse;
  }

  static std::size_t compressBoundQuality(std::size_t original_size) {
    /* 94 different levels, so need 7 bits per symbol */
    return std::max(extra_csize_fse * FSE_Quality::N_MODELS,
                    original_size * 7 / 8 + extra_csize_fse);
  }

public:
  explicit Workspace(const DatasetMeta *meta);

  friend struct WorkspaceTester;

protected:
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
};

class CompressionWorkspace : public Workspace {
public:
  friend struct WorkspaceTester;

  explicit CompressionWorkspace(const DatasetMeta *meta)
      : Workspace(meta), seq_encoder(meta->ft_seq.get()),
        qual_encoder(meta->ft_qual.get()) {
    comp_stats_.header_fields.resize(fmt_.n_fields());
  }

  /** Encodes reads into cbs, allocating memory in cbs as needed */
  void encodeChunk(FastqChunk &, CompressedBuffersDst &cbs);

  [[nodiscard]] const CompressedStats &stats() const { return comp_stats_; }

private:
  /** Reserves enough space in `cbs` to encode `chunk` */
  void prepareBuffersForEncoding(const FastqChunk &chunk,
                                 CompressedBuffersDst &cbs);

  void encodeHeader(const std::string_view header, CompressedBuffersDst &cbs);

  /**
   * Runs additional generic compression on header buffers, readlens, etc.
   * updates `comp_stats_`.
   */
  void compressMiscBuffers(CompressedBuffersDst &);

  void updateStats(const CompressedBuffersDst &cbs);

private:
  /** Accumulates statistics across encoded chunks */
  CompressedStats comp_stats_;
  SequenceEncoder seq_encoder;
  QualityEncoder qual_encoder;

private:
  /**
   * Compresses src to dst, resizing dst as needed
   * @return Resulting dst.size
   * */
  static std::size_t compressBuffer(std::vector<std::byte> &dst,
                                    const std::vector<std::byte> &src);
};

class DecompressionWorkspace : public Workspace {
public:
  friend struct WorkspaceTester;

  explicit DecompressionWorkspace(const DatasetMeta *meta)
      : Workspace(meta), seq_decoder(meta->ft_seq.get()),
        qual_decoder(meta->ft_qual.get()) {}

  /** Decodes reads from cbs into chunk; resizes chunk as needed */
  void decodeChunk(FastqChunk &chunk, CompressedBuffersSrc &cbs);

private:
  /** @return Number of bytes written to dst */
  unsigned decodeHeader(char *dst, CompressedBuffersSrc &cbs);

  /** Decompresses buffers that were compressed with generic algorithm */
  void decompressMiscBuffers(CompressedBuffersSrc &);

private:
  SequenceDecoder seq_decoder;
  QualityDecoder qual_decoder;

private:
  /** reserves enough space in `chunk` to decode `cbs` */
  static void prepareFastqChunk(FastqChunk &chunk,
                                const CompressedBuffersSrc &cbs) {
    chunk.clear();
    chunk.raw_data.resize(cbs.original_size.total);
    chunk.records.resize(cbs.original_size.n_records);
  }
};

} // namespace fqzcomp28
