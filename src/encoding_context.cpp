#include "encoding_context.h"
#include "headers.h"
#include <algorithm>
#include <charconv>
#include <cstring>

namespace fqzcomp28 {

EncodingContext::EncodingContext(const DatasetMeta *meta) : meta_(meta) {
  auto &fmt = meta->header_fmt;
  std::string_view first_header(meta->first_header);

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
#if 0
  cbs.seq.resize(compressBoundSequence(chunk.tot_reads_length));
  cbs.qual.resize(compressBoundQuality(chunk.tot_reads_length));
#endif

  startNewChunk();

  // TODO: a fn to preallocate space in cbs based on chunk

  for (const FastqRecord &r : chunk.records) {
    encodeHeader(r.header(), cbs);

    // I want to check the general workflow first,
    // so for now - just copy sequence and qualities
    cbs.readlens.push_back(r.length);

    for (auto p : {r.seqp, r.qualp}) {
      cbs.seq.insert(cbs.seq.end(), reinterpret_cast<const std::byte *>(p),
                     reinterpret_cast<const std::byte *>(p + r.length));
    }
  }
}

void EncodingContext::decodeChunk(FastqChunk &chunk,
                                  const CompressedBuffers &cbs) {
  startNewChunk();
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
}

void EncodingContext::startNewChunk() {
  prev_header_fields_.assign(first_header_fields_.begin(),
                             first_header_fields_.end());
}

void EncodingContext::encodeHeader(std::string_view header,
                                   CompressedBuffers &cbs) {
  auto &fmt = meta_->header_fmt;
  auto field_start = header.begin() + 1; /* skip '@' */
  for (std::size_t field_idx = 0, E = fmt.n_fields() - 1; field_idx < E;
       ++field_idx) {

    const auto field_end =
        std::find(field_start + 1, header.end(), fmt.separators[field_idx]);

    storeHeaderField(field_start, field_end, cbs.header_fields[field_idx],
                     field_idx);

    field_start = field_end + 1; /* skip separator */
  }

  storeHeaderField(field_start, header.end(), cbs.header_fields.back(),
                   fmt.n_fields() - 1);
}

readlen_t EncodingContext::decodeHeader(char *dst, const CompressedBuffers &cbs,
                                        std::size_t read_idx) {
  auto &fmt = meta_->header_fmt;
  char *const old_dst = dst;
  *dst++ = '@';

  for (std::size_t field_idx = 0, E = fmt.n_fields() - 1; field_idx < E;
       ++field_idx) {

    dst += loadHeaderField(dst, cbs.header_fields[field_idx], field_idx);
  }

  // TODO: test for store/load header field
  dst += loadHeaderField(dst, cbs.header_fields.back(), fmt.n_fields() - 1);

  return static_cast<readlen_t>(dst - old_dst);
}

void EncodingContext::storeHeaderField(std::string_view::iterator field_start,
                                       std::string_view::iterator field_end,
                                       headers::FieldStorage &storage,
                                       std::size_t field_idx) {
#if 0
  const auto &fmt = meta_->header_fmt;

  if (fmt.field_types[field_idx] == headers::FieldType::STRING) {
    headers::storeString(
        field_start, field_end,
        std::get<headers::string_t>(prev_header_fields_[field_idx]), storage);
  } else {
    headers::storeNumeric(
        field_start, field_end,
        std::get<headers::numeric_t>(prev_header_fields_[field_idx]), storage);
  }
#endif
}

readlen_t EncodingContext::loadHeaderField(char *dst,
                                           const headers::FieldStorage &storage,
                                           std::size_t field_idx) {}

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

} // namespace fqzcomp28
