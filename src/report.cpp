#include "report.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

namespace fqzcomp28 {

namespace {
double calc_ratio(std::size_t original, std::size_t compressed) {
  return static_cast<double>(original) / static_cast<double>(compressed);
}

double bytes2gb(std::size_t bytes) {
  return calc_ratio(bytes, 1024 * 1024 * 1024);
}

const std::string sections_separator = "------------\n";
const std::string field_sep = "\t";
constexpr int column_width = 20;
const int precision = 3;
static_assert(precision <= column_width);

void print_string(std::string s, std::ostream &os) {
  os << std::setw(column_width) << s << field_sep;
}

void print_integer(std::size_t x, std::ostream &os,
                   std::string suf = " bytes") {
  print_string(std::to_string(x) + suf, os);
}

void print_ratio(std::size_t num, std::size_t denom, std::ostream &os) {
  const double rat = calc_ratio(num, denom);
  std::string s(column_width, 0);
  const std::string fmt = "%." + std::to_string(precision) + "f";
  auto n = std::sprintf(s.data(), fmt.c_str(), rat);
  s.resize(static_cast<std::size_t>(n));
  print_string(s, os);
};

} // namespace

void printReport(const InputStats &inp, const CompressedSizes &comp,
                 std::ostream &os, const int precision) {

  os << std::left;
  os << sections_separator;
  os << "Input sizes:\n";
  auto print_inp = [&os](std::string name, std::size_t bytes) {
    print_string(name, os);
    print_integer(bytes, os);
    os << '\n';
  };
  print_inp("Sequence", inp.seq);
  print_inp("Quality", inp.seq);
  print_inp("Headers", inp.header);

  os << sections_separator;
  os << "Compressed stream sizes:\n";
  const std::size_t csize_total = comp.total();
  auto print_cstream = [&os, precision, csize_total](std::string name,
                                                     std::size_t bytes) {
    print_string(name, os);
    print_integer(bytes, os);
    print_ratio(bytes, csize_total, os);
    os << '\n';
  };
  print_cstream("seq", comp.seq);
  // os << "Compressed

  // TODO
  //
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
