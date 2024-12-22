#include "encoding_context.h"
#include "headers.h"
#include "utils.h"
#include <algorithm>
#include <charconv>
#include <cstring>

namespace fqzcomp28 {

EncodingContext::EncodingContext(const DatasetMeta *meta) : meta_(meta) {
  const auto &fmt = meta_->header_fmt;
  const std::string_view first_header(meta_->first_header);

  initalizeHeaderFields(first_header_fields_, fmt);
  initalizeHeaderFields(prev_header_fields_, fmt);

  auto field_start = first_header.begin() + 1; /* skip '@' */
  for (std::size_t field_idx = 0, E = fmt.n_fields() - 1; field_idx < E;
       ++field_idx) {
    const auto field_end = std::find(field_start + 1, first_header.end(),
                                     fmt.separators[field_idx]);
    convertHeaderFieldFromAscii(field_start, field_end,
                                first_header_fields_[field_idx],
                                fmt.field_types[field_idx]);
    field_start = field_end + 1; /* skip separator */
  }
  convertHeaderFieldFromAscii(field_start, first_header.end(),
                              first_header_fields_.back(),
                              fmt.field_types.back());

  comp_stats_.header_fields.resize(fmt.n_fields());
}

void EncodingContext::encodeChunk(const FastqChunk &chunk,
                                  CompressedBuffers &cbs) {
  prepareCompressedBuffers(chunk, cbs);
  startNewChunk();

  std::byte *dst_seq = cbs.seq.data();
  std::byte *dst_qual = cbs.qual.data();

  for (const FastqRecord &r : chunk.records) {
    encodeHeader(r.header(), cbs);

    // I want to check the general workflow first,
    // so for now - just copy sequence and qualities
    cbs.readlens.push_back(r.length);

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

  updateStats(cbs);
}

void EncodingContext::decodeChunk(FastqChunk &chunk, CompressedBuffers &cbs) {
  prepareFastqChunk(chunk, cbs);
  startNewChunk();

  char *dst = chunk.raw_data.data();
  const std::byte *src_seq = cbs.seq.data();
  const std::byte *src_qual = cbs.qual.data();

  // TODO: might skip assigning record fields as not necessary
  for (std::size_t i = 0, E = cbs.n_records(); i < E; ++i) {
    auto &r = chunk.records[i];
    r.headerp = dst;
    r.header_length = static_cast<readlen_t>(decodeHeader(dst, cbs));
    dst += r.header_length;
    *dst++ = '\n';

    r.seqp = dst;
    r.length = cbs.readlens[i];
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
                                   CompressedBuffers &cbs) {
  const auto &fmt = meta_->header_fmt;
  auto field_start = header.begin() + 1; /* skip '@' */
  for (std::size_t i = 0, E = fmt.n_fields() - 1; i < E; ++i) {
    const auto field_end =
        std::find(field_start + 1, header.end(), fmt.separators[i]);

    auto &storage = cbs.header_fields_in[i];
    auto &prev_value = prev_header_fields_[i];
    if (fmt.field_types[i] == headers::FieldType::STRING) {
      storage.storeString(field_start, field_end,
                          std::get<headers::string_t>(prev_value));
    } else {
      storage.storeNumeric(field_start, field_end,
                           std::get<headers::numeric_t>(prev_value));
    }

    field_start = field_end + 1; /* skip separator */
  }

  // TODO: check how processing all fields inside the loop affects performance
  auto &storage = cbs.header_fields_in.back();
  auto &prev_value = prev_header_fields_.back();
  if (fmt.field_types.back() == headers::FieldType::STRING) {
    storage.storeString(field_start, header.end(),
                        std::get<headers::string_t>(prev_value));
  } else {
    storage.storeNumeric(field_start, header.end(),
                         std::get<headers::numeric_t>(prev_value));
  }
}

unsigned EncodingContext::decodeHeader(char *dst, CompressedBuffers &cbs) {
  auto &fmt = meta_->header_fmt;
  char *const old_dst = dst;
  *dst++ = '@';

  for (std::size_t i = 0, E = fmt.n_fields() - 1; i < E; ++i) {

    auto &storage = cbs.header_fields_out[i];
    auto &prev_value = prev_header_fields_[i];
    if (fmt.field_types[i] == headers::FieldType::STRING) {
      dst +=
          storage.loadNextString(dst, std::get<headers::string_t>(prev_value));
    } else {
      dst += storage.loadNextNumeric(dst,
                                     std::get<headers::numeric_t>(prev_value));
    }

    *dst++ += fmt.separators[i];
  }

  auto &storage = cbs.header_fields_out.back();
  auto &prev_value = prev_header_fields_.back();
  if (fmt.field_types.back() == headers::FieldType::STRING) {
    dst += storage.loadNextString(dst, std::get<headers::string_t>(prev_value));
  } else {
    dst +=
        storage.loadNextNumeric(dst, std::get<headers::numeric_t>(prev_value));
  }

  return static_cast<unsigned>(dst - old_dst);
}

void EncodingContext::updateStats(const CompressedBuffers &cbs) {
  /* update stats */
  // TODO: also readlens and ns for sequences...
  comp_stats_.seq += cbs.seq.size();
  comp_stats_.qual += cbs.qual.size();
  const auto &fmt = meta_->header_fmt;
  // TODO: will call further compression on headers
  // TODO: more detailed report on headers ?
  for (std::size_t i = 0, E = fmt.n_fields(); i < E; ++i) {
    auto &csize = comp_stats_.header_fields[i];
    const auto &field = cbs.header_fields_in[i];
    if (fmt.field_types[i] == headers::FieldType::STRING) {
      csize += field.isDifferentFlag.size() + field.content.size() +
               field.contentLength.size();
    } else
      csize += field.content.size();
  }
}

void EncodingContext::initalizeHeaderFields(
    header_fields_t &fields, const headers::HeaderFormatSpeciciation &fmt) {
  fields.resize(fmt.n_fields());
  for (std::size_t i = 0; i < fmt.n_fields(); ++i) {
    if (fmt.field_types[i] == headers::FieldType::STRING)
      fields[i] = headers::string_t{};
    else
      fields[i] = headers::numeric_t{};
  }
}

void EncodingContext::convertHeaderFieldFromAscii(
    std::string_view::iterator field_start,
    std::string_view::iterator field_end, headers::field_data_t &to,
    headers::FieldType typ) {
  if (typ == headers::FieldType::STRING)
    std::get<headers::string_t>(to) = {field_start, field_end};
  else {
    headers::numeric_t &val = std::get<headers::numeric_t>(to);
    [[maybe_unused]] auto [_, ec] =
        std::from_chars(field_start, field_end, val);
    assert(ec == std::errc()); /* no error */
  }
}

} // namespace fqzcomp28
