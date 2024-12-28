#pragma once
#include <filesystem>
#include <string_view>
#include <vector>

namespace fqzcomp28 {

static_assert(std::is_same_v<uint64_t, std::size_t>);

template <typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

using path_t = std::filesystem::path;
using readlen_t = uint16_t;

constexpr std::byte BYTE0{0x00};
constexpr std::byte BYTE1{0x01};

using FastqData = std::vector<char>;

/**
 * non-owning - holds pointers into outside allocated data
 */
struct FastqRecord {
  // TODO: make parsing a static method of FastqRecord?
  // then fields can be made private...
public:
  char *seqp, *qualp, *headerp;
  readlen_t length, header_length;

  auto header() const { return std::string_view(headerp, header_length); }
  auto seq() const { return std::string_view(seqp, length); }
  auto qual() const { return std::string_view(qualp, length); }
};

struct FastqChunk {
  FastqData raw_data;
  std::vector<FastqRecord> records;

  /** accamulated over all reads */
  std::size_t tot_reads_length = 0;
  std::size_t headers_length = 0;
  void clear() {
    tot_reads_length = 0;
    headers_length = 0;
    raw_data.clear();
    records.clear();
  }
};

} // namespace fqzcomp28
