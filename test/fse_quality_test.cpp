#include "defs.h"
#include "fse_quality.h"
#include "test_utils.h"
#include "workspace.h"
#include <catch2/catch_test_macros.hpp>
#include <ranges>

using namespace fqzcomp28;

std::vector<char> getConcatenatedQualities(const FastqChunk &chunk) {
  std::vector<char> ret;
  for (const auto &r : chunk.records)
    ret.insert(ret.end(), r.qualp, r.qualp + r.length);
  return ret;
}

TEST_CASE("FSE Quality") {
  const path_t inp_path = "test/data/without_ns.fastq";

  FastqChunk chunk = loadFastqFileContents(inp_path);
  FSE_Quality::FreqTable ft = FSE_Quality::calculateFreqTable(chunk);

  const std::vector<char> qualities = getConcatenatedQualities(chunk);
  const std::size_t input_size = chunk.tot_reads_length;
  assert(input_size == qualities.size());

  std::vector<std::byte> output_buf;
  output_buf.resize(Workspace::compressBoundQuality(input_size));

  QualityEncoder encoder(&ft);
  QualityDecoder decoder(&ft);

  encoder.startChunk(output_buf);
  for (const auto &r : chunk.records)
    encoder.encodeRecord(r);
  const std::size_t compressed_size = encoder.endChunk();
  assert(compressed_size != 0); // 0 means didn't fit into dst
  output_buf.resize(compressed_size);

  decoder.startChunk(output_buf);
  for (auto &r : chunk.records | std::views::reverse)
    decoder.decodeRecord(r); // decoder modifies memory r points to
  decoder.endChunk();

  const std::vector<char> decoded_qualities = getConcatenatedQualities(chunk);

  CHECK(qualities.size() == decoded_qualities.size());
  CHECK(qualities == decoded_qualities);
}
