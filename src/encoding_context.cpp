#include "encoding_context.h"
#include "headers.h"
#include "utils.h"
#include <algorithm>
#include <charconv>
#include <cstring>

namespace fqzcomp28 {

EncodingContext::EncodingContext(const DatasetMeta *meta) : meta_(meta) {
  const auto &fmt = meta->header_fmt;
  const std::string_view first_header(meta->first_header);

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
}

void EncodingContext::encodeChunk(const FastqChunk &chunk,
                                  CompressedBuffers &cbs) {
  cbs.clear();
  cbs.seq.resize(compressBoundSequence(chunk.tot_reads_length));
  cbs.qual.resize(compressBoundQuality(chunk.tot_reads_length));

  startNewChunk();

  // TODO: a fn to preallocate space in cbs based on chunk

  for (const FastqRecord &r : chunk.records) {
    encodeHeader(r.header(), cbs);

    // I want to check the general workflow first,
    // so for now - just copy sequence and qualities
    cbs.readlens.push_back(r.length);
    cbs.seq.insert(cbs.seq.end(), to_byte_ptr(r.seqp),
                   to_byte_ptr(r.seqp + r.length));
    cbs.qual.insert(cbs.qual.end(), to_byte_ptr(r.qualp),
                    to_byte_ptr(r.qualp + r.length));
  }
}

void EncodingContext::decodeChunk(FastqChunk &chunk, CompressedBuffers &cbs) {
  startNewChunk();
#if 0
  chunk.records.resize(cbs.n_records());

  // TODO: use unsigned char everywhere
  char *dst = chunk.raw_data.data();
  const std::byte *src_seq = cbs.seq.data();
  const std::byte *src_qual = cbs.qual.data();

  for (std::size_t i = 0, E = cbs.n_records(); i < E; ++i) {
    auto &r = chunk.records[i];

    r.header_length = decodeHeader(dst, cbs, i);
    dst += r.header_length;
    *dst++ = '\n';

    r.length = cbs.readlens[i];
    std::memcpy(dst, src_seq, r.length);
    *dst++ = '\n';

    std::memcpy(dst, src_qual, r.length);
    *dst++ = '\n';

    src_seq += r.length;
    src_qual += r.length;
  }
#endif
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

/**
 * put default-constructed value into each field according to header format
 */
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
