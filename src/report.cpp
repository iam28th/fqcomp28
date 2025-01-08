#include "report.h"
#include "settings.h"
#include <cmath>
#include <iostream>
#include <string>

namespace fqcomp28 {

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
                 const Archive &archive, std::ostream &os) {
  const auto &meta = archive.meta();

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
  const std::size_t archive_size = archive.indexBytes() +
                                   comp.data_section_size(meta.n_fields_of_type(
                                       headers::FieldType::STRING)) +
                                   meta.size();
  auto print_cstream = [&os, archive_size](std::string name,
                                           std::size_t bytes) {
    print_string(name, os);
    print_integer(bytes, os);
    print_ratio(bytes, archive_size, os);
    os << '\n';
  };

  const std::vector<std::pair<std::string, std::size_t>> cstreams = {
      {"seq", comp.seq},         {"readlens", comp.readlens},
      {"n_count", comp.n_count}, {"n_pos", comp.n_pos},
      {"qual", comp.qual},
  };
  for (const auto &[name, csize] : cstreams)
    print_cstream(name, csize);

  for (std::size_t i = 0, E = comp.header_fields.size(); i < E; ++i) {
    const std::string name = "header_field_" + std::to_string(i + 1);
    print_cstream(name, comp.header_fields[i]);
  }
  print_cstream("meta_seq", meta.sequence());
  print_cstream("meta_qual", meta.quality());

  os << sections_separator;
  os << "CR" << '\n';
  auto print_cr = [&os](std::string name, std::size_t ori_size,
                        std::size_t csize) {
    print_string(name, os);
    print_ratio(ori_size, csize, os);
    os << '\n';
  };
  print_cr("Sequence", inp.seq, comp.sequence() + meta.sequence());
  print_cr("Quality", inp.seq, comp.quality() + meta.quality());
  print_cr("Headers", inp.header, comp.headers() + meta.headers());
  print_cr("Total", inp.total(), archive_size);

  /* check that CR is calculated correctly */
  [[maybe_unused]] const auto *set = Settings::getInstance();
  assert(archive_size == std::filesystem::file_size(set->non_storable.archive));

  os << sections_separator;
  print_string("# blocks: ", os);
  print_integer(comp.n_blocks, os);
  os << '\n';
}

std::size_t
CompressedStats::data_section_size(const long n_string_fields) const {
  std::size_t ret = sequence() + quality() + headers();

  std::size_t block_meta = 0;
  block_meta += sizeof(uint32_t);     // total size
  block_meta += sizeof(uint32_t);     // n_records
  block_meta += 2 * sizeof(uint32_t); // readlens (original & compressed)
  block_meta += 2 * sizeof(uint32_t); // npos
  block_meta += 2 * sizeof(uint32_t); // ncount
  block_meta +=
      2 * sizeof(uint32_t); // compressed sizes for sequence and quality

  const auto n_string = static_cast<uint64_t>(n_string_fields);
  const uint64_t n_numeric = header_fields.size() - n_string;
  block_meta += n_string * 3 * 2 * sizeof(uint32_t);
  block_meta += n_numeric * 2 * sizeof(uint32_t);

  ret += n_blocks * block_meta;
  return ret;
}
}; // namespace fqcomp28
