#include "fastq_io.h"
#include <cassert>
#include <cstring>
#include <fstream>

namespace fqzcomp28 {
FastqReader::FastqReader(std::string mates1, std::size_t reading_size)
    : reading_size_(reading_size), ifs1_(mates1),
      bytes_left1_(std::filesystem::file_size(mates1)) {};

FastqReader::FastqReader(std::string mates1, std::string mates2,
                         std::size_t reading_size)
    : FastqReader(mates1, reading_size) {
  ifs2_.open(mates2);
  bytes_left2_ = std::filesystem::file_size(mates2);
}

bool FastqReader::readNextChunk(FastqChunk &chunk) {
  if (bytes_left1_ == 0)
    return false;

  const std::size_t to_read =
      std::min(reading_size_ - partial1_.size(), bytes_left1_);

  chunk.raw_data.resize(partial1_.size() + to_read);

  char *buf = chunk.raw_data.data();

  /* copy partial data from previous call */
  std::memcpy(buf, partial1_.data(), partial1_.size());
  buf += partial1_.size();
  partial1_.clear();

  ifs1_.read(buf, static_cast<std::streamsize>(to_read));

  /* parse data to find last fully loaded record */
  const std::size_t actual_chunk_size = parseRecords(chunk);
  assert(actual_chunk_size <= reading_size_);

  /* store partial record */
  partial1_.resize(reading_size_ - actual_chunk_size);
  std::memcpy(partial1_.data(), chunk.raw_data.data() + actual_chunk_size,
              partial1_.size());

  chunk.raw_data.resize(actual_chunk_size);
  assert(chunk.raw_data.back() == '\n');

  bytes_left1_ -= to_read;
  return true;
}

std::size_t FastqReader::parseRecords(FastqChunk &chunk) {
  assert(chunk.records.size() == 0);

  const std::size_t size = chunk.raw_data.size();
  std::size_t bytes_processed = 0;

  FastqRecord rec;
  int ln = 0;
  const char *line_start = chunk.raw_data.data();
  assert(*line_start == '@'); /* as it should be fastq header */

  for (;;) {

    if (bytes_processed == size) {
      const char *beg = chunk.raw_data.data();
      return (ln == 0 ? chunk.raw_data.size()
                      : static_cast<std::size_t>(rec.headerp - beg));
    }

    const char *line_end = static_cast<const char *>(
        std::memchr(line_start, '\n', size - bytes_processed));

    if (line_end == nullptr) {
      const char *beg = chunk.raw_data.data();
      return static_cast<std::size_t>(ln == 0 ? line_start - beg
                                              : rec.headerp - beg);
    }

    const auto line_length = static_cast<unsigned>(line_end - line_start);
    // TODO: once I use delta to store readlens, will have to
    // compare with (max >> 1u)
    assert(line_length <= std::numeric_limits<readlen_t>::max());

    switch (ln) {
    case 0:
      rec.headerp = line_start;
      rec.header_length = static_cast<readlen_t>(line_length);
      chunk.tot_reads_length += line_length;
      break;
    case 1:
      rec.seqp = line_start;
      rec.length = static_cast<readlen_t>(line_length);
      break;
    case 2: /* skip fastq quality header */
      assert(*line_start == '+');
      break;
    case 3:
      assert(line_length == rec.length);
      rec.qualp = line_start;
      chunk.records.push_back(rec);
      ln = -1; /* so that it's 0 after increment... */
      break;
    default:
      __builtin_unreachable();
    }
    ++ln;
    line_start = line_end + 1;
    bytes_processed += line_length + 1;
  }
}

FastqWriter::FastqWriter(std::string mates1) : ofs1_(mates1) {}

void FastqWriter::writeChunk(FastqChunk const &chunk) {
  ofs1_.write(chunk.raw_data.data(),
              static_cast<std::streamsize>(chunk.raw_data.size()));
}

} // namespace fqzcomp28
