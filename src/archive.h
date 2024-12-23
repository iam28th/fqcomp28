#pragma once
#include "compressed_buffers.h"
#include "defs.h"
#include "prepare.h"
#include "settings.h"
#include <fstream>

namespace fqzcomp28 {
class Archive {
  // TODO {write,read}{Index}
  struct Index {};

public:
  /** Creates Archive to read compressed data from an existing fqzcomp28 file */
  Archive(const path_t archive_path);

  /**
   * @brief Creates an archive to write compressed data to
   *
   * @param file_to_gather_meta File on which meta (like header format and
   * frequency tables) is gathered
   * @param archive_path Archive creation path
   * @param sample_size_bytes How many bytes to process for calculating meta
   */
  Archive(const path_t archive_path, const path_t file_to_gather_meta,
          const std::size_t sample_size_bytes =
              Settings::getDefaultsInstance()->sample_chunk_size());

  void writeBlock(const CompressedBuffersDst &cb);
  void readBlock(const CompressedBuffersSrc &cb);

  void flush() { fs_.flush(); };
  auto size() const { return bytes_written; }

  const DatasetMeta &meta() const { return meta_; }

  friend struct ArchiveTester;

private:
  void writeHeader();
  void readHeader();

private:
  uint64_t bytes_written = 0;
  std::fstream fs_;
  DatasetMeta meta_;
};

} // namespace fqzcomp28
