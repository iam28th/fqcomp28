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

    CompressedBuffers cbs;
    cbs.header_fields_in.resize(meta.header_fmt.n_fields());

    /* concatenated original headers */
    std::vector<char> original_headers;

    for (const auto &r : chunk.records) {
      original_headers.insert(original_headers.end(), r.header().begin(),
                              r.header().end());
      ctx.encodeHeader(r.header(), cbs);
    }

    for (auto &storage : cbs.header_fields_in)
      cbs.header_fields_out.push_back(convertToOutStorage(std::move(storage)));
    ctx.startNewChunk();

    std::vector<char> decoded_headers(original_headers.size(), 0);
    char *dst = decoded_headers.data();
    for (std::size_t i = 0; i < chunk.records.size(); ++i)
      dst += ctx.decodeHeader(dst, cbs);

    CHECK(static_cast<std::size_t>(dst - decoded_headers.data()) ==
          original_headers.size());
    CHECK(original_headers == decoded_headers);
  }
};
} // namespace fqzcomp28

TEST_CASE("EncodingContext::{en,de}codeHeader") {
  EncodingContextTester::encodeHeader();
}
