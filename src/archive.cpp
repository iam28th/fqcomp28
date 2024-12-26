#include "archive.h"

namespace fqzcomp28 {

/** Creates Archive to read compressed data from an existing fqzcomp28 file */
Archive::Archive(const path_t archive_path)
    : fs_(archive_path, std::ios_base::binary | std::ios_base::in) {
  readArchiveHeader();
}

Archive::Archive(const path_t archive_path, const path_t file_to_gather_meta,
                 const std::size_t sample_size_bytes)
    : fs_(archive_path, std::ios_base::binary | std::ios_base::out) {
  meta_ = analyzeDataset(file_to_gather_meta, sample_size_bytes);
  writeArchiveHeader();
}

void Archive::writeArchiveHeader() {
  // TODO: write parameters which are required for decompression
  // (once there're any ...)
  DatasetMeta::storeToStream(meta_, fs_);
}

void Archive::readArchiveHeader() { meta_ = DatasetMeta::loadFromStream(fs_); }

void Archive::writeBlock(const CompressedBuffersDst &cb) {
  /* these 2 number can be deduced,
   * but let's just store them to simplify decompression */
  writeInteger(cb.original_size.total);
  writeInteger(cb.original_size.n_records);

  /* for each buffer we store original size,
   * followed by compressed size, followed by compressed data */
  writeInteger(cb.original_size.readlens);
  writeBytes(cb.compressed_readlens);
  // TODO Ns

  writeBytes(cb.seq);
  writeBytes(cb.qual);

  assert(cb.header_fields.size() == cb.compressed_header_fields.size());
  for (std::size_t i = 0, E = meta_.header_fmt.n_fields(); i < E; ++i) {
    const auto &field_cdata = cb.compressed_header_fields[i];
    const auto &field_original_size = cb.original_size.header_fields[i];

    if (meta_.header_fmt.field_types[i] == headers::FieldType::STRING) {
      writeInteger(field_original_size.isDifferentFlag);
      writeBytes(field_cdata.isDifferentFlag);

      writeInteger(field_original_size.content);
      writeBytes(field_cdata.content);

      writeInteger(field_original_size.contentLength);
      writeBytes(field_cdata.contentLength);

    } else {
      writeInteger(field_original_size.content);
      writeBytes(field_cdata.content);
    }
  }
}

bool Archive::readBlock(CompressedBuffersSrc &cb) {
  cb.clear();

  cb.original_size.total = readInteger<uint64_t>();
  if (fs_.eof())
    return false;
  cb.original_size.n_records = readInteger<uint64_t>();

  cb.original_size.readlens = readInteger<uint64_t>();
  readBytes(cb.compressed_readlens);

  readBytes(cb.seq);
  readBytes(cb.qual);

  const auto n_fields = meta_.header_fmt.n_fields();
  cb.original_size.header_fields.resize(n_fields);
  cb.header_fields.resize(n_fields);
  cb.compressed_header_fields.resize(n_fields);

  for (std::size_t i = 0; i < n_fields; ++i) {
    auto &field_original_size = cb.original_size.header_fields[i];
    auto &field_cdata = cb.compressed_header_fields[i];

    if (meta_.header_fmt.field_types[i] == headers::FieldType::STRING) {
      field_original_size.isDifferentFlag = readInteger<uint64_t>();
      readBytes(field_cdata.isDifferentFlag);

      field_original_size.content = readInteger<uint64_t>();
      readBytes(field_cdata.content);

      field_original_size.contentLength = readInteger<uint64_t>();
      readBytes(field_cdata.contentLength);

    } else {
      field_original_size.content = readInteger<uint64_t>();
      readBytes(field_cdata.content);
    }
  }

  return true;
}

void Archive::writeBytes(const std::vector<std::byte> &bytes) {
  const auto sz = static_cast<std::streamsize>(bytes.size());
  writeInteger(sz);
  fs_.write(to_char_ptr(bytes.data()), sz);
}

void Archive::readBytes(std::vector<std::byte> &bytes) {
  const auto sz = readInteger<std::streamsize>();
  bytes.resize(static_cast<std::size_t>(sz));
  fs_.read(to_char_ptr(bytes.data()), sz);
}

} // namespace fqzcomp28
