#pragma once
#include "defs.h"
#include "utils.h"
#include <cstddef>

namespace fqcomp28 {
enum class Command { COMPRESS, DECOMPRESS };

class Settings {
public:
private:
  Settings() = default;

public:
  static Settings *getInstance() {
    static Settings set;
    return &set;
  }

  static const Settings *getDefaultsInstance() {
    static const Settings defaults;
    return &defaults;
  }

  static void resetToDefaults() { *getInstance() = *getDefaultsInstance(); }

  struct { /* required for decompression */
  } storable;

  struct { /* not required for decompression */
    path_t mates1;
    path_t mates2;
    path_t archive;
    unsigned n_threads = 1;
    unsigned read_chunk_size_Mb = 32;
    unsigned sample_chunk_size_Mb = 1;
    bool verbose = false;
    Command cmd = Command::COMPRESS;
  } non_storable;

  [[nodiscard]] bool is_pe() const { return !non_storable.mates2.empty(); }

  [[nodiscard]] std::size_t reading_chunk_size() const {
    return mbToBytes(non_storable.read_chunk_size_Mb);
  }

  [[nodiscard]] std::size_t sample_chunk_size() const {
    return mbToBytes(non_storable.sample_chunk_size_Mb);
  }
};
} // namespace fqcomp28
