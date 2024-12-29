#pragma once
#include "defs.h"
#include "entropy_common.h"
#include "sequtils.h"
#include <array>

namespace fqzcomp28 {

class FSE_Sequence {
public:
  /** number of bases in sequence context */
  constexpr static int CONTEXT_SIZE = 5;
  constexpr static unsigned CTX_MASK = (1 << (CONTEXT_SIZE * 2)) - 1;

  /** records base in the lower 2 bits of ctx */
  constexpr static unsigned addBaseLower(unsigned ctx, char base) {
    return ((ctx << 2) + base2bits(base)) & CTX_MASK;
  }

  /** records base (in 2bit reperesentation) in the upper 2 bits of ctx */
  constexpr static unsigned addSymUpper(unsigned ctx, unsigned sym) {
    return (ctx >> 2) + (sym << (2 * (CONTEXT_SIZE - 1)));
  }

  constexpr static unsigned getUpperTwoBits(unsigned ctx) {
    return ctx >> (2 * (CONTEXT_SIZE - 1));
  }

  constexpr static char getClosestBase(unsigned ctx) {
    return bits2base(getUpperTwoBits(ctx));
  }

  constexpr static int MAX_SYMBOL = base2bits('T');
  constexpr static int ALPHABET_SIZE = MAX_SYMBOL + 1;
  // using initial ctx value from fqzcomp4
  // (but in reverse base order)
  // direct base order is: 0x007616c7
  // "it corresponds to 12-kmer that does not occur in a human genome" (c)
  constexpr static int RARE_KMER_LEN = 12;
  constexpr static char INITIAL_CONTEXT_SEQ[RARE_KMER_LEN + 1] = "CTCCACCCTCCT";
  constexpr static char INITIAL_CONTEXT_SEQ_REV[RARE_KMER_LEN + 1] =
      "TCCTCCCACCTC";
  constexpr static unsigned INITIAL_CONTEXT = []() constexpr {
    unsigned ctx = 0;
    auto add_base = [&ctx](int idx) {
      ctx = (ctx << 2) + base2bits(INITIAL_CONTEXT_SEQ[idx]);
    };
    add_base(11);
    add_base(10);
    add_base(9);
    add_base(8);
    add_base(7);
    add_base(6);
    add_base(5);
    add_base(4);
    add_base(3);
    add_base(2);
    add_base(1);
    add_base(0);
    return ctx;
  }() >> (RARE_KMER_LEN * 2 - CONTEXT_SIZE * 2);

  /* should be positve ? */
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
