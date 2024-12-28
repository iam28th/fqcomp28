#pragma once
#include "defs.h"
#include "entropy_common.h"
#include "sequtils.h"
#include <array>

namespace fqzcomp28 {

class FSE_Sequence {
public:
  constexpr static int MAX_SYMBOL = base2bits('T');
  constexpr static int ALPHABET_SIZE = MAX_SYMBOL + 1;
  // TODO: use initial ctx value from fqzcomp4
  // (but in reverse base order)
  // direct base order is: 0x007616c7
  constexpr static unsigned INITIAL_CONTEXT = 0;
  // TODO: define it through macro
  constexpr static int CONTEXT_SIZE = 5;
  constexpr static unsigned CTX_MASK = (1 << (CONTEXT_SIZE * 2)) - 1;
  constexpr static unsigned N_MODELS = 1 << (2 * CONTEXT_SIZE);

protected:
  /** stores some object (e.g., counts) for each model */
  template <typename T> using fse_array = std::array<T, N_MODELS>;

public:
  struct FreqTable {
    /** normalized to sum to power of 2^log */
    fse_array<std::array<short, ALPHABET_SIZE>> norm_counts;
    fse_array<unsigned> logs;
    unsigned max_log;

    bool operator==(const FreqTable &other) const = default;
  };

  FSE_Sequence(const FreqTable *ft) : ft_(ft) {}

  static FreqTable calculateFreqTable(const FastqChunk &chunk);

protected:
  const FreqTable *ft_;
};

class SequenceEncoder : FSE_Sequence {
public:
  SequenceEncoder(const FreqTable *ft);
  ~SequenceEncoder();

  void encodeRecord(const FastqRecord &);

  void startChunk(std::vector<std::byte> &dst);
  /** @return Resulting compressed size */
  std::size_t endChunk();

private:
  BIT_CStream_t bitStream_;
  fse_array<FSE_CState_t> states_;
  fse_array<FSE_CTable *> tables_;
};

class SequenceDecoder : FSE_Sequence {
public:
  SequenceDecoder(const FreqTable *ft);
  ~SequenceDecoder();

  /** assumes that record fields readlen and seqp are correctly set */
  void decodeRecord(FastqRecord &);

  void startChunk(std::vector<std::byte> &src);
  void endChunk() const;

private:
  BIT_DStream_t bitStream_;
  fse_array<FSE_DState_t> states_;
  fse_array<FSE_DTable *> tables_;
};

} // namespace fqzcomp28
