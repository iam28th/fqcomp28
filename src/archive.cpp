#include "archive.h"

namespace fqzcomp28 {

/** Creates Archive to read compressed data from an existing fqzcomp28 file */
Archive::Archive(const path_t archive_path)
    : fs_(archive_path, std::ios_base::binary | std::ios_base::in) {
  readHeader();
}

Archive::Archive(const path_t archive_path, const path_t file_to_gather_meta,
                 const std::size_t sample_size_bytes)
    : fs_(archive_path, std::ios_base::binary | std::ios_base::out) {
  meta_ = analyzeDataset(file_to_gather_meta, sample_size_bytes);
  writeHeader();
}

void Archive::writeHeader() {
  // TODO: write parameters which are required for decompression
  // (once there're any ...)
  DatasetMeta::storeToStream(meta_, fs_);
}

void Archive::readHeader() { meta_ = DatasetMeta::loadFromStream(fs_); }

} // namespace fqzcomp28
