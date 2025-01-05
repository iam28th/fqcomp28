#pragma once
#include "defs.h"
#include "fse_common.hpp"
#include <memory>

namespace fqzcomp28 {
/* Quality context for the current symbol is:
 - previous symbol (6 bits)
 - max of symbols at offsets -2 and -3 (6 bits)
 - whether symbols at offsets -2 and -3 are equal or not (1 bit) */
// TODO: make it stateless (that it, remove ft_ field)
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
  constexpr static unsigned QCONTEXT_MASK = (1U << QBITS) - 1;
  constexpr static unsigned CONTEXT_BITS = QBITS + 1;
  constexpr static int N_MODELS = 1U << CONTEXT_BITS;

protected:
  /**
   * @param q - Previous symbol
   * @param q1 - Symbol before q
   * @param q2 - Symbol before q1
   */
  /** number of bits in ctx reserved for previous quality values */
  static unsigned calcContext(unsigned q, unsigned q1, unsigned q2) {
    unsigned ctx = (((q1 > q2 ? q1 : q2) << 6U) + q) & ((1U << QBITS) - 1);
    ctx += (static_cast<unsigned>(q1 == q2) << QBITS);
    return ctx;
  }

public:
  using FreqTableT = FreqTable<N_MODELS, ALPHABET_SIZE>;
  template <typename T> using fse_array = FreqTableT::fse_array<T>;

  static std::unique_ptr<FreqTableT>
  calculateFreqTable(const FastqChunk &chunk);

protected:
  static unsigned symbolToBits(const char q) {
    return static_cast<unsigned char>(q) - QUAL_OFFSET;
  }
};

/**
 * Inherits parameters such as context size from FSE_Quality,
 * and initialization & cleanup methods from FSE_Encoder
 */
class QualityEncoder : FSE_Quality,
                       public FSE_Encoder<FSE_Quality::FreqTableT> {
public:
  explicit QualityEncoder(const FreqTableT *ft)
      : FSE_Encoder<FSE_Quality::FreqTableT>(ft) {};

  void encodeRecord(const FastqRecord &);
};

/**
 * Inherits parameters such as context size from FSE_Quality,
 * and initialization & cleanup methods from FSE_Encoder
 */
class QualityDecoder : FSE_Quality,
                       public FSE_Decoder<FSE_Quality::FreqTableT> {
public:
  explicit QualityDecoder(const FreqTableT *ft)
      : FSE_Decoder<FSE_Quality::FreqTableT>(ft) {};

  /** assumes that record fields readlen and qualp are correctly set */
  void decodeRecord(FastqRecord &);
};
} // namespace fqzcomp28
