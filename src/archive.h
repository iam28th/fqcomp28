#pragma once
#include "compressed_buffers.h"
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
  struct Header {};
  struct Index {};

  unsigned long long writeDataBlock(const CompressedBuffers &cb);
};

} // namespace fqzcomp28
