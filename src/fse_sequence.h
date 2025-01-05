#pragma once
#include "compressed_buffers.h"
#include "defs.h"
#include "fse_common.hpp"
#include "sequtils.h"

namespace fqzcomp28 {

class FSE_Sequence {
public:
  /**
   * Number of bases in sequence context; increasing leads to
   * slightly higher CR but noticeably longer compression
   */
  constexpr static int CONTEXT_SIZE = 4;
  constexpr static unsigned CONTEXT_MASK = (1 << (CONTEXT_SIZE * 2)) - 1;

  /** records base (in 2bit representation) in the lower 2 bits of ctx */
  static unsigned addBaseLower(unsigned ctx, char base);

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

  constexpr static unsigned MAX_SYMBOL = base2bits('T');
  constexpr static unsigned ALPHABET_SIZE = MAX_SYMBOL + 1;

  /* Initial ctx value from fqzcomp4
   * (but in reverse base order)
   * direct base order is: 0x007616c7
   * "it corresponds to 12-kmer that does not occur in a human genome" (c) */
  constexpr static unsigned RARE_KMER_LEN = 12;
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

public:
  using FreqTableT = FreqTable<N_MODELS, ALPHABET_SIZE>;
  template <typename T> using fse_array = FreqTableT::fse_array<T>;

  static std::unique_ptr<FreqTableT>
  calculateFreqTable(const FastqChunk &chunk);
};

/**
 * Inherits parameters such as context size from FSE_Sequence,
 * and initialization & cleanup methods from FSE_Encoder
 */
class SequenceEncoder : FSE_Sequence,
                        public FSE_Encoder<FSE_Sequence::FreqTableT> {
public:
  SequenceEncoder(const FreqTableT *ft)
      : FSE_Encoder<FSE_Sequence::FreqTableT>(ft) {};

  /**
   * Writes sequence to bitStream_, skipping Ns;
   * count and positions of Ns are written into cbs
   *
   * @param r - Record to encode; it's not const because its contents
   * are possibly modified by replacing N with one of ACGT
   */
  void encodeRecord(FastqRecord &rec, CompressedBuffersDst &cbs);
};

class SequenceDecoder : FSE_Sequence,
                        public FSE_Decoder<FSE_Sequence::FreqTableT> {
public:
  SequenceDecoder(const FreqTableT *ft)
      : FSE_Decoder<FSE_Sequence::FreqTableT>(ft) {}

  /**
   * Assumes that r's fields readlen and seqp are correctly set;
   * decodes bases from bitStream_ and reads Ns from cbs
   */
  void decodeRecord(FastqRecord &r, CompressedBuffersSrc &cbs);

private:
  /** Used to store positions of Ns in a read currently being decoded */
  std::vector<readlen_t> npos_buffer_;
};

} // namespace fqzcomp28
