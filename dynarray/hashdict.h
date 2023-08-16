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

uint32_t hash(const void *input, size_t length, uint32_t seed);

#endif // HASHDICT_H
