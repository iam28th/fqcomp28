#include "archive.h"

namespace fqzcomp28 {

/** Creates Archive to read compressed data from an existing fqzcomp28 file */
Archive::Archive(const path_t archive_path)
    : fs_(archive_path, std::ios_base::binary | std::ios_base::in) {
  readArchiveHeader();
}

Archive::Archive(const path_t archive_path, const path_t file_to_gather_meta,
                 const std::size_t sample_size_bytes)
    : fs_(archive_path, std::ios_base::binary | std::ios_base::out) {
  meta_ = analyzeDataset(file_to_gather_meta, sample_size_bytes);
  writeArchiveHeader();
}

void Archive::writeArchiveHeader() {
  // TODO: write parameters which are required for decompression
  // (once there're any ...)
  DatasetMeta::storeToStream(meta_, fs_);
}

void Archive::readArchiveHeader() { meta_ = DatasetMeta::loadFromStream(fs_); }

void Archive::writeBlock(const CompressedBuffersDst &cb) {
  writeInteger(cb.original_size.total);
  writeInteger(cb.original_size.n_records);

  /* basically, for each buffer we store original size,
   * followed by compressed size, followed by compressed data
   * then compressed data */
  writeInteger(cb.original_size.readlens);
  // writeInteger(cb.compressed_readlens.size());

#if 0
  fs_.write(to_char_ptr(&cb.original_size.total),
            sizeof(cb.original_size.total));

  fs_.write(to_char_ptr(&cb.original_size.total),
            sizeof(cb.original_size.total));
#endif
}

void Archive::readBlock(CompressedBuffersSrc &cb) {
  cb.original_size.total = readInteger<uint64_t>();
#if 0
  fs_.write(to_char_ptr(&cb.original_size.total),
            sizeof(cb.original_size.total));

  fs_.write(to_char_ptr(&cb.original_size.total),
            sizeof(cb.original_size.total));
#endif
}

void Archive::writeBytes(const std::vector<std::byte> &bytes) {
  const auto sz = static_cast<std::streamsize>(bytes.size());
  writeInteger(sz);
  fs_.write(to_char_ptr(bytes.data()), sz);
}
void Archive::readBytes(std::vector<std::byte> &bytes) {
  const auto sz = readInteger<std::streamsize>();
  bytes.resize(static_cast<std::size_t>(sz));
  fs_.read(to_char_ptr(bytes.data()), sz);
}

} // namespace fqzcomp28
