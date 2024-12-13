#pragma once
#include "defs.h"

namespace fqzcomp28 {
void loadFileContents(path_t const path, std::vector<char> &data);
std::vector<char> loadFileContents(path_t const path);
}; // namespace fqzcomp28
