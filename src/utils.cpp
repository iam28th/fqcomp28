#include "utils.h"
#include <fstream>

namespace fqzcomp28 {

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

}; // namespace fqzcomp28
