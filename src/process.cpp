#include "app.h"
#include "archive.h"
#include "compressed_buffers.h"
#include "defs.h"
#include "encoding_context.h"
#include "fastq_io.h"
#include "fse_compressor.h"
#include "settings.h"
#include <vector>

/* headers from zstd library */
#define FSE_STATIC_LINKING_ONLY
#include <common/fse.h>

namespace fqzcomp28 {

std::vector<FSE_FUNCTION_TYPE>
createCTableBuildWksp(const unsigned maxSymbolValue, const unsigned tableLog) {
  return std::vector<FSE_FUNCTION_TYPE>(
      FSE_BUILD_CTABLE_WORKSPACE_SIZE(maxSymbolValue, tableLog));
}

/** fqzcomp28 entry point */
int startProgram(int argc, char **argv) {
  CLI::App app;
  fqzcomp28::addOptions(&app);

  try {
    app.parse(argc, argv);
  } catch (const CLI::CallForHelp &e) {
    return app.exit(e);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  return 0;
}

void processReads() {

  const auto set = Settings::getInstance();
  const auto mates1 = set->non_storable.mates1;

  Archive archive(set->non_storable.archive, mates1);

  FastqChunk chunk;
  FastqReader reader(mates1, set->reading_chunk_size());

  InputStats istats;

  CompressedBuffersDst cbs;
  EncodingContext ctx(&archive.meta());

  [[maybe_unused]] unsigned n_blocks = 0;
  while (reader.readNextChunk(chunk)) {
    ++n_blocks;

    istats.seq = chunk.tot_reads_length;
    istats.header = chunk.headers_length;
    istats.n_records += chunk.records.size();

    ctx.encodeChunk(chunk, cbs);

    archive.writeBlock(cbs);
  }

#if 0
  FSE_CTable *ct = FSE_createCTable('T', 5);
  auto tableSymbol = createCTableBuildWksp('T', 5);
#endif
}

void processArchiveParts() {
  const auto set = Settings::getInstance();
  Archive archive(set->non_storable.archive);
  FastqWriter writer(set->non_storable.mates1);

  CompressedBuffersSrc cbs;
  FastqChunk chunk;
  EncodingContext ctx(&archive.meta());

  while (archive.readBlock(cbs)) {
    ctx.decodeChunk(chunk, cbs);
    writer.writeChunk(chunk);
  }
}
} // namespace fqzcomp28
