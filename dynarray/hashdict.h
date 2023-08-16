#ifndef HASHDICT_H
#define HASHDICT_H

/**
 * @file dynarray.h
 *
 * @brief Dynamic Array header file
 *
 * TODO list:
 *
 * - build dictionary
 *
 */

#include "dynarray.h"
#include <stdint.h>

/**
 * @brief A key/value entity
 */
typedef struct KeyEntry {
  int hash;    ///< The hash
  void *key;   ///< The key
  void *entry; ///< The entry
} keyEntry;

/**
 * @brief Generate a hash value for a byte array
 *
 * This is an implementaion of xxHash. The seed can be 0, as it is just used to
 * provide a predictable result, which can be used for collision avoidance.
 *
 * @param input the byte array to hash
 * @param length the input langth
 * @param seed the hash starting seed
 * @return the hash value
 */
uint32_t hash(const void *input, size_t length, uint32_t seed);

#endif // HASHDICT_H
