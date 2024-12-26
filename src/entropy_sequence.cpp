#include "entropy_sequence.h"
#include "sequtils.h"
#include <numeric>

namespace fqzcomp28 {
SequenceCoder::FreqTable

SequenceCoder::calculateFreqTable(const FastqChunk &chunk) {

  /* count frequencies of each symbol in each context */
  fse_array<std::array<unsigned, ALPHABET_SIZE>> counts{};
  for (const auto &r : chunk.records) {
    unsigned ctx = INITIAL_CONTEXT; // reset at the beginning of a record

    for (char c : r.seq()) {
      if (c == 'N')
        continue;

      const unsigned symbol = base2bits(c);
      counts[ctx][symbol]++;
      ctx = ((ctx << 2) + symbol) & CTX_MASK;
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
