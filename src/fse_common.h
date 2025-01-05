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

/**
 * A class template for common FSE encoder methods
 * (currently these are ctor/dtor, and {start,end}Chunk)
 */
template <class FreqTableT> class FSE_Encoder {
protected:
  const FreqTableT *ft_;
  BIT_CStream_t bitStream_;

  template <typename T> using fse_array = FreqTableT::template fse_array<T>;
  fse_array<FSE_CState_t> states_;
  fse_array<FSE_CTable *> tables_;

  FSE_Encoder(const FreqTableT *ft) : ft_(ft) {
    /* according to fse.h, MAX_SYMBOL should be enough here
     * but according to valgrind it's not (in case of quality
     * alphabet on a very small input;
     * could be because some frequency tables are "empty") */
    auto wksp = createCTableBuildWksp(ft_->MAX_SYMBOL + 1, ft_->max_log);

    for (unsigned ctx = 0; ctx < FreqTableT::N_MODELS; ++ctx) {
      tables_[ctx] = FSE_createCTable(FreqTableT::MAX_SYMBOL, ft_->logs[ctx]);
      assert(static_cast<long int>(wksp.size()) >= (1 << ft_->max_log));

      [[maybe_unused]] const std::size_t ret = FSE_buildCTable_wksp(
          tables_[ctx], ft_->norm_counts[ctx].data(), FreqTableT::MAX_SYMBOL,
          ft_->logs[ctx], wksp.data(),
          wksp.size() * sizeof(typename decltype(wksp)::value_type));
      assert(ret == 0);
    }
  }

  ~FSE_Encoder() {
    for (FSE_CTable *ct : tables_)
      FSE_freeCTable(ct);
  }

  /**
   * Init states and tie bitStream to dst;
   * dst should have been resized by the caller
   */
public:
  void startChunk(std::vector<std::byte> &dst) {
    [[maybe_unused]] auto ret =
        BIT_initCStream(&bitStream_, dst.data(), dst.size());
    assert(ret == 0);
    for (unsigned ctx = 0; ctx < FreqTableT::N_MODELS; ++ctx)
      FSE_initCState(states_.begin() + ctx, tables_[ctx]);
  }

  /** @return Resulting compressed size */
  std::size_t endChunk() {
    for (auto &state : states_)
      FSE_flushCState(&bitStream_, &state);
    return BIT_closeCStream(&bitStream_);
  }
};

/**
 * A class template for common FSE decoder methods
 * (currently these are ctor/dtor, and {start,end}Chunk)
 */
template <class FreqTableT> class FSE_Decoder {
protected:
  const FreqTableT *ft_;
  BIT_DStream_t bitStream_;

  template <typename T> using fse_array = FreqTableT::template fse_array<T>;
  fse_array<FSE_DState_t> states_;
  fse_array<FSE_DTable *> tables_;

  FSE_Decoder(const FreqTableT *ft) : ft_(ft) {
    std::vector<unsigned> wksp(
        FSE_BUILD_DTABLE_WKSP_SIZE_U32(ft->max_log, FreqTableT::MAX_SYMBOL));

    for (unsigned ctx = 0; ctx < FreqTableT::N_MODELS; ++ctx) {
      tables_[ctx] = FSE_createDTable(ft_->logs[ctx]);

      [[maybe_unused]] const std::size_t ret = FSE_buildDTable_wksp(
          tables_[ctx], ft_->norm_counts[ctx].data(), FreqTableT::MAX_SYMBOL,
          ft->logs[ctx], wksp.data(),
          wksp.size() * sizeof(decltype(wksp)::value_type));
      assert(ret == 0);
    }
  }

  ~FSE_Decoder() {
    for (FSE_DTable *dt : tables_)
      FSE_freeDTable(dt);
  }

public:
  void startChunk(std::vector<std::byte> &src) {
    [[maybe_unused]] auto ret =
        BIT_initDStream(&bitStream_, src.data(), src.size());
    assert(ret == src.size());
    /* init states in reverse flushing order */
    for (unsigned i = FreqTableT::N_MODELS; i > 0; --i) {
      const unsigned ctx = i - 1;
      FSE_initDState(states_.begin() + ctx, &bitStream_, tables_[ctx]);
    }
  }

  void endChunk() const { assert(BIT_endOfDStream(&bitStream_)); }
};

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
