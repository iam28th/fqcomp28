#pragma once
#include "headers.h"
#include <vector>

namespace fqzcomp28 {

/**
 * helper struct to load original sizes into at decompression
 */
struct cb_original_sizes_t {
  /* how many bytes to allocate for each header field */
  std::vector<headers::FieldStorage::sizes> header_fields;
  /* how many bytes to allocate for raw data */
  uint64_t total;
  /* how many bytes to allocate for CompressedBuffers::readlens */
  uint64_t readlens;
  /* number of fastq records in chunk */
  uint64_t n_records;

  uint64_t n_count, n_pos;

  bool operator==(const cb_original_sizes_t &) const = default;

  void clear() {
    total = 0;
    readlens = 0;
    n_records = 0;
    n_count = n_pos = 0;
    for (auto &sz : header_fields)
      sz = {};
  }
};

template <typename T>
  requires std::is_base_of_v<headers::FieldStorage, T>
struct CompressedBuffers {
  CompressedBuffers() = default;
  std::vector<std::byte> seq, qual;

  std::vector<std::byte> readlens;
  std::vector<std::byte> compressed_readlens;

  std::vector<T> header_fields;
  std::vector<headers::CompressedFieldStorage> compressed_header_fields;

  std::vector<std::byte> n_count;
  std::vector<std::byte> compressed_n_count;

  std::vector<std::byte> n_pos;
  std::vector<std::byte> compressed_n_pos;

  cb_original_sizes_t original_size;

  void clear() {
    seq.clear();
    qual.clear();

    readlens.clear(), compressed_readlens.clear();

    for (auto &hf : header_fields)
      hf.clear();
    for (auto &chf : compressed_header_fields)
      chf.clear();

    original_size.clear();
  }
};

/** Used at compression */
using CompressedBuffersDst = CompressedBuffers<headers::FieldStorageDst>;
/** Used at decompression */
using CompressedBuffersSrc = CompressedBuffers<headers::FieldStorageSrc>;

} // namespace fqzcomp28
