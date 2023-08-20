#ifndef HASHTREE_H
#define HASHTREE_H

#include "dynarray.h"
#include <stdint.h>

/**
 * @file dynarray.h
 *
 * @brief Dynamic Array header file
 *
 * TODO:
 * - Add key/values
 * - Add tree search
 * - balance tree
 */

/**
 * @brief A key/value entity
 */
typedef struct HashEntry {
  uint32_t hash; ///< the hash
  void *key;     ///< the key
  void *value;   ///< the value
  size_t parent; ///< the parent node
  size_t left;   ///< the smaller left node
  size_t right;  ///< the larger right node
} hashEntry;

/**
 * @brief Hash tree entity
 */
typedef struct HashTree {
  dynArray *da; ///< the storage array
  size_t root;  ///< the root node
} hashTree;

/**
 * @brief Hash tree creation parameters
 */
typedef struct HashTreeParams {
  float growth;    ///< the growth factor for the tree
  size_t capacity; ///< the initial reserved capacity for the entries
} hashTreeParams;

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

/**
 * @brief Create a new hash tree
 *
 * @param compare the key comparator function
 * @param params a pointer to the hash tree parameters or NULL for default
 * @return An initialised hash tree that
 *          should be freed with freeHT()
 */
hashTree *createHT(int compare(const void *a, const void *b),
                   hashTreeParams *params);

/**
 * @brief Add a key value pair to the tree
 * @param pHT the hash tree pointer
 * @param key the key pointer
 * @param keyLength the key length
 * @param value the value pointer
 */
void addHT(hashTree *pHT, void *key, size_t keyLength, void *value);

/**
 * @brief Free a hash tree
 * @param pHT the hash tree to free
 */
void freeHT(hashTree *pHT);

#endif