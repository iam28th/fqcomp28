#pragma once
#include "defs.h"
#include "headers.h"

namespace fqzcomp28 {

/**
 * function to load entire file in memory
 */
void loadFileContents(const fqzcomp28::path_t path, std::vector<char> &data);
std::vector<char> loadFileContents(const fqzcomp28::path_t path);
FastqChunk loadFastqFileContents(const fqzcomp28::path_t path);

inline headers::FieldStorageOut
convertToOutStorage(headers::FieldStorageIn &&in) {
  headers::FieldStorageOut out;
  out.isDifferentFlag = std::move(in.isDifferentFlag);
  out.content = std::move(in.content);
  out.contentLength = std::move(in.contentLength);
  return out;
}

} // namespace fqzcomp28
