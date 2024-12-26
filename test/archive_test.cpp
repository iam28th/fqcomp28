#include "archive.h"
#include "encoding_context.h"
#include "fastq_io.h"
#include <catch2/catch_test_macros.hpp>

namespace fqzcomp28 {

bool compressedPartEqual(const CompressedBuffersDst &dst,
                         const CompressedBuffersSrc &src) {
  return dst.seq == src.seq && dst.qual == src.qual &&
         dst.compressed_readlens == src.compressed_readlens &&
         dst.compressed_header_fields == src.compressed_header_fields;
}
struct ArchiveTester {
  static void createArchive() {
    const path_t input = "test/data/SRR065390_sub_1.fastq";
    const path_t archive_path = "archive.f2q8z";

    SECTION("checks") {
      auto compr_archive = Archive(archive_path, input);
      /* header should have been written after flush */
      compr_archive.flush();

      auto decompr_archive = Archive(archive_path);
      CHECK(compr_archive.meta() == decompr_archive.meta());
    }

    std::filesystem::remove_all(archive_path);
  }

  /**
   * create archive, write block of data, read it; and check that compressed
   * contents are the same
   * TODO move to "test_process" since it's (or will be) kind of entire program
   * run ?
   */
  static void writeBlock() {
    const path_t input = "test/data/SRR065390_sub_1.fastq";
    const path_t archive_path = "archive.f2q8z";
    const std::size_t chunk_size = 4 * 1024; // 4Kb

    SECTION("checks") {
      Archive archive_out(archive_path, input);
      archive_out.flush(); // to write meta
      Archive archive_in(archive_path);

      FastqChunk chunk_in, chunk_out;
      FastqReader reader(input, chunk_size);

      CompressedBuffersDst cbs_dst;
      CompressedBuffersSrc cbs_src;

      EncodingContext ctx(&archive_out.meta());

      [[maybe_unused]] unsigned n_chunks = 0;
      while (reader.readNextChunk(chunk_in)) {
        ++n_chunks;

        ctx.encodeChunk(chunk_in, cbs_dst);

        archive_out.writeBlock(cbs_dst);
        archive_out.flush();

        archive_in.readBlock(cbs_src);

        // TODO: another check after "decompressedMiscBuffers" ?
        CHECK(compressedPartEqual(cbs_dst, cbs_src));

        ctx.decodeChunk(chunk_out, cbs_src);
      }

      const bool ret = archive_in.readBlock(cbs_src);
      CHECK(!ret);
    }

    std::filesystem::remove_all(archive_path);
  }
};
} // namespace fqzcomp28

using namespace fqzcomp28;
TEST_CASE("Archive creation") { ArchiveTester::createArchive(); }

TEST_CASE("Archive::{read,write}Block") { ArchiveTester::writeBlock(); }
