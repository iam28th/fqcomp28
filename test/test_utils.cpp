#include "test_utils.h"
#include "defs.h"
#include "fastq_io.h"
#include <fstream>

namespace fqcomp28 {

void loadFileContents(path_t const path, std::vector<char> &buf) {
  const auto fsize = std::filesystem::file_size(path);
  buf.resize(fsize);
  std::ifstream ifs(path);
  ifs.read(buf.data(), static_cast<std::streamsize>(fsize));
}

std::vector<char> loadFileContents(path_t const path) {
  std::vector<char> buf;
  loadFileContents(path, buf);
  return buf;
}

FastqChunk loadFastqFileContents(const fqcomp28::path_t path) {
  FastqChunk chunk;
  chunk.raw_data = loadFileContents(path);
  FastqReader::parseRecords(chunk);
  return chunk;
}

} // namespace fqcomp28
