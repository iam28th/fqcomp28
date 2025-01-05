#include "fse_quality.h"

namespace fqzcomp28 {

void QualityEncoder::encodeRecord(const FastqRecord &r) {
  /** symbol to encode */
  const char *qual = r.qualp + r.length - 1;
  unsigned sym = symbolToBits(*qual);

  /* context for the last quality symbol */
  unsigned q = symbolToBits(*(qual - 1)), q1 = symbolToBits(*(qual - 2)),
           q2 = symbolToBits(*(qual - 3));

  const char *const first_sym_with_partial_ctx =
      r.qualp +
      (r.length >= CONTEXT_SIZE + 1 ? CONTEXT_SIZE - 1 : r.length - 1);

  unsigned ctx;
  for (; qual > first_sym_with_partial_ctx;) {
    assert(q == symbolToBits(*(qual - 1)));
    assert(q1 == symbolToBits(*(qual - 2)));
    assert(q2 == symbolToBits(*(qual - 3)));

    ctx = calcContext(q, q1, q2);
    FSE_encodeSymbol(&bitStream_, states_.begin() + ctx, sym);
    BIT_flushBitsFast(&bitStream_);

    sym = q;
    q = q1;
    q1 = q2;
    q2 = symbolToBits(*(--qual - 3));
  }

  // TODO: rewrite branchless
  if (r.length >= 3) [[likely]] {
    assert(*(qual - 3) == '\n');
    ctx = calcContext(q, q1, 0);
    FSE_encodeSymbol(&bitStream_, states_.begin() + ctx, sym);
    BIT_flushBitsFast(&bitStream_);
  }

  if (r.length >= 2) [[likely]] {
    ctx = calcContext(q1, 0, 0);
    assert(q == symbolToBits(*(r.qualp + 1)));
    FSE_encodeSymbol(&bitStream_, states_.begin() + ctx, q);
    BIT_flushBitsFast(&bitStream_);
  }

  ctx = calcContext(0, 0, 0);
  assert(q1 == symbolToBits(*r.qualp));
  FSE_encodeSymbol(&bitStream_, states_.begin() + ctx, q1);
  BIT_flushBitsFast(&bitStream_);
}

void QualityDecoder::decodeRecord(FastqRecord &r) {
  unsigned q2 = 0, q1 = 0;
  unsigned ctx = calcContext(0, 0, 0);
  for (char *out = r.qualp, *E = r.qualp + r.length; out != E; ++out) {
    const unsigned q = FSE_decodeSymbol(states_.data() + ctx, &bitStream_);
    BIT_reloadDStream(&bitStream_);

    *out = static_cast<char>(q + QUAL_OFFSET);
    ctx = calcContext(q, q1, q2);
    q2 = q1;
    q1 = q;
  }
}

std::unique_ptr<FSE_Quality::FreqTableT>
FSE_Quality::calculateFreqTable(const FastqChunk &chunk) {
  /* count frequencies of each symbol in every context */

  auto counts =
      std::make_unique<fse_array<std::array<unsigned, ALPHABET_SIZE>>>();
  for (auto &c : *counts)
    c.fill(1);

  /* the closest symbol is in the upper 2 bits of the context */
  for (const auto &r : chunk.records) {
    unsigned ctx = calcContext(0, 0, 0); // reset at the beginning of a record
    unsigned q1 = 0, q2 = 0;

    for (char c : r.qual()) {
      assert(static_cast<char>(static_cast<unsigned>(c)) == c);
      const unsigned q = static_cast<unsigned>(c) - QUAL_OFFSET;
      assert(q <= MAX_SYMBOL);

      counts->at(ctx).at(q)++;

      ctx = calcContext(q, q1, q2);
      q2 = q1;
      q1 = q;
    }
  }

  return makeNormalizedFreqTable<FreqTableT>(*counts);
}
}; // namespace fqzcomp28
