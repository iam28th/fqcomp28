#include <filesystem>

class Settings {
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
    using path = std::filesystem::path;
    path mates1;
    path mates2;
    path archive;
    int n_threads = 1;
    bool verbose = false;
  } non_storable;

  bool is_pe() const { return !non_storable.mates2.empty(); }
};
