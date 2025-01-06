#include "archive.h"
#include "utils.h"

namespace fqzcomp28 {

Archive::Archive(const path_t archive_path)
    : fs_(archive_path, std::ios_base::binary | std::ios_base::in) {

  if (!fs_)
    throw std::system_error(errno, std::generic_category(), archive_path);

  readArchiveHeader();
}

Archive::Archive(const path_t archive_path, const path_t file_to_gather_meta,
                 const std::size_t sample_size_bytes)
    : fs_(archive_path, std::ios_base::binary | std::ios_base::out) {
  if (!fs_)
    throw std::system_error(errno, std::generic_category(), archive_path);

  meta_ = analyzeDataset(file_to_gather_meta, sample_size_bytes);

  writeMeta();
}

void Archive::writeMeta() {
  fs_.seekp(OFFSET_META);
  DatasetMeta::storeToStream(meta_, fs_);
}

void Archive::readArchiveHeader() {
  uint32_t n_blocks;
  fs_.read(to_char_ptr(&n_blocks), sizeof(n_blocks));

  meta_ = DatasetMeta::loadFromStream(fs_);
  const auto data_start_pos = fs_.tellg();

  fs_.seekg(-narrow_cast<std::streamoff>(n_blocks * sizeof(BlockInfo)),
            std::ios_base::end);
  index_.resize(n_blocks);
  fs_.read(to_char_ptr(index_.data()),
           narrow_cast<std::streamsize>(n_blocks * sizeof(BlockInfo)));

  fs_.seekg(data_start_pos);

  sortIndex();
}

void Archive::writeIndex() {
  const std::streamoff data_end_pos = fs_.tellp();

  fs_.seekp(0);
  const auto index_size = narrow_cast<uint32_t>(index_.size());
  fs_.write(to_char_ptr(&index_size), sizeof(index_size));

  fs_.seekp(data_end_pos);
  fs_.write(to_char_ptr(index_.data()),
            narrow_cast<std::streamoff>(index_.size() * sizeof(BlockInfo)));
}

void Archive::writeBlock(const CompressedBuffersDst &cb) {
  BlockInfo binfo;
  binfo.idx = cb.chunk_idx;

  const std::lock_guard guard(mtx_);

  binfo.offset = narrow_cast<int64_t>(fs_.tellp());

  /* these 2 number can be deduced,
   * but let's just store them to simplify decompression */
  writeInteger(cb.original_size.total);
  writeInteger(cb.original_size.n_records);

  /* for each buffer we store original size,
   * followed by compressed size, followed by compressed data */
  writeInteger(cb.original_size.readlens);
  writeBytes(cb.compressed_readlens);

  writeInteger(cb.original_size.n_count);
  writeBytes(cb.compressed_n_count);

  writeInteger(cb.original_size.n_pos);
  writeBytes(cb.compressed_n_pos);

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

  index_.push_back(binfo);
}

bool Archive::readBlock(CompressedBuffersSrc &cb) {
  /* safe to call before lock,
   * becase each thread operates on each own's objects */
  cb.clear();

  const std::lock_guard guard(mtx_);
  if (blocks_processed_ == index_.size())
    return false;

  const auto &binfo = index_[blocks_processed_++];
  fs_.seekg(binfo.offset);
  cb.chunk_idx = binfo.idx;

  cb.original_size.total = readInteger<uint32_t>();

  cb.original_size.n_records = readInteger<uint32_t>();

  cb.original_size.readlens = readInteger<uint32_t>();
  readBytes(cb.compressed_readlens);

  cb.original_size.n_count = readInteger<uint32_t>();
  readBytes(cb.compressed_n_count);

  cb.original_size.n_pos = readInteger<uint32_t>();
  readBytes(cb.compressed_n_pos);

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
      field_original_size.isDifferentFlag = readInteger<uint32_t>();
      readBytes(field_cdata.isDifferentFlag);

      field_original_size.content = readInteger<uint32_t>();
      readBytes(field_cdata.content);

      field_original_size.contentLength = readInteger<uint32_t>();
      readBytes(field_cdata.contentLength);

    } else {
      field_original_size.content = readInteger<uint32_t>();
      readBytes(field_cdata.content);
    }
  }

  return true;
}

void Archive::writeBytes(const std::vector<std::byte> &bytes) {
  const auto sz = narrow_cast<uint32_t>(bytes.size());
  writeInteger(sz);
  fs_.write(to_char_ptr(bytes.data()), sz);
}

void Archive::readBytes(std::vector<std::byte> &bytes) {
  const auto sz = readInteger<uint32_t>();
  bytes.resize(sz);
  fs_.read(to_char_ptr(bytes.data()), sz);
}

} // namespace fqzcomp28
