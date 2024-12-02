#include <seqan3/core/debug_stream.hpp>
#include <seqan3/io/sequence_file/input.hpp>

int main(int argc, char **argv) {

  std::string fname = argv[1];
  seqan3::sequence_file_input file_in{fname};

  // Retrieve the sequences and ids.
  for (auto &[seq, id, qual] : file_in) {
    // works with both fastq and fasta
    seqan3::debug_stream << "ID:\t" << id << '\n';
    seqan3::debug_stream << "SEQ:\t" << seq << '\n';
    seqan3::debug_stream << "Qual.\t" << qual
                         << '\n'; // qual is empty for FASTA files
  }

  std::size_t tot_name_len = 0;
  std::cout << tot_name_len << '\n';
  return 0;
}
