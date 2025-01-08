#include "defs.h"
#include "memcompress.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>

using namespace fqcomp28;

/** compress, decompress back & check integrity */
TEST_CASE("memcompress") {
  constexpr int LIBBSC_HEADER_SIZE = 28;
  const path_t file = "test/data/SRR065390_sub_1.fastq";
  const std::vector<char> inp = loadFileContents(file);
  std::vector<std::byte> compressed(inp.size() + LIBBSC_HEADER_SIZE, BYTE0);
  std::vector<char> decompressed(inp.size(), 0);

  const std::size_t csize =
      memcompress(compressed.data(),
                  reinterpret_cast<const std::byte *>(inp.data()), inp.size());
  compressed.resize(csize);

  const std::size_t dec_size =
      memdecompress(reinterpret_cast<std::byte *>(decompressed.data()),
                    decompressed.size(), compressed.data(), compressed.size());
  CHECK(dec_size == inp.size());
  CHECK(inp == decompressed);
}
