#pragma once
#include "defs.h"
#include <fstream>

namespace fqzcomp28 {

class FastqReader {
public:
  FastqReader(std::string mates1, std::size_t reading_size);
  FastqReader(std::string mates1, std::string mates2, std::size_t reading_size);

  /**
   * @return true if reading was succesfull
   */
  bool readNextChunk(FastqChunk &chunk);

private:
  /**
   * sets pointers in chunk.reads;
   * @return position at which a partially loaded read starts
   */
  static std::size_t parseRecords(FastqChunk &chunk);

  /**
   * chunk size in bytes (approximate, in case of partial read)
   */
  const std::size_t reading_size_;

  FastqData partial1_, partial2_;

  std::ifstream ifs1_, ifs2_;
  std::size_t bytes_left1_, bytes_left2_;
};

class FastqWriter {
public:
  FastqWriter(std::string mates1);
  void writeChunk(FastqChunk const &chunk);

  void flush() { ofs1_.flush(); }

private:
  std::ofstream ofs1_;
};

} // namespace fqzcomp28