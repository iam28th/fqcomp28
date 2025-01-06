#include "fastq_io.h"
#include "utils.h"
#include <cassert>
#include <cstring>
#include <fstream>

namespace fqzcomp28 {
FastqReader::FastqReader(std::string mates1, std::size_t reading_size)
    : reading_size_(reading_size), ifs1_(mates1),
      bytes_left1_(std::filesystem::file_size(mates1)), bytes_left2_(0) {};

FastqReader::FastqReader(std::string mates1, std::string mates2,
                         std::size_t reading_size)
    : FastqReader(mates1, reading_size) {
  ifs2_.open(mates2);
  bytes_left2_ = std::filesystem::file_size(mates2);
}

bool FastqReader::readNextChunk(FastqChunk &chunk) {
  /* safe to call before lock,
   * becase each thread operates on each own's objects */
  chunk.clear();
  chunk.idx = chunks_read_++;

  const std::lock_guard guard(mtx_);

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

  ifs1_.read(buf, narrow_cast<std::streamsize>(to_read));
  assert(ifs1_);

  /* parse data to find last fully loaded record */
  // TODO: only find the end of the last record,
  // and parse the rest of the chunk in parallel
  const std::size_t actual_chunk_size = parseRecords(chunk);
  assert(actual_chunk_size <= reading_size_);

  /* store partial record */
  partial1_.resize(chunk.raw_data.size() - actual_chunk_size);
  std::memcpy(partial1_.data(), chunk.raw_data.data() + actual_chunk_size,
              partial1_.size());

  chunk.raw_data.resize(actual_chunk_size);
  assert(chunk.raw_data.back() == '\n');

  bytes_left1_ -= to_read;
  return true;
}

std::size_t FastqReader::parseRecords(FastqChunk &chunk) {
  assert(chunk.records.empty());

  const std::size_t size = chunk.raw_data.size();
  std::size_t bytes_processed = 0;

  FastqRecord rec;
  int ln = 0;
  char *line_start = chunk.raw_data.data();
  assert(*line_start == '@'); /* as it should be fastq header */

  for (;;) {

    if (bytes_processed == size) {
      const char *beg = chunk.raw_data.data();
      return (ln == 0 ? chunk.raw_data.size()
                      : narrow_cast<std::size_t>(rec.headerp - beg));
    }

    char *const line_end = static_cast<char *>(
        std::memchr(line_start, '\n', size - bytes_processed));

    if (line_end == nullptr) {
      const char *beg = chunk.raw_data.data();
      return narrow_cast<std::size_t>(ln == 0 ? line_start - beg
                                              : rec.headerp - beg);
    }

    const auto line_length = narrow_cast<readlen_t>(line_end - line_start);
    assert(line_length <= std::numeric_limits<readlen_t>::max());

    switch (ln) {
    case 0:
      rec.headerp = line_start;
      rec.header_length = line_length;
      chunk.headers_length += line_length;
      break;
    case 1:
      rec.seqp = line_start;
      rec.length = line_length;
      chunk.tot_reads_length += line_length;
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
  std::unique_lock guard(mtx_);

  while (chunk.idx != chunks_written_)
    cv_.wait(guard);

  ofs1_.write(chunk.raw_data.data(),
              narrow_cast<std::streamsize>(chunk.raw_data.size()));

  guard.unlock();
  chunks_written_++;
  cv_.notify_all();
}

} // namespace fqzcomp28
