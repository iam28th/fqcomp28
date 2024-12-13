#pragma once
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

namespace fqzcomp28 {

using path_t = std::filesystem::path;

using FastqData = std::vector<char>;

/**
 * non-owning - holds pointers into outside allocated data
 */
struct FastqRecord {
  // TODO: make parsing a static method of FastqRecord?
  // then fields can be made private...
  // TODO: two string_view-s of the same size is redundant
public:
  uint32_t length, header_length;
  const char *seqp, *qualp, *headerp;

  auto header() const { return std::string_view(headerp, header_length); }
  auto seq() const { return std::string_view(seqp, length); }
  auto qual() const { return std::string_view(qualp, length); }
};

struct FastqChunk {
  FastqData raw_data;
  std::vector<FastqRecord> reads;
  void clear() {
    raw_data.clear();
    reads.clear();
  }
};

} // namespace fqzcomp28
