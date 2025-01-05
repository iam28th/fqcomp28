#include "fse_sequence.h"
#include "sequtils.h"
#include "utils.h"
#include <ranges>

namespace fqzcomp28 {

constexpr std::array<unsigned, 128> base2bits_arr = []() {
  std::array<unsigned, 128> ret;
  ret.fill(std::numeric_limits<unsigned>::max());
  ret['A'] = 0b00;
  ret['C'] = 0b01;
  ret['G'] = 0b10;
  ret['T'] = 0b11;
  return ret;
}();

unsigned FSE_Sequence::addBaseLower(unsigned ctx, char base) {
  return ((ctx << 2) + base2bits_arr[static_cast<unsigned>(base)]) &
         CONTEXT_MASK;
}

void SequenceEncoder::encodeRecord(FastqRecord &r, CompressedBuffersDst &cbs) {
  readlen_t n_count = 0;

  readlen_t prev_n_pos = 0;
  for (unsigned short i = 0; i < r.length; ++i) {
    if (r.seqp[i] == 'N') {
      n_count++;
      const readlen_t delta = i - prev_n_pos;
      storeAsBytes(delta, cbs.n_pos);
      // TODO 'most likely' base instead of always A
      r.seqp[i] = 'A';
      prev_n_pos = i;
    }
  }

  storeAsBytes(n_count, cbs.n_count);

  unsigned ctx = INITIAL_CONTEXT & CONTEXT_MASK;

  /* form context of the last base; add CONTEXT_SIZE - 1 symbols
   * (the closest symbol is in the upper 2 bits of the context) */
  const int to_add_from_sequence = std::min(CONTEXT_SIZE - 1, r.length - 1);
  const char *base = r.seqp + r.length - 2;

  for (int i = 0; i < to_add_from_sequence; ++i)
    ctx = addBaseLower(ctx, *base--);

  /* points to the first base that has less than
   * CONTEXT_SIZE bases to its left */
  const char *first_base_with_partial_ctx =
      r.seqp + (r.length > CONTEXT_SIZE ? CONTEXT_SIZE - 1 : r.length - 1);

  assert(*(r.seqp - 1) == '\n'); /* it's safe to use this address becuase it's
                                    in a block of fastq data */

  /* encode the part which has all context formed by r */
  for (base = r.seqp + r.length - 1; base > first_base_with_partial_ctx;
       --base) {

    /* this loop isn't entered if r.length <= CONTEXT_SIZE */
    assert(r.length > CONTEXT_SIZE);

    /* update ctx with the furtherest symbol;
     * the closest symbol is in the upper 2 bits of the context */
    ctx = addBaseLower(ctx, *(base - CONTEXT_SIZE));
    assert(getClosestBase(ctx) == *(base - 1));

    const unsigned sym = base2bits_arr[static_cast<unsigned>(*base)];
    FSE_encodeSymbol(&bitStream_, states_.begin() + ctx, sym);
    BIT_flushBitsFast(&bitStream_); // TODO only flush on every K-th iteration?
  }

  const int to_add_from_initial = (CONTEXT_SIZE - 1) - to_add_from_sequence;
  int i;
  for (i = 0; i < to_add_from_initial; ++i) {
    /* this loop isn't entered if r.length > CONTEXT_SIZE */
    assert(r.length <= CONTEXT_SIZE);
    ctx = addBaseLower(ctx, INITIAL_CONTEXT_SEQ_REV[i]);
  }

  for (base = first_base_with_partial_ctx; base >= r.seqp; --base, ++i) {

    ctx = addBaseLower(ctx, INITIAL_CONTEXT_SEQ_REV[i]);

    if (base == r.seqp)
      assert(ctx == INITIAL_CONTEXT);

    const unsigned sym = base2bits_arr[static_cast<unsigned>(*base)];
    FSE_encodeSymbol(&bitStream_, states_.begin() + ctx, sym);
    BIT_flushBitsFast(&bitStream_);
  }
}

void SequenceDecoder::decodeRecord(FastqRecord &r, CompressedBuffersSrc &cbs) {
  assert(cbs.index.n_count >= sizeof(readlen_t));
  const auto n_count = loadFromBytes<readlen_t>(
      cbs.n_count, cbs.index.n_count - sizeof(readlen_t));
  cbs.index.n_count -= sizeof(readlen_t);

  npos_buffer_.resize(n_count);
  for (auto &npos : npos_buffer_ | std::views::reverse) {
    assert(cbs.index.n_pos >= sizeof(readlen_t));
    npos = loadFromBytes<readlen_t>(cbs.n_pos,
                                    cbs.index.n_pos - sizeof(readlen_t));
    assert(npos < r.length);
    cbs.index.n_pos -= sizeof(readlen_t);
  }

  unsigned ctx = INITIAL_CONTEXT;

  readlen_t prev_n_pos = 0;
  unsigned n_added = 0;
  for (unsigned short i = 0; i < r.length; ++i) {
    const unsigned sym = FSE_decodeSymbol(states_.data() + ctx, &bitStream_);
    BIT_reloadDStream(&bitStream_);

    r.seqp[i] = bits2base(sym);
    ctx = addSymUpper(ctx, sym);

    if ((n_added != n_count) && (i == prev_n_pos + npos_buffer_[n_added])) {
      r.seqp[i] = 'N';
      prev_n_pos = i;
      n_added++;
    }
  }
}

std::unique_ptr<FSE_Sequence::FreqTableT>
FSE_Sequence::calculateFreqTable(const FastqChunk &chunk) {
  /* count frequencies of each symbol in every context */
  fse_array<std::array<unsigned, ALPHABET_SIZE>> counts{};
  for (auto &c : counts)
    c.fill(1);

  /* the closest symbol is in the upper 2 bits of the context */
  for (const auto &r : chunk.records) {
    unsigned ctx = INITIAL_CONTEXT; // reset at the beginning of a record

    for (char c : r.seq()) {
      if (c == 'N')
        continue;

      const unsigned symbol = base2bits_arr[static_cast<unsigned>(c)];
      assert(symbol <= MAX_SYMBOL);
      counts[ctx][symbol]++;

      ctx = addSymUpper(ctx, symbol);
    }
  }

  return makeNormalizedFreqTable<FreqTableT>(counts);
}

} // namespace fqzcomp28
