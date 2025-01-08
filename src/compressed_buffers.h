#pragma once
#include "headers.h"
#include <vector>

namespace fqcomp28 {

/**
 * helper struct to load original sizes into at decompression
 */
struct cb_original_sizes_t {
  /* how many bytes to allocate for each header field */
  std::vector<headers::FieldStorage::sizes> header_fields;
  /* how many bytes to allocate for raw data */
  uint32_t total;
  /* how many bytes to allocate for CompressedBuffers::readlens */
  uint32_t readlens;
  /* number of fastq records in chunk */
  uint32_t n_records;

  uint32_t n_count, n_pos;

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

struct CompressedBuffers {
protected:
  CompressedBuffers() = default;
  virtual ~CompressedBuffers() = default;

public:
  std::vector<std::byte> seq, qual;

  std::vector<std::byte> readlens;
  std::vector<std::byte> compressed_readlens;

  std::vector<headers::CompressedFieldStorage> compressed_header_fields;

  std::vector<std::byte> n_count;
  std::vector<std::byte> compressed_n_count;

  std::vector<std::byte> n_pos;
  std::vector<std::byte> compressed_n_pos;

  cb_original_sizes_t original_size;

  /** position of the chunk in the input file */
  uint32_t chunk_idx;

  virtual void clear() {
    seq.clear();
    qual.clear();

    readlens.clear(), compressed_readlens.clear();

    for (auto &chf : compressed_header_fields)
      chf.clear();

    original_size.clear();
  }
};

/** Used at compression */
class CompressedBuffersDst : public CompressedBuffers {
public:
  std::vector<headers::FieldStorageDst> header_fields;

  void clear() override {
    CompressedBuffers::clear();

    for (auto &hf : header_fields)
      hf.clear();
  }
};

/** Used at decompression */
class CompressedBuffersSrc : public CompressedBuffers {
public:
  std::vector<headers::FieldStorageSrc> header_fields;

  // required, becuase sequences are encoded in decoded in a different order
  struct {
    std::size_t n_count = 0;
    std::size_t n_pos = 0;
  } index;

  void clear() override {
    CompressedBuffers::clear();
    for (auto &hf : header_fields)
      hf.clear();
    index = {};
  }
};
} // namespace fqcomp28
