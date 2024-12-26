#pragma once
#include "defs.h"
#include "entropy_common.h"
#include <array>

namespace fqzcomp28 {

class SequenceCoder {
public:
  constexpr static int MAX_SYMBOL = 0b11; // 'T'
  constexpr static int ALPHABET_SIZE = MAX_SYMBOL + 1;
  // TODO: use initial ctx value from fqzcomp4
  constexpr static unsigned INITIAL_CONTEXT = 0;
  constexpr static int CONTEXT_SIZE = 1;
  constexpr static unsigned CTX_MASK = (1 << (CONTEXT_SIZE * 2)) - 1;
  constexpr static unsigned N_MODELS = 1 << (2 * CONTEXT_SIZE);

private:
  /** stores some object (e.g., counts) for each model */
  template <typename T> using fse_array = std::array<T, N_MODELS>;

public:
  /** normalized to sum to power of 2**log */
  struct FreqTable {
    fse_array<std::array<short, ALPHABET_SIZE>> norm_counts;
    fse_array<unsigned> logs;
    unsigned max_log;

    bool operator==(const FreqTable &other) const = default;
  };
  static FreqTable calculateFreqTable(const FastqChunk &chunk);

  // TODO: initialize different fields depending on whether it's compressed or
  // decomrpession (or split into 2 classes)
  SequenceCoder(const FreqTable *ft) : ft_(ft) {
    auto wksp = createCTableBuildWksp(MAX_SYMBOL, ft_->max_log);

    for (unsigned ctx = 0; ctx < N_MODELS; ++ctx) {
      ctables_[ctx] = FSE_createCTable(MAX_SYMBOL, ft_->logs[ctx]);

      [[maybe_unused]] std::size_t ret = FSE_buildCTable_wksp(
          ctables_[ctx], ft_->norm_counts[ctx].data(), MAX_SYMBOL,
          ft->logs[ctx], wksp.data(), wksp.size());
      assert(ret == 0);
    }
  }

  ~SequenceCoder() {
    for (FSE_CTable *ct : ctables_)
      FSE_freeCTable(ct);
  }

  void startChunk(std::vector<std::byte> &dst) {
    [[maybe_unused]] auto ret =
        BIT_initCStream(&bitStream_, dst.data(), dst.size());
    assert(ret == 0);

    for (unsigned ctx = 0; ctx < N_MODELS; ++ctx)
      FSE_initCState(states_.begin() + ctx, ctables_[ctx]);
  }

private:
  const FreqTable *ft_;
  BIT_CStream_t bitStream_;
  fse_array<FSE_CState_t> states_;
  fse_array<FSE_CTable *> ctables_;
};

} // namespace fqzcomp28
