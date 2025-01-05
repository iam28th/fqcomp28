#include "fse_quality.h"

namespace fqzcomp28 {

QualityEncoder::QualityEncoder(const FreqTableT *ft) : FSE_Quality(ft) {
  // according to commens in fse.h, MAX_SYMBOL should be enough there
  // but according to valgrind it's not;
  // (could be because some frequency tables are "empty")
  auto wksp = createCTableBuildWksp(MAX_SYMBOL + 1, ft_->max_log);

  for (unsigned ctx = 0; ctx < N_MODELS; ++ctx) {
    tables_[ctx] = FSE_createCTable(MAX_SYMBOL, ft_->logs[ctx]);
    assert(static_cast<long int>(wksp.size()) >= (1 << ft_->max_log));

    [[maybe_unused]] const std::size_t ret = FSE_buildCTable_wksp(
        tables_[ctx], ft_->norm_counts[ctx].data(), MAX_SYMBOL, ft->logs[ctx],
        wksp.data(), wksp.size() * sizeof(decltype(wksp)::value_type));
    assert(ret == 0);
  }
}

QualityDecoder::QualityDecoder(const FreqTableT *ft) : FSE_Quality(ft) {
  std::vector<unsigned> wksp(
      FSE_BUILD_DTABLE_WKSP_SIZE_U32(ft->max_log, MAX_SYMBOL));

  for (unsigned ctx = 0; ctx < N_MODELS; ++ctx) {
    tables_[ctx] = FSE_createDTable(ft_->logs[ctx]);

    [[maybe_unused]] const std::size_t ret = FSE_buildDTable_wksp(
        tables_[ctx], ft_->norm_counts[ctx].data(), MAX_SYMBOL, ft->logs[ctx],
        wksp.data(), wksp.size() * sizeof(decltype(wksp)::value_type));
    assert(ret == 0);
  }
}

QualityEncoder::~QualityEncoder() {
  for (FSE_CTable *ct : tables_)
    FSE_freeCTable(ct);
}

QualityDecoder::~QualityDecoder() {
  for (FSE_DTable *dt : tables_)
    FSE_freeDTable(dt);
}

void QualityEncoder::startChunk(std::vector<std::byte> &dst) {
  [[maybe_unused]] auto ret =
      BIT_initCStream(&bitStream_, dst.data(), dst.size());
  assert(ret == 0);
  for (unsigned ctx = 0; ctx < N_MODELS; ++ctx)
    FSE_initCState(states_.begin() + ctx, tables_[ctx]);
}

std::size_t QualityEncoder::endChunk() {
  for (auto &state : states_)
    FSE_flushCState(&bitStream_, &state);
  return BIT_closeCStream(&bitStream_);
}

void QualityDecoder::startChunk(std::vector<std::byte> &src) {
  [[maybe_unused]] auto ret =
      BIT_initDStream(&bitStream_, src.data(), src.size());
  assert(ret == src.size());
  /* init states in reverse flushing order */
  for (unsigned i = N_MODELS; i > 0; --i) {
    const unsigned ctx = i - 1;
    FSE_initDState(states_.begin() + ctx, &bitStream_, tables_[ctx]);
  }
}

void QualityDecoder::endChunk() const { assert(BIT_endOfDStream(&bitStream_)); }

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
