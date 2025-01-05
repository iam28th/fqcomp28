#pragma once
#include <array>
#include <memory>
#include <numeric>
#include <vector>

// assert needs to be defined before including fse headrs,
// otherwise zstd defines it to noop ...
#include <cassert>

/* headers from zstd library */
extern "C" {
#define FSE_STATIC_LINKING_ONLY
#include <common/bitstream.h>
#include <common/fse.h>
#include <compress/hist.h>
}

namespace fqzcomp28 {

inline std::vector<FSE_FUNCTION_TYPE>
createCTableBuildWksp(const unsigned maxSymbolValue, const unsigned tableLog) {
  return std::vector<FSE_FUNCTION_TYPE>(
      FSE_BUILD_CTABLE_WORKSPACE_SIZE(maxSymbolValue, tableLog));
}

#if 0
/**
 * A class template for common FSE encoder methods
 * (currently these are ctor and dtor)
 */
template <class FreqTableT> class FSE_Encoder {};
#endif

/**
 * A class template for frequency tables used by FSE codec
 */
template <unsigned N_MODELS_, unsigned ALPHABET_SIZE_> struct FreqTable {

  const static unsigned N_MODELS = N_MODELS_;
  const static unsigned ALPHABET_SIZE = ALPHABET_SIZE_;
  const static unsigned MAX_SYMBOL = ALPHABET_SIZE - 1;

  template <typename T> using fse_array = std::array<T, N_MODELS>;

  /** normalized to sum to power of 2^log */
  fse_array<std::array<short, ALPHABET_SIZE>> norm_counts;

  fse_array<unsigned> logs;
  unsigned max_log;

  bool operator==(const FreqTable &other) const = default;
};

/**
 * Creates FreqTable with normalized counts using exact counts in each context
 */
template <typename FreqTableT, typename Counts>
std::unique_ptr<FreqTableT> makeNormalizedFreqTable(const Counts &counts)
  requires(std::tuple_size_v<Counts> == FreqTableT::N_MODELS &&
           std::tuple_size_v<typename Counts::value_type> ==
               FreqTableT::ALPHABET_SIZE)
{
  auto ft = std::make_unique<FreqTableT>();

  for (unsigned ctx = 0; ctx < FreqTableT::N_MODELS; ++ctx) {
    const std::size_t ctx_size =
        std::accumulate(counts[ctx].begin(), counts[ctx].end(), std::size_t{});

    ft->logs[ctx] = FSE_optimalTableLog(0, ctx_size, FreqTableT::MAX_SYMBOL);
    [[maybe_unused]] const std::size_t log = FSE_normalizeCount(
        ft->norm_counts[ctx].data(), ft->logs[ctx], counts[ctx].data(),
        ctx_size, FreqTableT::MAX_SYMBOL, 1);

    assert(log == ft->logs[ctx]);
    ft->max_log = std::max(ft->max_log, ft->logs[ctx]);
  }
  return ft;
};

} // namespace fqzcomp28
