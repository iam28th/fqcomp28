#include "fastq_io.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <ranges>

using namespace fqcomp28;

namespace fs = std::filesystem;

/**
 * reads file with different chunk size, writes in back to disc,
 * compares that contents are the same
 */
TEST_CASE("fastq_io SE") {
  const path_t folder = "test/data",
               input = folder / "SRR065390_1_first5.fastq",
               output = folder / "out1.fastq";
  REQUIRE(fs::exists(input));

  using file_contents_t = std::vector<char>;
  const file_contents_t in_contents = loadFileContents(input);
  file_contents_t out_contents;

  /* in practice we use block_size >> record_size */
  constexpr std::size_t min_block_size = 1000;
  const std::size_t max_block_size = fs::file_size(input);

  SECTION("checks") {
#if 1
    for (const auto block_size :
         std::views::iota(min_block_size, max_block_size + 1)) {
#else /* to debug some particular value */
    unsigned val = 1002;
    for (const std::size_t block_size : std::views::iota(val, val + 1)) {
#endif
      FastqChunk chunk;
      FastqReader reader(input, block_size);
      FastqWriter writer(output);

      while (reader.readNextChunk(chunk)) {
        writer.writeChunk(chunk);
        chunk.clear();
      }
      writer.flush();

      loadFileContents(output, out_contents);
      REQUIRE(in_contents == out_contents);
    }
  }

  fs::remove_all(output);
}
