#pragma once
#include <vector>

// assert needs to be defined before including fse headrs,
// otherwise zstd defines it to noop ...
#include <cassert>

/* headers from zstd library */
extern "C" {
#define FSE_STATIC_LINKING_ONLY
#include <common/bitstream.h>
#include <common/fse.h>
#include <compress/hist.h>
}

namespace fqzcomp28 {

inline std::vector<FSE_FUNCTION_TYPE>
createCTableBuildWksp(const unsigned maxSymbolValue, const unsigned tableLog) {
  return std::vector<FSE_FUNCTION_TYPE>(
      FSE_BUILD_CTABLE_WORKSPACE_SIZE(maxSymbolValue, tableLog));
}

} // namespace fqzcomp28
