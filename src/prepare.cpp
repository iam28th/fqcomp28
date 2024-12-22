#include "prepare.h"
#include "fastq_io.h"

namespace fqzcomp28 {

void DatasetMeta::storeToStream(const DatasetMeta &meta, std::ostream &os) {
  // so far, only write the header... later will add frequency tables from fse
  const auto hlen = static_cast<readlen_t>(meta.first_header.size());
  os.write(reinterpret_cast<const char *>(&hlen), sizeof(hlen));
  os.write(meta.first_header.data(), hlen);
}

DatasetMeta DatasetMeta::loadFromStream(std::istream &is) {
  readlen_t hlen;
  is.read(reinterpret_cast<char *>(&hlen), sizeof(hlen));
  std::string first_header(hlen, '!');
  is.read(first_header.data(), hlen);
  return DatasetMeta(first_header);
}

DatasetMeta analyzeDataset(path_t fastq_file, std::size_t sample_size_bytes) {
  FastqChunk chunk;
  FastqReader reader(fastq_file, sample_size_bytes);
  reader.readNextChunk(chunk);
  return DatasetMeta(chunk);
}

} // namespace fqzcomp28
