#include "defs.h"
#include "fse_sequence.h"
#include "test_utils.h"
#include "workspace.h"
#include <catch2/catch_test_macros.hpp>
#include <ranges>

using namespace fqcomp28;

std::vector<char> getConcatenatedSequences(const FastqChunk &chunk) {
  std::vector<char> ret;
  for (const auto &r : chunk.records)
    ret.insert(ret.end(), r.seqp, r.seqp + r.length);
  return ret;
}

TEST_CASE("FSE Sequence") {
  const path_t inp_path = "test/data/SRR065390_sub_1.fastq";

  FastqChunk chunk = loadFastqFileContents(inp_path);
  const auto ft = FSE_Sequence::calculateFreqTable(chunk);

  const std::vector<char> sequences = getConcatenatedSequences(chunk);
  const std::size_t input_size = chunk.tot_reads_length;
  assert(input_size == sequences.size());

  std::vector<std::byte> output_buf;
  output_buf.resize(Workspace::compressBoundSequence(input_size));

  SequenceEncoder encoder(ft.get());
  SequenceDecoder decoder(ft.get());
  CompressedBuffersDst cbs_dst;

  encoder.startChunk(output_buf);
  for (auto &r : chunk.records)
    encoder.encodeRecord(r, cbs_dst);
  const std::size_t compressed_size = encoder.endChunk();
  assert(compressed_size != 0); // 0 means didn't fit into dst
  output_buf.resize(compressed_size);

  CompressedBuffersSrc cbs_src = convertToSrcBuffers(std::move(cbs_dst));
  decoder.startChunk(output_buf);
  for (auto &r : chunk.records | std::views::reverse)
    decoder.decodeRecord(r, cbs_src); // decoder modifies memory r points to
  const std::vector<char> decoded_sequences = getConcatenatedSequences(chunk);
  CHECK(sequences == decoded_sequences);
  decoder.endChunk();

  CHECK(sequences == decoded_sequences);
}
