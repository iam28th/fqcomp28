#include "workspace.h"
#include "headers.h"
#include "memcompress.h"
#include "utils.h"
#include <algorithm>
#include <cstring>

namespace fqzcomp28 {

Workspace::Workspace(const DatasetMeta *meta)
    : meta_(meta), fmt_(meta->header_fmt),
      first_header_fields_(fromHeader(meta->first_header, fmt_)) {}

void CompressionWorkspace::encodeChunk(FastqChunk &chunk,
                                       CompressedBuffersDst &cbs) {
  prepareBuffersForEncoding(chunk, cbs);
  startNewChunk();
  comp_stats_.n_blocks++;

  seq_encoder.startChunk(cbs.seq);
  qual_encoder.startChunk(cbs.qual);

  assert(prev_header_fields_ == first_header_fields_);

  for (FastqRecord &r : chunk.records) {
    storeAsBytes(r.length, cbs.readlens);

    encodeHeader(r.header(), cbs);
    seq_encoder.encodeRecord(r, cbs);
    qual_encoder.encodeRecord(r);
  }

  // const std::size_t compressed_size_seq = seq_coder.endChunk();
  const std::size_t compressed_size_seq = seq_encoder.endChunk();
  const std::size_t compressed_size_qual = qual_encoder.endChunk();

  cbs.seq.resize(compressed_size_seq);
  cbs.qual.resize(compressed_size_qual);
  /* other fields are updated in compress misc buffers */
  comp_stats_.seq += compressed_size_seq;
  comp_stats_.qual += compressed_size_qual;

  cbs.original_size.n_records = chunk.records.size();
  cbs.original_size.total = chunk.raw_data.size();
  compressMiscBuffers(cbs);
}

void DecompressionWorkspace::decodeChunk(FastqChunk &chunk,
                                         CompressedBuffersSrc &cbs) {
  prepareFastqChunk(chunk, cbs);
  startNewChunk();

  decompressMiscBuffers(cbs);

  seq_decoder.startChunk(cbs.seq);
  qual_decoder.startChunk(cbs.qual);

  char *dst = chunk.raw_data.data();

  /* decompression is done in two passes:
   * the first one sets record pointers and decompresses headers,
   * the second decompresses sequence and quality into pointers */
  for (std::size_t i = 0, E = cbs.original_size.n_records; i < E; ++i) {
    auto &r = chunk.records[i];
    r.headerp = dst;
    r.header_length = static_cast<readlen_t>(decodeHeader(dst, cbs));
    dst += r.header_length;
    *dst++ = '\n';

    r.seqp = dst;
    r.length = loadFromBytes<readlen_t>(cbs.readlens, sizeof(readlen_t) * i);
    dst += r.length;
    *dst++ = '\n';

    *dst++ = '+';
    *dst++ = '\n';

    r.qualp = dst;
    dst += r.length;
    *dst++ = '\n';
  }

  assert(cbs.index.n_pos == cbs.n_pos.size());
  assert(cbs.index.n_count == cbs.n_count.size());
  for (std::size_t i = cbs.original_size.n_records; i > 0; --i) {
    seq_decoder.decodeRecord(chunk.records[i - 1], cbs);
    qual_decoder.decodeRecord(chunk.records[i - 1]);
  }
}

void Workspace::startNewChunk() {
  prev_header_fields_.assign(first_header_fields_.begin(),
                             first_header_fields_.end());
}

void CompressionWorkspace::encodeHeader(const std::string_view header,
                                        CompressedBuffersDst &cbs) {
  auto field_start = header.begin() + 1; /* skip '@' */
  for (std::size_t i = 0, E = fmt_.n_fields() - 1; i < E; ++i) {
    const auto field_end =
        std::find(field_start + 1, header.end(), fmt_.separators[i]);

    auto &storage = cbs.header_fields[i];
    auto &prev_value = prev_header_fields_[i];
    if (fmt_.field_types[i] == headers::FieldType::STRING) {
      storage.storeString(field_start, field_end,
                          std::get<headers::string_t>(prev_value));
    } else {
      storage.storeNumeric(field_start, field_end,
                           std::get<headers::numeric_t>(prev_value));
    }

    field_start = field_end + 1; /* skip separator */
  }

  // TODO: check how processing all fields inside the loop affects performance
  auto &storage = cbs.header_fields.back();
  auto &prev_value = prev_header_fields_.back();
  if (fmt_.field_types.back() == headers::FieldType::STRING) {
    storage.storeString(field_start, header.end(),
                        std::get<headers::string_t>(prev_value));
  } else {
    storage.storeNumeric(field_start, header.end(),
                         std::get<headers::numeric_t>(prev_value));
  }
}

unsigned DecompressionWorkspace::decodeHeader(char *dst,
                                              CompressedBuffersSrc &cbs) {
  char *const old_dst = dst;
  *dst++ = '@';

  for (std::size_t i = 0, E = fmt_.n_fields() - 1; i < E; ++i) {
    auto &storage = cbs.header_fields[i];
    auto &prev_value = prev_header_fields_[i];

    if (fmt_.field_types[i] == headers::FieldType::STRING) {
      dst +=
          storage.loadNextString(dst, std::get<headers::string_t>(prev_value));
    } else {
      dst += storage.loadNextNumeric(dst,
                                     std::get<headers::numeric_t>(prev_value));
    }

    *dst++ += fmt_.separators[i];
  }
  auto &storage = cbs.header_fields.back();
  auto &prev_value = prev_header_fields_.back();

  if (fmt_.field_types.back() == headers::FieldType::STRING) {
    dst += storage.loadNextString(dst, std::get<headers::string_t>(prev_value));
  } else {
    dst +=
        storage.loadNextNumeric(dst, std::get<headers::numeric_t>(prev_value));
  }

  return static_cast<unsigned>(dst - old_dst);
}

void CompressionWorkspace::prepareBuffersForEncoding(
    const FastqChunk &chunk, CompressedBuffersDst &cbs) {
  cbs.clear();

  // TODO: estimate compressed sizes for all buffers during
  // initial dataset analysis
  cbs.seq.resize(compressBoundSequence(chunk.tot_reads_length));
  cbs.qual.resize(compressBoundQuality(chunk.tot_reads_length));

  cbs.header_fields.resize(fmt_.n_fields());
  cbs.compressed_header_fields.resize(fmt_.n_fields());
  cbs.original_size.header_fields.resize(fmt_.n_fields());
  for (auto &field : cbs.header_fields)
    field.clear();
}

void CompressionWorkspace::compressMiscBuffers(CompressedBuffersDst &cbs) {
  // TODO: use narrow/checked_cast
  cbs.original_size.readlens = static_cast<uint32_t>(cbs.readlens.size());
  comp_stats_.readlens += compressBuffer(cbs.compressed_readlens, cbs.readlens);

  cbs.original_size.n_count = static_cast<uint32_t>(cbs.n_count.size());
  comp_stats_.n_count += compressBuffer(cbs.compressed_n_count, cbs.n_count);

  cbs.original_size.n_pos = static_cast<uint32_t>(cbs.n_pos.size());
  comp_stats_.n_pos += compressBuffer(cbs.compressed_n_pos, cbs.n_pos);

  for (std::size_t i = 0, E = fmt_.n_fields(); i < E; ++i) {
    const auto &field_data = cbs.header_fields[i];
    auto &field_cdata = cbs.compressed_header_fields[i];
    auto &field_csize = comp_stats_.header_fields[i];

    auto &original_size = cbs.original_size.header_fields[i];

    if (fmt_.field_types[i] == headers::FieldType::STRING) {
      field_csize += compressBuffer(field_cdata.isDifferentFlag,
                                    field_data.isDifferentFlag);
      field_csize += compressBuffer(field_cdata.content, field_data.content);
      field_csize +=
          compressBuffer(field_cdata.contentLength, field_data.contentLength);

      original_size.isDifferentFlag = field_data.isDifferentFlag.size();
      original_size.content = field_data.content.size();
      original_size.contentLength = field_data.contentLength.size();

    } else { /* NUMERIC */
      field_csize += compressBuffer(field_cdata.content, field_data.content);

      original_size.content = field_data.content.size();
    }
  }
}

void DecompressionWorkspace::decompressMiscBuffers(CompressedBuffersSrc &cbs) {
  // TODO refactor to loop over all buffers
  cbs.readlens.resize(cbs.original_size.readlens);
  memdecompress(cbs.readlens.data(), cbs.readlens.size(),
                cbs.compressed_readlens.data(), cbs.compressed_readlens.size());

  cbs.index.n_count = cbs.original_size.n_count;
  cbs.n_count.resize(cbs.original_size.n_count);
  memdecompress(cbs.n_count.data(), cbs.n_count.size(),
                cbs.compressed_n_count.data(), cbs.compressed_n_count.size());

  cbs.index.n_pos = cbs.original_size.n_pos;
  cbs.n_pos.resize(cbs.original_size.n_pos);
  memdecompress(cbs.n_pos.data(), cbs.n_pos.size(), cbs.compressed_n_pos.data(),
                cbs.compressed_n_pos.size());

  for (std::size_t i = 0, E = fmt_.n_fields(); i < E; ++i) {
    const auto &field_cdata = cbs.compressed_header_fields[i];
    const auto &original_size = cbs.original_size.header_fields[i];
    auto &field_data = cbs.header_fields[i];

    if (fmt_.field_types[i] == headers::FieldType::STRING) {
      field_data.isDifferentFlag.resize(original_size.isDifferentFlag);
      field_data.content.resize(original_size.content);
      field_data.contentLength.resize(original_size.contentLength);

      memdecompress(field_data.isDifferentFlag.data(),
                    field_data.isDifferentFlag.size(),
                    field_cdata.isDifferentFlag.data(),
                    field_cdata.isDifferentFlag.size());
      memdecompress(field_data.content.data(), field_data.content.size(),
                    field_cdata.content.data(), field_cdata.content.size());
      memdecompress(
          field_data.contentLength.data(), field_data.contentLength.size(),
          field_cdata.contentLength.data(), field_cdata.contentLength.size());
    } else {
      field_data.content.resize(original_size.content);
      memdecompress(field_data.content.data(), field_data.content.size(),
                    field_cdata.content.data(), field_cdata.content.size());
    }
  }
}

std::size_t
CompressionWorkspace::compressBuffer(std::vector<std::byte> &dst,
                                     const std::vector<std::byte> &src) {
  dst.resize(src.size() + extra_csize_misc);
  const std::size_t csize = memcompress(dst.data(), src.data(), src.size());
  dst.resize(csize);
  return csize;
}

void CompressionWorkspace::updateStats(const CompressedBuffersDst &cbs) {
  /* update stats */
  // TODO: also account for readlens and ns for sequences...
  comp_stats_.seq += cbs.seq.size();
  comp_stats_.qual += cbs.qual.size();
  // TODO: more detailed report on headers ?
  for (std::size_t i = 0, E = fmt_.n_fields(); i < E; ++i) {
    auto &csize = comp_stats_.header_fields[i];
    const auto &field = cbs.header_fields[i];
    if (fmt_.field_types[i] == headers::FieldType::STRING) {
      csize += field.isDifferentFlag.size() + field.content.size() +
               field.contentLength.size();
    } else
      csize += field.content.size();
  }
}
} // namespace fqzcomp28
