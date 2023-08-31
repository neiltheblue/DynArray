#include "hashtree.h"
#include <stdlib.h>
#include <string.h>

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

/**
 * @private
 */
static inline int _compareHashElement(hashTree *pHD, hashEntry *entry,
                                      hashEntry *node) {

  int comp = entry->hash < node->hash ? -1 : entry->hash > node->hash ? 1 : 0;

  if (comp == 0) {
    comp = pHD->da->compare(entry->key, node->key);
  }

  return comp;
}

/**
 * @private
 */
static inline hashEntry *_getIndexNodeHT(hashTree *pHT, size_t nodeIndex) {
  return getDA(pHT->da, nodeIndex);
}

/**
 * @private
 */
static inline hashEntry *_getRootNodeHT(hashTree *pHT, size_t nodeIndex) {
  return _getIndexNodeHT(pHT, pHT->root);
}

/**
 * @private
 */
void _drawNode(hashTree *pHT, size_t nodeIdx, char *topPrefix, char *botPrefix,
               FILE *file) {
  char nextTopPrefix[strlen(topPrefix) + 3];
  char nextBotPrefix[strlen(botPrefix) + 3];
  hashEntry *entry = getDA(pHT->da, nodeIdx);

  if (entry->right != -1) {
    strcpy(nextTopPrefix, topPrefix);
    strcat(nextTopPrefix, "   ");
    strcpy(nextBotPrefix, topPrefix);
    strcat(nextBotPrefix, "  |");
    _drawNode(pHT, entry->right, nextTopPrefix, nextBotPrefix, file);
  } else {
    fprintf(file, "%s   .\n", topPrefix);
  }
  fprintf(file, "%s  /\n", topPrefix);

  fprintf(file, "%s+%02lu %s [%u]\n", topPrefix, nodeIdx, (char *)entry->key,
          entry->hash);
  fprintf(file, "%s  \\\n", botPrefix);

  if (entry->left != -1) {
    strcpy(nextTopPrefix, botPrefix);
    strcat(nextTopPrefix, "  |");
    strcpy(nextBotPrefix, botPrefix);
    strcat(nextBotPrefix, "   ");
    _drawNode(pHT, entry->left, nextTopPrefix, nextBotPrefix, file);
  } else {
    fprintf(file, "%s   .\n", botPrefix);
  }
}

void drawNode(hashTree *pHT, size_t nodeIdx, FILE *file) {
  if (file == NULL) {
    file = stdout;
  }
  fprintf(file, "\n");
  _drawNode(pHT, 0, "", "", file);
  fprintf(file, "\n");
}

int maxDepthHT(hashTree *pHT, size_t nodeIndex) {
  hashEntry *entry = _getIndexNodeHT(pHT, nodeIndex);
  int depth = 1;
  if (entry->left != -1) {
    depth += maxDepthHT(pHT, entry->left);
  }
  if (entry->right != -1) {
    depth += maxDepthHT(pHT, entry->right);
  }
  return depth;
}

/**
 * @private
 */
void _addToNodeHT(hashTree *pHT, hashEntry *entry, size_t nodeIndex) {
  hashEntry *node = _getIndexNodeHT(pHT, nodeIndex);
  int comp = _compareHashElement(pHT, entry, node);

  if (comp == 0) {
    // key matches node so replace value
    node->value = entry->value;
  } else if (comp < 0 && node->left == -1) {
    // add left node
    entry = addDA(pHT->da, entry);
    entry->parent = nodeIndex;
    // get new node as may have reallocated
    node = _getIndexNodeHT(pHT, nodeIndex);
    node->left = pHT->da->size - 1;
  } else if (comp > 0 && node->right == -1) {
    // add right node
    entry = addDA(pHT->da, entry);
    entry->parent = nodeIndex;
    // get new node as may have reallocated
    node = _getIndexNodeHT(pHT, nodeIndex);
    node->right = pHT->da->size - 1;
  } else if (comp < 0) {
    // handle left node addition
    _addToNodeHT(pHT, entry, node->left);
  } else {
    // handle right node addition
    _addToNodeHT(pHT, entry, node->right);
  }
}

hashEntry *_findNodeHT(hashTree *pHT, hashEntry *entry, size_t nodeIndex) {
  hashEntry *node = _getIndexNodeHT(pHT, nodeIndex);
  int comp = _compareHashElement(pHT, entry, node);

  if (comp < 0) {
    node = (node->left != -1) ? _findNodeHT(pHT, entry, node->left) : NULL;
  } else if (comp > 0) {
    node = (node->right != -1) ? _findNodeHT(pHT, entry, node->right) : NULL;
  }
  // key matches node so return node

  return node;
}

hashEntry *getHT(hashTree *pHT, void *key, size_t keyLength) {
  hashEntry entry = (hashEntry){.key = key,
                                .value = NULL,
                                .hash = hash(key, keyLength, 0),
                                .left = -1,
                                .right = -1};

  return _findNodeHT(pHT, &entry, pHT->root);
}

hashTree *createHT(int compare(const void *a, const void *b),
                   hashTreeParams *params) {

  hashTree *pHT = _safeCalloc(1, sizeof(hashTree));

  if (params == NULL) {
    params = &((hashTreeParams){.growth = 1.5, .capacity = 10});
  }

  dynArrayParams daParams = (dynArrayParams){
      .size = 0, .growth = params->growth, .capacity = params->capacity};

  pHT->da = createDA(sizeof(hashEntry), compare, &daParams);
  return pHT;
}

void addHT(hashTree *pHT, void *key, size_t keyLength, void *value) {
  hashEntry entry = (hashEntry){.key = key,
                                .value = value,
                                .hash = hash(key, keyLength, 0),
                                .left = -1,
                                .right = -1};

  if (pHT->da->size == 0) {
    //  if size zero then add to root
    addDA(pHT->da, &entry);
    // set the root pointer
    pHT->root = pHT->da->size - 1;
    // update the parent index pointer to point to itself
    ((hashEntry *)getDA(pHT->da, pHT->root))->parent = pHT->root;
  } else {
    // update the root node
    _addToNodeHT(pHT, &entry, pHT->root);
  }
}

void freeHT(hashTree *pHT) {
  if (pHT) {
    freeDA(pHT->da);
    free(pHT);
  }
}
