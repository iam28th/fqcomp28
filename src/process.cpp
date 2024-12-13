#include "app.h"
#include "fse_compressor.h"
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
  std::cout << "hehe I'm compressing" << std::endl;

  FSE_CTable *ct = FSE_createCTable('T', 5);
  auto tableSymbol = createCTableBuildWksp('T', 5);
}

void processArchiveParts() {
  std::cout << "hehe I'm decompressing" << std::endl;
}
} // namespace fqzcomp28
