#pragma once
#include "compressed_buffers.h"
#include "defs.h"
#include "headers.h"

namespace fqzcomp28 {

/**
 * function to load entire file in memory
 */
void loadFileContents(const fqzcomp28::path_t path, std::vector<char> &data);
std::vector<char> loadFileContents(const fqzcomp28::path_t path);
FastqChunk loadFastqFileContents(const fqzcomp28::path_t path);

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
  src.original_size = in.original_size;
  src.seq = std::move(in.seq);
  src.qual = std::move(in.qual);
  src.readlens = std::move(in.readlens);
  for (auto &hf : in.header_fields)
    src.header_fields.push_back(convertToSrcStorage(std::move(hf)));

  return src;
}

} // namespace fqzcomp28
