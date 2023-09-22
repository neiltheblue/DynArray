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
 * - generate a set from a hash tree
 * -- add has all function for array
 * -- add add all function for array
 * -- add get all function for tree
 * -- add delete all function for array
 * -- add retain all function for tree
 * -- add union function for tree
 */

/**
 * @brief A key entity
 */
typedef struct KeyEntry {
  const void *key; ///< the key
  size_t length;   ///< the key length
} keyEntry;

/**
 * @brief A key/value entity
 */
typedef struct HashEntry {
  const uint32_t hash;    ///< the hash
  const keyEntry *kEntry; ///< the key
  size_t parent;          ///< the parent node
  size_t left;            ///< the smaller left node
  size_t right;           ///< the larger right node
  void *value;            ///< the value
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
 * @brief Generate a hash value for a key entry
 *
 * This is an implementaion of xxHash. The seed can be 0, as it is just used to
 * provide a predictable result, which can be used for collision avoidance.
 *
 * @param kEntry the key entry
 * @param seed the hash starting seed
 * @return the hash value
 */
uint32_t hashKey(const keyEntry *kEntry, const uint32_t seed);

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
 * @param kEntry the key entry pointer
 * @param value the value pointer
 */
void addHT(hashTree *pHT, const keyEntry *kEntry, void *value);

/**
 * @brief Get the tree depth of the sub-tree
 * @param pHT the hash tree pointer
 * @param nodeIndex the node index to count from
 * @return the max tree depth from the node
 */
unsigned int maxDepthHT(const hashTree *pHT, const size_t nodeIndex);

/**
 * @brief Draw the sub node
 * @param pHT the hash tree pointer
 * @param file the stream to write to or stdout if NULL
 * @param nodeIdx the node index to draw from
 */
void drawNode(const hashTree *pHT, const size_t nodeIdx, FILE *file);

/**
 * @brief Draw the tree
 * @param pHT the hash tree pointer
 * @param file the stream to write to or stdout if NULL
 */
void drawTree(const hashTree *pHT, FILE *file);

/**
 * @brief Find a node in the tree
 * @param pHT the hash tree pointer to search
 * @param kEntry the key entry
 * @return the found entry or NULL if not found
 */
hashEntry *getHT(const hashTree *pHT, const keyEntry *kEntry);

/**
 * @brief Balance the tree
 * @param pHT the has tree pointer to balance
 */
void balanceHT(hashTree *pHT);

/**
 * @brief Vist each node in the tree in a depth first path, from left to right
 *
 * If the visitor method returns false then the
 * tree traversal will stop.
 *
 * @param pHT the hash tree pointer to visit
 * @param visit the function to call for each node
 * @param ref optional value to pass to visit method, maybe NULL
 */
void visitNodesHT(const hashTree *pHT,
                  bool visit(const hashEntry *entry, const size_t entryIndex,
                             void *ref),
                  void *ref);

/**
 * @brief Delete a node from the tree
 * @param pHT the hash tree pointer to delete from
 * @param kEntry the key entry to delete
 */
void deleteHT(hashTree *pHT, const keyEntry *kEntry);

/**
 * @brief Delete a node from the tree
 * @param pHT the hash tree pointer to delete from
 * @param kEntry the key entry to delete
 * @param deleted the method call back when an item is deleted, may be NULL
 * @param ref the reference to pass to the deleted function
 */
void deleteCallbackHT(hashTree *pHT, const keyEntry *kEntry,
                      void deleted(const hashTree *pHT, const keyEntry *kEntry,
                                   void *value, void *ref),
                      void *ref);

/**
 * @brief Clear the contents of the hash tree.
 * @param pHT the hash tree pointer to clear
 */
void clearHT(hashTree *pHT);

/**
 * @brief Free a hash tree
 * @param pHT the hash tree to free
 */
void freeHT(hashTree *pHT);

#endif