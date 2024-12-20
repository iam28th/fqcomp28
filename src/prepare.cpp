#include "prepare.h"
#include "fastq_io.h"

namespace fqzcomp28 {

DatasetMeta::DatasetMeta(const FastqChunk &chunk)
    : first_header(chunk.records.front().header()), header_fmt(first_header) {}

DatasetMeta analyzeDataset(path_t fastq_file, std::size_t sample_size_bytes) {
  FastqChunk chunk;
  FastqReader reader(fastq_file, sample_size_bytes);
  reader.readNextChunk(chunk);
  return DatasetMeta(chunk);
}

} // namespace fqzcomp28
