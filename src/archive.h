#pragma once
#include "compressed_buffers.h"
#include "prepare.h"
#include "settings.h"
#include <filesystem>
#include <string_view>

namespace fqzcomp28 {
class Archive {
  using path_t = std::filesystem::path;

  const path_t path_;

public:
  Archive(std::string_view path) : path_(path) {}

  // TODO {write,read}{Header,Index}
  struct Index {};

  unsigned writeHeader(const DatasetMeta &);
  unsigned long long writeDataBlock(const CompressedBuffers &cb);

  auto size() const { return bytes_written; }

private:
  uint64_t bytes_written = 0;
};

} // namespace fqzcomp28
