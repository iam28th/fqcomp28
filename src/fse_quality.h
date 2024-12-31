#pragma once
#include "defs.h"
#include "fse_common.h"
#include <array>

namespace fqzcomp28 {
/* Quality context for the current symbol is:
 - previous symbol (6 bits)
 - max of symbols at offsets -2 and -3 (6 bits)
 - whether symbols at offsets -2 and -3 are equal or not (1 bit) */
class FSE_Quality {
public:
  /** number of preceeding symbols in quality ctx; (unlike sequence contest,
   * this value can not be tweaked) */
  constexpr static int CONTEXT_SIZE = 3;

  /** max. quality level; if higher quality values are expected (e.g. in pacbio
   * hifi ?), will need to seet the value to 127
   * (to keep alphabest size a power of 2) */
  constexpr static int MAX_SYMBOL = 63;
  constexpr static int ALPHABET_SIZE = MAX_SYMBOL + 1;

  constexpr static unsigned QUAL_OFFSET = 33;

  constexpr static unsigned QBITS = 12;

  /** masks bits which store quality symbols */
  constexpr static unsigned QCONTEXT_MASK = (1 << QBITS) - 1;
  constexpr static int CONTEXT_BITS = QBITS + 1;
  constexpr static int N_MODELS = 1 << CONTEXT_BITS;

protected:
  /**
   * @param q - Previous symbol
   * @param q1 - Symbol before q
   * @param q2 - Symbol before q1
   */
  /** number of bits in ctx reserved for previous quality values */
  static unsigned calcContext(unsigned q, unsigned q1, unsigned q2) {
    unsigned ctx = (((q1 > q2 ? q1 : q2) << 6) + q) & ((1 << QBITS) - 1);
    ctx += (static_cast<unsigned>(q1 == q2) << QBITS);
    return ctx;
  }

  template <typename T> using fse_array = std::array<T, N_MODELS>;

public:
  struct FreqTable {
    /** normalized to sum to power of 2^log */
    fse_array<std::array<short, ALPHABET_SIZE>> norm_counts;
    fse_array<unsigned> logs;
    unsigned max_log;

    bool operator==(const FreqTable &other) const = default;
  };

  FSE_Quality(const FreqTable *ft) : ft_(ft) {}

  static FreqTable calculateFreqTable(const FastqChunk &chunk);

protected:
  const FreqTable *ft_;

  static unsigned symbolToBits(const char q) {
    return static_cast<unsigned char>(q) - QUAL_OFFSET;
  }
};

// TODO merge repeating methods with FSE_Sequence
class QualityEncoder : FSE_Quality {
public:
  QualityEncoder(const FreqTable *);
  ~QualityEncoder();

  /**
   * Init states and tie bitStream to dst;
   * dst should have been resized by the caller
   */
  void startChunk(std::vector<std::byte> &dst);

  void encodeRecord(const FastqRecord &);

  /** @return Resulting compressed size */
  std::size_t endChunk();

private:
  BIT_CStream_t bitStream_;
  fse_array<FSE_CState_t> states_;
  fse_array<FSE_CTable *> tables_;
};

class QualityDecoder : FSE_Quality {
public:
  QualityDecoder(const FreqTable *ft);
  ~QualityDecoder();

  /** assumes that record fields readlen and qualp are correctly set */
  void decodeRecord(FastqRecord &);

  void startChunk(std::vector<std::byte> &src);
  void endChunk() const;

private:
  BIT_DStream_t bitStream_;
  fse_array<FSE_DState_t> states_;
  fse_array<FSE_DTable *> tables_;
};
} // namespace fqzcomp28
