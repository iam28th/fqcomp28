#include "report.h"
#include <iostream>
#include <string>

namespace fqzcomp28 {

namespace {
double calc_ratio(std::size_t original, std::size_t compressed) {
  return static_cast<double>(original) / static_cast<double>(compressed);
}
} // namespace

void printReport(const InputStats &inp, const CompressedSizes &comp,
                 std::ostream &os) {
  const std::string sections_separator = "------------\n";

  // TODO
  // input sizes
  // ----
  // buffer archive fractions
  // seq
  // readlen
  // n_count
  // n_pos
  // qual
  // hf1
  // hf2
  // ...
  // table compression_ratio archive_fraction
  // seq 20x 0.5
  // qual ...
  // headers ...
  // ---
  // total CR:
  // size of compressed file:
}
}; // namespace fqzcomp28
