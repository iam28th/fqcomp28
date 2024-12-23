#include "encoding_context.h"
#include "headers.h"
#include "memcompress.h"
#include "utils.h"
#include <algorithm>
#include <cstring>

namespace fqzcomp28 {

EncodingContext::EncodingContext(const DatasetMeta *meta)
    : meta_(meta),
      fmt_(headers::HeaderFormatSpeciciation::fromHeader(meta->first_header)),
      first_header_fields_(fromHeader(meta->first_header, fmt_)) {

  comp_stats_.header_fields.resize(fmt_.n_fields());
}

void EncodingContext::encodeChunk(const FastqChunk &chunk,
                                  CompressedBuffersDst &cbs) {
  prepareBuffersForEncoding(chunk, cbs);
  startNewChunk();

  assert(prev_header_fields_ == first_header_fields_);

  std::byte *dst_seq = cbs.seq.data();
  std::byte *dst_qual = cbs.qual.data();

  for (const FastqRecord &r : chunk.records) {
    encodeHeader(r.header(), cbs);

    storeAsBytes(r.length, cbs.readlens);

    // I want to check the general workflow first,
    // so for now - just copy sequence and qualities
    std::memcpy(dst_seq, to_byte_ptr(r.seqp), r.length);
    std::memcpy(dst_qual, to_byte_ptr(r.qualp), r.length);

    dst_seq += r.length, dst_qual += r.length;
  }

  const std::size_t compressed_size_seq =
      static_cast<std::size_t>(dst_seq - cbs.seq.data());
  const std::size_t compressed_size_qual =
      static_cast<std::size_t>(dst_qual - cbs.qual.data());

  cbs.seq.resize(compressed_size_seq);
  cbs.qual.resize(compressed_size_qual);
  /* other fields are updated in compress misc buffers */
  comp_stats_.seq += compressed_size_seq;
  comp_stats_.qual += compressed_size_seq;

  cbs.original_size.n_records = chunk.records.size();
  cbs.original_size.total = chunk.raw_data.size();
  compressMiscBuffers(cbs);
}

void EncodingContext::decodeChunk(FastqChunk &chunk,
                                  CompressedBuffersSrc &cbs) {
  prepareFastqChunk(chunk, cbs);
  startNewChunk();

  decompressMiscBuffers(cbs);

  char *dst = chunk.raw_data.data();
  const std::byte *src_seq = cbs.seq.data();
  const std::byte *src_qual = cbs.qual.data();

  // TODO: might skip assigning record fields as not necessary
  for (std::size_t i = 0, E = cbs.original_size.n_records; i < E; ++i) {
    auto &r = chunk.records[i];
    r.headerp = dst;
    r.header_length = static_cast<readlen_t>(decodeHeader(dst, cbs));
    dst += r.header_length;
    *dst++ = '\n';

    r.seqp = dst;
    r.length = loadFromBytes<readlen_t>(cbs.readlens, 2 * i);
    std::memcpy(dst, src_seq, r.length);
    dst += r.length;
    *dst++ = '\n';

    r.qualp = dst;
    std::memcpy(dst, src_qual, r.length);
    dst += r.length;
    *dst++ = '\n';

    src_seq += r.length;
    src_qual += r.length;
  }
}

void EncodingContext::startNewChunk() {
  prev_header_fields_.assign(first_header_fields_.begin(),
                             first_header_fields_.end());
}

void EncodingContext::encodeHeader(const std::string_view header,
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

unsigned EncodingContext::decodeHeader(char *dst, CompressedBuffersSrc &cbs) {
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

void EncodingContext::prepareBuffersForEncoding(const FastqChunk &chunk,
                                                CompressedBuffersDst &cbs) {
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

void EncodingContext::compressMiscBuffers(CompressedBuffersDst &cbs) {
  // TODO: N counts, N pos...
  cbs.original_size.readlens = static_cast<uint32_t>(cbs.readlens.size());
  comp_stats_.readlens += compressBuffer(cbs.compressed_readlens, cbs.readlens);

  for (std::size_t i = 0, E = fmt_.n_fields(); i < E; ++i) {
    auto &field_csize = comp_stats_.header_fields[i];
    auto &field_data = cbs.header_fields[i];
    auto &field_cdata = cbs.compressed_header_fields[i];

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

void EncodingContext::decompressMiscBuffers(CompressedBuffersSrc &cbs) {
  cbs.readlens.resize(cbs.original_size.readlens);
  memdecompress(cbs.readlens.data(), cbs.compressed_readlens.data(),
                cbs.compressed_readlens.size());

  for (std::size_t i = 0, E = fmt_.n_fields(); i < E; ++i) {
    auto &field_data = cbs.header_fields[i];
    auto &field_cdata = cbs.compressed_header_fields[i];

    auto &original_size = cbs.original_size.header_fields[i];
    if (fmt_.field_types[i] == headers::FieldType::STRING) {
      field_data.isDifferentFlag.resize(original_size.isDifferentFlag);
      field_data.content.resize(original_size.content);
      field_data.contentLength.resize(original_size.contentLength);

      memdecompress(field_data.isDifferentFlag.data(),
                    field_cdata.isDifferentFlag.data(),
                    field_cdata.isDifferentFlag.size());
      memdecompress(field_data.content.data(), field_cdata.content.data(),
                    field_cdata.content.size());
      memdecompress(field_data.contentLength.data(),
                    field_cdata.contentLength.data(),
                    field_cdata.contentLength.size());
    } else {
      field_data.content.resize(original_size.content);
      memdecompress(field_data.content.data(), field_cdata.content.data(),
                    field_cdata.content.size());
    }
  }
}

std::size_t EncodingContext::compressBuffer(std::vector<std::byte> &dst,
                                            const std::vector<std::byte> &src) {
  dst.resize(src.size() + extra_cbuffer_size);
  const std::size_t csize = memcompress(dst.data(), src.data(), src.size());
  dst.resize(csize);
  return csize;
}

void EncodingContext::updateStats(const CompressedBuffersDst &cbs) {
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
