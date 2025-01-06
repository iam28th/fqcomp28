#pragma once
#include "defs.h"
#include <condition_variable>
#include <fstream>
#include <mutex>

namespace fqzcomp28 {

/** Reads data from a single .fastq file */
class FastqReader {
public:
  FastqReader(std::string mates1, std::size_t reading_size);
  /** This ctor is more for the future... */
  FastqReader(std::string mates1, std::string mates2, std::size_t reading_size);

  /** @return true if reading was succesfull */
  bool readNextChunk(FastqChunk &chunk);

  /**
   * sets pointers in chunk.reads;
   *
   * @return position at which a partially loaded read starts
   */
  static std::size_t parseRecords(FastqChunk &chunk);

private:
  /** chunk size in bytes (approximate, in case of partial read) */
  const std::size_t reading_size_;

  FastqData partial1_, partial2_;

  std::ifstream ifs1_, ifs2_;
  std::size_t bytes_left1_, bytes_left2_;
  /** tracks how many chunks were read already */
  unsigned chunks_read_ = 0;

  std::mutex mtx_;
};

/** Writes data to a single .fastq file */
class FastqWriter {
public:
  explicit FastqWriter(std::string mates1);
  void writeChunk(FastqChunk const &chunk);

  void flush() { ofs1_.flush(); }

private:
  std::ofstream ofs1_;
  /** Used to compare against chunk index, to write chunks in original order */
  unsigned chunks_written_ = 0;
  std::mutex mtx_;
  /** To wait until preceeding chunks have been written */
  std::condition_variable cv_;
};

} // namespace fqzcomp28
