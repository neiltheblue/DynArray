#include "hashdict.h"

#include <stdint.h>

const uint32_t Prime1 = 2654435761U;
const uint32_t Prime2 = 2246822519U;
const uint32_t Prime3 = 3266489917U;
const uint32_t Prime4 = 668265263U;
const uint32_t Prime5 = 374761393U;
const uint32_t MaxBufferSize = 15 + 1;

/**
 * @private
 */
static inline uint32_t _rotateLeft(uint32_t x, uint8_t bits) {
  return (x << bits) | (x >> (32 - bits));
}

/**
 * @private
 */
static inline void _process(const void *data, uint32_t state0, uint32_t state1,
                            uint32_t state2, uint32_t state3) {
  const uint32_t *block = (const uint32_t *)data;
  state0 = _rotateLeft(state0 + block[0] * Prime2, 13) * Prime1;
  state1 = _rotateLeft(state1 + block[1] * Prime2, 13) * Prime1;
  state2 = _rotateLeft(state2 + block[2] * Prime2, 13) * Prime1;
  state3 = _rotateLeft(state3 + block[3] * Prime2, 13) * Prime1;
}

uint32_t hash(const void *input, size_t length, uint32_t seed) {

  const uint8_t *data = (uint8_t *)input;
  uint32_t state0 = seed + Prime1 + Prime2;
  uint32_t state1 = seed + Prime2;
  uint32_t state2 = seed;
  uint32_t state3 = seed - Prime1;
  uint32_t result = (uint32_t)length;

  // point beyond last byte
  const uint8_t *stop = data + length;
  const uint8_t *stopBlock = stop - MaxBufferSize;

  while (data <= stopBlock) {
    _process(data, state0, state1, state2, state3);
    data += 16;
  }

  // fold 128 bit state into one single 32 bit value
  if (length >= MaxBufferSize) {
    result += _rotateLeft(state0, 1) + _rotateLeft(state1, 7) +
              _rotateLeft(state2, 12) + _rotateLeft(state3, 18);
  } else {
    result += state2 + Prime5;
  }

  // at least 4 bytes left ? => eat 4 bytes per step
  while (data + 4 <= stop) {
    result = _rotateLeft(result + *(uint32_t *)data * Prime3, 17) * Prime4;
    data += 4;
  }

  // take care of remaining 0..3 bytes, eat 1 byte per step
  while (data != stop) {
    result = _rotateLeft(result + (*data++) * Prime5, 11) * Prime1;
  }

  // mix bits
  result ^= result >> 15;
  result *= Prime2;
  result ^= result >> 13;
  result *= Prime3;
  result ^= result >> 16;
  return result;
}

///**
// * @private
// */
//uint32_t _hash(const void *input, size_t length, uint32_t seed) {
//
//  return _bufferAdd(input, length, seed);
//}