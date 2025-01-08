#pragma once
#include <cassert>

namespace fqcomp28 {

// TODO: use table
constexpr inline unsigned base2bits(const char base) {
  switch (base) {
  case 'A':
    return 0b00;
  case 'C':
    return 0b01;
  case 'G':
    return 0b10;
  case 'T':
    return 0b11;
  default:
    assert(false);
    __builtin_unreachable();
  }
}

constexpr inline char bits2base(const unsigned bits) {
  switch (bits) {
  case 0b00:
    return 'A';
  case 0b01:
    return 'C';
  case 0b10:
    return 'G';
  case 0b11:
    return 'T';
  default:
    assert(false);
    __builtin_unreachable();
  }
}

} // namespace fqcomp28
