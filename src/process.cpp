#include "app.h"
#include <iostream>
#include <seqan3/core/debug_stream.hpp>
#include <seqan3/io/sequence_file/input.hpp>

/**
 * fqzcomp28 entry point
 */
int startProgram(int argc, char **argv) {
  CLI::App app;
  addOptions(&app);

  try {
    app.parse(argc, argv);
  } catch (const CLI::CallForHelp &e) {
    return app.exit(e);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

#if 0
  std::string fname = argv[1];
  seqan3::sequence_file_input file_in{fname};

  // Retrieve the sequences and ids.
  for (const auto &[seq, id, qual] : file_in) {
    // works with both fastq and fasta
    seqan3::debug_stream << "ID:\t" << id << '\n';
    seqan3::debug_stream << "SEQ:\t" << seq << '\n';
    seqan3::debug_stream << "Qual.\t" << qual
                         << '\n'; // qual is empty for FASTA files
  }

  std::size_t tot_name_len = 0;
  std::cout << tot_name_len << '\n';
#endif
  return 0;
}

namespace fqzcomp28 {
void processReads() { std::cout << "hehe I'm compressing" << std::endl; }

void processArchiveParts() {
  std::cout << "hehe I'm decompressing" << std::endl;
}
} // namespace fqzcomp28
