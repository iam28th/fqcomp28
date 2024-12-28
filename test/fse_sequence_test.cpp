#include "defs.h"
#include "encoding_context.h"
#include "entropy_sequence.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <ranges>

using namespace fqzcomp28;

std::vector<char> getConcatenatedSequences(const FastqChunk &chunk) {
  std::vector<char> ret;
  for (const auto &r : chunk.records)
    ret.insert(ret.end(), r.seqp, r.seqp + r.length);
  return ret;
}

TEST_CASE("Sequence encoding (without Ns)") {
  const path_t inp_path = "test/data/without_ns.fastq";
  FastqChunk chunk = loadFastqFileContents(inp_path);
  FSE_Sequence::FreqTable ft = FSE_Sequence::calculateFreqTable(chunk);

  const std::vector<char> sequences = getConcatenatedSequences(chunk);

  const std::size_t input_size = chunk.tot_reads_length;
  assert(input_size == sequences.size());

  std::vector<std::byte> output_buf;
  output_buf.resize(EncodingContext::compressBoundSequence(input_size));

  SequenceEncoder encoder(&ft);
  SequenceDecoder decoder(&ft);
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

  const std::vector<char> decoded_sequences = getConcatenatedSequences(chunk);

  CHECK(sequences == decoded_sequences);
}
