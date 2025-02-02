#pragma once
#include "compressed_buffers.h"
#include "defs.h"
#include "headers.h"

namespace fqcomp28 {

/**
 * function to load entire file in memory
 */
void loadFileContents(const fqcomp28::path_t path, std::vector<char> &data);
std::vector<char> loadFileContents(const fqcomp28::path_t path);
FastqChunk loadFastqFileContents(const fqcomp28::path_t path);

inline headers::FieldStorageSrc
convertToSrcStorage(headers::FieldStorageDst &&dst) {
  headers::FieldStorageSrc src;
  src.isDifferentFlag = std::move(dst.isDifferentFlag);
  src.content = std::move(dst.content);
  src.contentLength = std::move(dst.contentLength);
  return src;
}

inline CompressedBuffersSrc convertToSrcBuffers(CompressedBuffersDst &&in) {
  CompressedBuffersSrc src;

  src.original_size = std::move(in.original_size);
  src.seq = std::move(in.seq);
  src.qual = std::move(in.qual);

  src.n_count = std::move(in.n_count);
  src.index.n_count = src.n_count.size();
  src.compressed_n_count = std::move(in.compressed_n_count);

  src.n_pos = std::move(in.n_pos);
  src.index.n_pos = src.n_pos.size();
  src.compressed_n_pos = std::move(in.compressed_n_pos);

  src.readlens = std::move(in.readlens);
  src.compressed_readlens = std::move(in.compressed_readlens);

  for (auto &hf : in.header_fields)
    src.header_fields.push_back(convertToSrcStorage(std::move(hf)));

  src.compressed_header_fields = std::move(in.compressed_header_fields);

  return src;
}

} // namespace fqcomp28
