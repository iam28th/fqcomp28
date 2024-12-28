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
  if (CONTEXT_SIZE + 1 > r.length) [[unlikely]] // TODO
    throw std::runtime_error("too short reads not support (yet)");

  unsigned ctx = INITIAL_CONTEXT;

  /* form context of the last base; add CONTEXT_SIZE - 1 symbols
   * (the closest symbol is in the upper 2 bits of the context) */
  for (char *base = r.seqp + r.length - 2,
            *E = std::max(base - (CONTEXT_SIZE - 1), r.seqp - 1);
       base > E; --base) {
    ctx = (ctx << 2) + base2bits(*base);
  }

  for (const char *base = r.seqp + r.length - 1, *E = r.seqp + CONTEXT_SIZE - 1;
       base > E; --base) {
    ctx = ((ctx << 2) + base2bits(*(base - CONTEXT_SIZE))) & CTX_MASK;

    /* the closest symbol is in the upper 2 bits of the context */
    assert(bits2base(ctx >> 2 * (CONTEXT_SIZE - 1)) == *(base - 1));

    const unsigned sym = base2bits(*base);
    FSE_encodeSymbol(&bitStream_, states_.begin() + ctx, sym);
    BIT_flushBitsFast(&bitStream_); // TODO only flush on every K-th iteration?
  }

  /* when encoding beginning of the sequence, ctx is updated with
   * bytes from INITIAL_CONTEXT */
  for (int i = 0,
           E = std::min(CONTEXT_SIZE - 1, static_cast<int>(r.length - 1));
       i <= E; ++i) {
    ctx = ((ctx << 2) +
           *(reinterpret_cast<const unsigned char *>(&INITIAL_CONTEXT) + i)) &
          CTX_MASK;

    if (i == E) {
      assert(ctx == INITIAL_CONTEXT);
    }

    const unsigned sym = base2bits(*(r.seqp + (CONTEXT_SIZE - i - 1)));
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
    ctx = (ctx >> 2) + (sym << (2 * (CONTEXT_SIZE - 1)));
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

      ctx = (ctx >> 2) + (symbol << (2 * (CONTEXT_SIZE - 1)));
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

void encodeRecord(const FastqRecord &r);
} // namespace fqzcomp28
