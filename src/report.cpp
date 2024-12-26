#include "report.h"
#include <cmath>
#include <iostream>
#include <string>

namespace fqzcomp28 {

namespace {
double calc_ratio(std::size_t original, std::size_t compressed) {
  return static_cast<double>(original) / static_cast<double>(compressed);
}

const std::string sections_separator = "*****************************\n";
const std::string field_sep = "\t";
constexpr int column_width = 20;
const int precision = 3;
static_assert(precision <= column_width);

void print_string(std::string s, std::ostream &os) { os << s << field_sep; }

void print_integer(std::size_t x, std::ostream &os) {
  print_string(std::to_string(x), os);
}

void print_ratio(std::size_t num, std::size_t denom, std::ostream &os) {
  const double rat = calc_ratio(num, denom);
  std::string s(column_width, 0);
  const std::string fmt = "%." + std::to_string(precision) + "f";
  const auto n = std::sprintf(s.data(), fmt.c_str(), rat);
  s.resize(static_cast<std::size_t>(n));
  print_string(s, os);
};

} // namespace

void printReport(const InputStats &inp, const CompressedStats &comp,
                 const DatasetMeta &meta, std::ostream &os) {

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
  const std::size_t csize_total =
      comp.total(meta.n_fields_of_type(headers::FieldType::STRING));
  auto print_cstream = [&os, csize_total](std::string name, std::size_t bytes) {
    print_string(name, os);
    print_integer(bytes, os);
    print_ratio(bytes, csize_total, os);
    os << '\n';
  };

  const std::vector<std::pair<std::string, std::size_t>> cstreams = {
      {"seq", comp.seq},         {"readlens", comp.readlens},
      {"n_count", comp.n_count}, {"n_pos", comp.n_pos},
      {"qual", comp.qual},
  };
  for (auto [name, csize] : cstreams)
    print_cstream(name, csize);

  for (std::size_t i = 0, E = comp.header_fields.size(); i < E; ++i) {
    std::string name = "header_field_" + std::to_string(i + 1);
    print_cstream(name, comp.header_fields[i]);
  }
  print_cstream("meta", meta.size());

  os << sections_separator;
  os << "CR" << '\n';
  auto print_cr = [&os](std::string name, std::size_t ori_size,
                        std::size_t csize) {
    print_string(name, os);
    print_ratio(ori_size, csize, os);
    os << '\n';
  };
  print_cr("Sequence", inp.seq, comp.sequence());
  print_cr("Quality", inp.seq, comp.quality());
  print_cr("Headers", inp.header, comp.headers());
  print_cr("Total", inp.total(), csize_total + meta.size());

  os << sections_separator;
  print_string("# blocks: ", os);
  print_integer(comp.n_blocks, os);
  os << '\n';
}
}; // namespace fqzcomp28
