#pragma once
#include "compressed_buffers.h"
#include "defs.h"
#include "prepare.h"
#include "settings.h"
#include "utils.h"
#include <fstream>
#include <mutex>

namespace fqzcomp28 {
/**
 * Archive structure:
 * - Number of data blocks (uin32_t)
 * - Metadata block (frequency tables etc.)
 * - Data block 1, data block 2, ..., data block N
 * - Index (n_blocks * sizeof(BlockInfo) bytes), describing
 *   offset and original order or each block
 */
class Archive {
  /** Describes location and size of a data block in the archive file */
  struct BlockInfo {
    int64_t offset;
    /** Position (order) of the corresponding chunk in the input file */
    uint32_t idx;

    bool operator==(const BlockInfo &other) const = default;
  };

  /** Offset at which metadata is located in the file */
  constexpr static std::streamoff OFFSET_META = sizeof(uint32_t);

public:
  /** Creates Archive to read compressed data from an existing fqzcomp28 file */
  explicit Archive(const path_t archive_path);

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
  bool readBlock(CompressedBuffersSrc &cb);

  void writeIndex();
  void flush() { fs_.flush(); };

  const DatasetMeta &meta() const { return meta_; }

  /** @return Number of bytes occupied by the idnex */
  [[nodiscard]] std::size_t indexBytes() const {
    return sizeof(uint32_t) + index_.size() * sizeof(BlockInfo);
  }

  friend struct ArchiveTester;

private:
  void writeMeta();

  void readArchiveHeader();

  void writeBytes(const std::vector<std::byte> &);
  void readBytes(std::vector<std::byte> &);

  template <std::integral T> void writeInteger(const T val) {
    fs_.write(to_char_ptr(&val), sizeof(T));
  };

  template <std::integral T> T readInteger() {
    T ret;
    fs_.read(to_char_ptr(&ret), sizeof(T));
    return ret;
  };

private:
  /**
   * Sorts index entries according to the order of corresponding input chunks
   * in the original file
   */
  void sortIndex() {
    std::sort(
        index_.begin(), index_.end(),
        [](const auto &lhs, const auto &rhs) { return lhs.idx < rhs.idx; });
  }

  std::vector<BlockInfo> index_;
  std::size_t blocks_processed_;
  std::fstream fs_;
  DatasetMeta meta_;
  std::mutex mtx_;
};

} // namespace fqzcomp28
