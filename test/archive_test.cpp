#include "archive.h"
#include "fastq_io.h"
#include "workspace.h"
#include <catch2/catch_test_macros.hpp>

namespace fqcomp28 {

bool compressedPartEqual(const CompressedBuffersDst &dst,
                         const CompressedBuffersSrc &src) {
  return dst.seq == src.seq && dst.qual == src.qual &&
         dst.compressed_readlens == src.compressed_readlens &&
         dst.compressed_header_fields == src.compressed_header_fields;
}

struct ArchiveTester {
  /**
   * Compress a file; create decompression archive;
   * check that index and meta sections match between compression
   * and decompression archives
   * (after sorting index in the compression archive)
   */
  static void header() {
    const auto *set = Settings::getInstance();
    const path_t input = "test/data/SRR065390_sub_1.fastq";
    const path_t archive_path = "test_archive_header.fqz";

    Archive compr_archive(archive_path, input);
    FastqReader reader(input, set->reading_chunk_size());
    FastqChunk chunk;

    CompressedBuffersDst cbs;
    CompressionWorkspace wksp(&compr_archive.meta());

    while (reader.readNextChunk(chunk)) {
      wksp.encodeChunk(chunk, cbs);
      compr_archive.writeBlock(cbs);
    }

    compr_archive.writeIndex();
    compr_archive.flush();

    const Archive decompr_archive(archive_path);
    CHECK(compr_archive.meta() == decompr_archive.meta());

    compr_archive.sortIndex();
    CHECK(compr_archive.index_ == decompr_archive.index_);

    std::filesystem::remove_all(archive_path);
  }
};
} // namespace fqcomp28

using namespace fqcomp28;
TEST_CASE("Archive header") { ArchiveTester::header(); }
