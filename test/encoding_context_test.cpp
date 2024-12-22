#include "compressed_buffers.h"
#include "defs.h"
#include "encoding_context.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>

using namespace fqzcomp28;

namespace fqzcomp28 {
struct EncodingContextTester {
  static void encodeHeader() {
    const path_t input = "test/data/SRR065390_sub_1.fastq";
    const auto chunk = loadFastqFileContents(input);
    const DatasetMeta meta(chunk);

    EncodingContext ctx(&meta);
    ctx.startNewChunk();

    CompressedBuffersDst cbs_dst;
    ctx.prepareCompressedBuffers(chunk, cbs_dst);

    /* concatenated original headers */
    std::vector<char> original_headers;

    for (const auto &r : chunk.records) {
      original_headers.insert(original_headers.end(), r.header().begin(),
                              r.header().end());
      ctx.encodeHeader(r.header(), cbs_dst);
    }

    CompressedBuffersSrc cbs_src = convertToSrcBuffers(std::move(cbs_dst));
    ctx.startNewChunk();

    std::vector<char> decoded_headers(original_headers.size(), 0);
    char *dst = decoded_headers.data();
    for (std::size_t i = 0; i < chunk.records.size(); ++i)
      dst += ctx.decodeHeader(dst, cbs_src);

    CHECK(static_cast<std::size_t>(dst - decoded_headers.data()) ==
          original_headers.size());
    CHECK(original_headers == decoded_headers);
  }

  static void encodeChunk() {
    // TODO probably can repeat this code only once somehow
    const path_t input = "test/data/SRR065390_1_first5.fastq";
    const FastqChunk chunk_in = loadFastqFileContents(input);
    const DatasetMeta meta(chunk_in);

    EncodingContext ctx(&meta);
    CompressedBuffersDst cbs;

    ctx.encodeChunk(chunk_in, cbs);

    FastqChunk chunk_out;

    CompressedBuffersSrc src = convertToSrcBuffers(std::move(cbs));
    ctx.decodeChunk(chunk_out, src);

    // CHECK(chunk_out.raw_data == chunk_in.raw_data);
    CHECK(chunk_out.records.size() == chunk_in.records.size());
    for (std::size_t i = 0, E = chunk_in.records.size(); i < E; ++i) {
      const auto &rec_in = chunk_in.records[i], rec_out = chunk_out.records[i];
      CHECK(rec_in.header() == rec_out.header());
      CHECK(rec_in.seq() == rec_out.seq());
      CHECK(rec_in.qual() == rec_out.qual());
    }
  }
};

} // namespace fqzcomp28

TEST_CASE("EncodingContext::{en,de}codeHeader") {
  EncodingContextTester::encodeHeader();
}

TEST_CASE("EncodingContext::{en,de}codeChunk") {
  EncodingContextTester::encodeChunk();
}
