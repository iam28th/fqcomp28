#pragma once
#include "compressed_buffers.h"
#include "prepare.h"
#include "settings.h"
#include <filesystem>
#include <fstream>

namespace fqzcomp28 {
class Archive {
  using path_t = std::filesystem::path;

public:
  Archive(path_t path) {
    auto mode = std::ios_base::binary;
    const auto set = Settings::getInstance();
    if (set->non_storable.cmd == Command::COMPRESS)
      mode |= std::ios_base::out;
    else
      mode |= std::ios_base::in;
    fs_.open(path, mode);
  }

  // TODO {write,read}{Header,Index}
  struct Index {};

  void writeArchiveHeader(const DatasetMeta &);
  void writeDataBlock(const CompressedBuffersDst &cb);

  auto size() const { return bytes_written; }

private:
  uint64_t bytes_written = 0;
  std::fstream fs_;
};

} // namespace fqzcomp28
