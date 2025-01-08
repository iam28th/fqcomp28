#include "defs.h"
namespace fqzcomp28 {

void checkStreamState(std::ios &stream, path_t path) {
  if (!stream)
    throw std::system_error(errno, std::generic_category(), path);
}
} // namespace fqzcomp28
