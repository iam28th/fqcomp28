#include "prepare.h"
#include "fastq_io.h"

namespace fqzcomp28 {

bool operator==(const DatasetMeta &lhs, const DatasetMeta &rhs) {
  return lhs.first_header == rhs.first_header && lhs.ft_seq == rhs.ft_seq &&
         lhs.ft_qual == rhs.ft_qual;
}

void DatasetMeta::storeToStream(const DatasetMeta &meta, std::ostream &os) {
  // so far, only write the header... later will add frequency tables from fse
  const auto hlen = static_cast<readlen_t>(meta.first_header.size());
  os.write(reinterpret_cast<const char *>(&hlen), sizeof(hlen));
  os.write(meta.first_header.data(), hlen);

  // TODO: use FSE_writeNCount to store tables compactly
  os.write(reinterpret_cast<const char *>(&(meta.ft_seq)), sizeof(ft_seq));
  os.write(reinterpret_cast<const char *>(&(meta.ft_qual)), sizeof(ft_qual));
}

DatasetMeta DatasetMeta::loadFromStream(std::istream &is) {
  readlen_t hlen;
  is.read(reinterpret_cast<char *>(&hlen), sizeof(hlen));
  std::string first_header(hlen, '!');
  is.read(first_header.data(), hlen);
  assert(is.good());

  DatasetMeta meta(first_header);

  is.read(reinterpret_cast<char *>(&(meta.ft_seq)), sizeof(ft_seq));
  assert(is.good());
  is.read(reinterpret_cast<char *>(&(meta.ft_qual)), sizeof(ft_qual));
  assert(is.good());
  return meta;
}

DatasetMeta analyzeDataset(path_t fastq_file, std::size_t sample_size_bytes) {
  FastqChunk chunk;
  FastqReader reader(fastq_file, sample_size_bytes);
  reader.readNextChunk(chunk);
  return DatasetMeta(chunk);
}

} // namespace fqzcomp28
