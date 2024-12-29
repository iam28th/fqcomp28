#include "entropy_sequence.h"
#include "sequtils.h"
#include <numeric>

namespace fqzcomp28 {
SequenceEncoder::SequenceEncoder(const FreqTable *ft) : FSE_Sequence(ft) {
  auto wksp = createCTableBuildWksp(MAX_SYMBOL, ft_->max_log);

  for (unsigned ctx = 0; ctx < N_MODELS; ++ctx) {
    tables_[ctx] = FSE_createCTable(MAX_SYMBOL, ft_->logs[ctx]);

    [[maybe_unused]] std::size_t ret = FSE_buildCTable_wksp(
        tables_[ctx], ft_->norm_counts[ctx].data(), MAX_SYMBOL, ft->logs[ctx],
        wksp.data(), wksp.size());
    assert(ret == 0);
  }
}

SequenceEncoder::~SequenceEncoder() {
  for (FSE_CTable *ct : tables_)
    FSE_freeCTable(ct);
}

SequenceDecoder::SequenceDecoder(const FreqTable *ft) : FSE_Sequence(ft) {
  std::vector<unsigned> dWorkspace(
      FSE_BUILD_DTABLE_WKSP_SIZE_U32(ft->max_log, MAX_SYMBOL));

  for (unsigned ctx = 0; ctx < N_MODELS; ++ctx) {
    tables_[ctx] = FSE_createDTable(ft->logs[ctx]);
    std::size_t ret = FSE_buildDTable_wksp(
        tables_[ctx], ft->norm_counts[ctx].data(), MAX_SYMBOL, ft->logs[ctx],
        dWorkspace.data(),
        dWorkspace.size() * sizeof(decltype(dWorkspace)::value_type));
    assert(ret == 0);
  }
}

SequenceDecoder::~SequenceDecoder() {
  for (FSE_DTable *dt : tables_)
    FSE_freeDTable(dt);
}

void SequenceEncoder::startChunk(std::vector<std::byte> &dst) {
  [[maybe_unused]] auto ret =
      BIT_initCStream(&bitStream_, dst.data(), dst.size());
  assert(ret == 0);

  for (unsigned ctx = 0; ctx < N_MODELS; ++ctx)
    FSE_initCState(states_.begin() + ctx, tables_[ctx]);
}

std::size_t SequenceEncoder::endChunk() {
  for (auto &state : states_)
    FSE_flushCState(&bitStream_, &state);
  return BIT_closeCStream(&bitStream_);
}

void SequenceDecoder::startChunk(std::vector<std::byte> &src) {
  [[maybe_unused]] auto ret =
      BIT_initDStream(&bitStream_, src.data(), src.size());
  assert(ret == src.size());
  /* init states in reverse flushing order */
  for (unsigned i = N_MODELS; i > 0; --i) {
    unsigned ctx = i - 1;
    FSE_initDState(states_.begin() + ctx, &bitStream_, tables_[ctx]);
  }
}

void SequenceDecoder::endChunk() const {
  assert(BIT_endOfDStream(&bitStream_));
}

void SequenceEncoder::encodeRecord(const FastqRecord &r) {

  unsigned ctx = INITIAL_CONTEXT & CTX_MASK;

  /* form context of the last base; add CONTEXT_SIZE - 1 symbols
   * (the closest symbol is in the upper 2 bits of the context) */
  const int to_add_from_sequence = std::min(CONTEXT_SIZE - 1, r.length - 1);
  const char *base = r.seqp + r.length - 2;

  for (int i = 0; i < to_add_from_sequence; ++i)
    ctx = addBaseLower(ctx, *base--);

  /* points to the first base that has less than
   * CONTEXT_SIZE bases to its left */
  const char *first_base_with_partial_ctx = r.length > CONTEXT_SIZE
                                                ? r.seqp + CONTEXT_SIZE - 1
                                                : r.seqp + r.length - 1;
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

    const unsigned sym = base2bits(*base);
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

    const unsigned sym = base2bits(*base);
    FSE_encodeSymbol(&bitStream_, states_.begin() + ctx, sym);
    BIT_flushBitsFast(&bitStream_);
  }
}

void SequenceDecoder::decodeRecord(FastqRecord &r) {
  unsigned ctx = INITIAL_CONTEXT;
  for (char *out = r.seqp, *E = r.seqp + r.length; out != E; ++out) {
    unsigned sym = FSE_decodeSymbol(states_.data() + ctx, &bitStream_);
    BIT_reloadDStream(&bitStream_);

    *out = bits2base(sym);
    ctx = addSymUpper(ctx, sym);
  }
}

FSE_Sequence::FreqTable
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

      const unsigned symbol = base2bits(c);
      counts[ctx][symbol]++;

      ctx = addSymUpper(ctx, symbol);
    }
  }
  /* normalize frequencies */
  FreqTable ft{};

  for (unsigned ctx = 0; ctx < N_MODELS; ++ctx) {
    const std::size_t ctx_size =
        std::accumulate(counts[ctx].begin(), counts[ctx].end(), std::size_t{});

    ft.logs[ctx] = FSE_optimalTableLog(0, ctx_size, MAX_SYMBOL);
    const std::size_t log =
        FSE_normalizeCount(ft.norm_counts[ctx].data(), ft.logs[ctx],
                           counts[ctx].data(), ctx_size, MAX_SYMBOL, 1);
    assert(log == ft.logs[ctx]);
    ft.max_log = std::max(ft.max_log, ft.logs[ctx]);
  }
  return ft;
}

} // namespace fqzcomp28
