#include "archive.h"
#include <catch2/catch_test_macros.hpp>

namespace fqzcomp28 {

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
};
} // namespace fqzcomp28

using namespace fqzcomp28;
TEST_CASE("Archive creation") { ArchiveTester::createArchive(); }
