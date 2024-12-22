#include "archive.h"

namespace fqzcomp28 {
void Archive::writeArchiveHeader(const DatasetMeta &meta) {
  // TODO: write parameters which are required for decompression
  // (once there're any ...)
  DatasetMeta::storeToStream(meta, fs_);
}
} // namespace fqzcomp28
