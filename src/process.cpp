#include "app.h"
#include "compressed_buffers.h"
#include "defs.h"
#include "encoding_context.h"
#include "fastq_io.h"
#include "fse_compressor.h"
#include "prepare.h"
#include "settings.h"
#include <iostream>
#include <vector>

/* headers from zstd library */
#define FSE_STATIC_LINKING_ONLY
#include <common/fse.h>

std::vector<FSE_FUNCTION_TYPE>
createCTableBuildWksp(const unsigned maxSymbolValue, const unsigned tableLog) {
  return std::vector<FSE_FUNCTION_TYPE>(
      FSE_BUILD_CTABLE_WORKSPACE_SIZE(maxSymbolValue, tableLog));
}

/**
 * fqzcomp28 entry point
 */
int startProgram(int argc, char **argv) {
  CLI::App app;
  addOptions(&app);

  try {
    app.parse(argc, argv);
  } catch (const CLI::CallForHelp &e) {
    return app.exit(e);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  return 0;
}

namespace fqzcomp28 {
void processReads() {
  // parse the first header

  const auto set = Settings::getInstance();
  const auto mates1 = set->non_storable.mates1;

  const DatasetMeta meta = analyzeDataset(mates1, set->sample_chunk_size());

  FastqChunk chunk;
  FastqReader reader(mates1, set->reading_chunk_size());
  CompressedBuffers cbs;
  EncodingContext ctx(&meta);

  while (reader.readNextChunk(chunk)) {
    ctx.encodeChunk(chunk, cbs);
    chunk.clear();
  }

#if 0
  FSE_CTable *ct = FSE_createCTable('T', 5);
  auto tableSymbol = createCTableBuildWksp('T', 5);
#endif
}

void processArchiveParts() {
  std::cout << "hehe I'm decompressing" << std::endl;
}
} // namespace fqzcomp28
