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
static inline uint32_t _rotateLeft(const uint32_t x, const uint8_t bits) {
  return (x << bits) | (x >> (32 - bits));
}

/**
 * @private
 */
static inline void _process(const void *data, uint32_t *state0,
                            uint32_t *state1, uint32_t *state2,
                            uint32_t *state3) {
  const uint32_t *block = (const uint32_t *)data;
  *state0 = _rotateLeft(*state0 + block[0] * Prime2, 13) * Prime1;
  *state1 = _rotateLeft(*state1 + block[1] * Prime2, 13) * Prime1;
  *state2 = _rotateLeft(*state2 + block[2] * Prime2, 13) * Prime1;
  *state3 = _rotateLeft(*state3 + block[3] * Prime2, 13) * Prime1;
}

uint32_t hash(const void *input, const size_t length, const uint32_t seed) {

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
    _process(data, &state0, &state1, &state2, &state3);
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

uint32_t hashKey(const keyEntry *kEntry, const uint32_t seed) {
  return hash(kEntry->key, kEntry->length, seed);
}

/**
 * @private
 */
static inline int _compareHashElement(const hashTree *pHD,
                                      const hashEntry *entry,
                                      const hashEntry *node) {

  int comp = entry->hash < node->hash ? -1 : entry->hash > node->hash ? 1 : 0;

  if (comp == 0) {
    comp = pHD->da->compare(entry->kEntry->key, node->kEntry->key);
  }

  return comp;
}

/**
 * @private
 */
static inline hashEntry *_getIndexNodeHT(const hashTree *pHT,
                                         const size_t nodeIndex) {
  return getDA(pHT->da, nodeIndex);
}

/**
 * @private
 */
static inline size_t _getRootIndexHT(const hashTree *pHT) { return pHT->root; }

/**
 * @private
 */
static inline void _setRootIndexHT(hashTree *pHT, const size_t root) {
  pHT->root = root;

  if (pHT->da->fp) {
    char buffer[FILE_BUFFER];
    *((size_t *)buffer) = pHT->root;
    saveHeaderBufferDA(pHT->da, buffer);
  }
}

/**
 * @private
 */
static inline hashEntry *_getRootNodeHT(const hashTree *pHT,
                                        const size_t nodeIndex) {
  return _getIndexNodeHT(pHT, _getRootIndexHT(pHT));
}

/**
 * @private
 */
void _drawNode(const hashTree *pHT, const size_t nodeIdx, const char *topPrefix,
               const char *botPrefix, FILE *file) {
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

  fprintf(file, "%s+%02lu[%02lu] %s [%u]\n", topPrefix, nodeIdx, entry->parent,
          (char *)entry->kEntry->key, entry->hash);
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

/**
 * @private
 */
void _addToNodeHT(hashTree *pHT, hashEntry *entry, const size_t nodeIndex) {
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

/**
 * @private
 */
hashEntry *_findNodeHT(const hashTree *pHT, const hashEntry *entry,
                       const size_t nodeIndex) {
  hashEntry *node = NULL;

  if (nodeIndex != -1) {
    node = _getIndexNodeHT(pHT, nodeIndex);
    int comp = _compareHashElement(pHT, entry, node);

    if (comp < 0) {
      node = (node->left != -1) ? _findNodeHT(pHT, entry, node->left) : NULL;
    } else if (comp > 0) {
      node = (node->right != -1) ? _findNodeHT(pHT, entry, node->right) : NULL;
    }
    // key matches node so return node
  }

  return node;
}

/**
 * @private
 */
size_t _findNodeIndexHT(const hashTree *pHT, const hashEntry *entry,
                        const size_t nodeIndex) {
  size_t found = -1;
  hashEntry *node = _getIndexNodeHT(pHT, nodeIndex);
  int comp = _compareHashElement(pHT, entry, node);

  if (comp == 0) {
    found = nodeIndex;
  } else if (comp < 0) {
    found = (node->left != -1) ? _findNodeIndexHT(pHT, entry, node->left) : -1;
  } else if (comp > 0) {
    found =
        (node->right != -1) ? _findNodeIndexHT(pHT, entry, node->right) : -1;
  }
  // key matches node so return node

  return found;
}

/**
 * @private
 */
void _balanceNodeHT(hashTree *pHT, const size_t nodeIndex, const int side) {
  hashEntry *node = _getIndexNodeHT(pHT, nodeIndex);

  if (node->left != -1) {
    _balanceNodeHT(pHT, node->left, -1);
  }

  if (node->right != -1) {
    _balanceNodeHT(pHT, node->right, 1);
  }

  unsigned int depthLeft = (node->left != -1) ? maxDepthHT(pHT, node->left) : 0;
  unsigned int depthRight =
      (node->right != -1) ? maxDepthHT(pHT, node->right) : 0;
  int diff = depthLeft - depthRight;

  if (diff < -1 || diff > 1) {

    size_t childIdx;
    hashEntry *childNode;

    if (diff < -1) {
      // need to rotate left
      childIdx = node->right;
      childNode = _getIndexNodeHT(pHT, childIdx);

      // rotate the nodes
      node->right = childNode->left;
      childNode->left = nodeIndex;
      _getIndexNodeHT(pHT, node->right)->parent = nodeIndex;

    } else if (diff > 1) {
      // need to rotate right
      childIdx = node->left;
      childNode = _getIndexNodeHT(pHT, childIdx);

      // rotate the nodes
      node->left = childNode->right;
      childNode->right = nodeIndex;
      _getIndexNodeHT(pHT, node->left)->parent = nodeIndex;
    }

    if (_getRootIndexHT(pHT) != nodeIndex) {
      // if not root node
      size_t parentIdx = node->parent;

      // update the parent link
      childNode->parent = node->parent;
      if (side < 0) {
        _getIndexNodeHT(pHT, parentIdx)->left = childIdx;
      } else if (side > 0) {
        _getIndexNodeHT(pHT, parentIdx)->right = childIdx;
      }
    } else {
      // if root node
      _setRootIndexHT(pHT, childIdx);

      // update the parent link
      childNode->parent = childIdx;
    }

    node->parent = childIdx;
  }
}

/**
 * @private
 */
bool _visitNodeHT(const hashTree *pHT, const size_t nodeIndex,
                  bool visit(const hashEntry *entry, const size_t nodeIndex,
                             void *ref),
                  void *ref) {
  bool cont = true;
  if (nodeIndex != -1) {
    hashEntry *node = _getIndexNodeHT(pHT, nodeIndex);

    if (node->left != -1) {
      cont = _visitNodeHT(pHT, node->left, visit, ref);
    }
    if (node->right != -1 && cont) {
      cont = _visitNodeHT(pHT, node->right, visit, ref);
    }

    if (cont) {
      cont = visit(node, nodeIndex, ref);
    }
  }

  return cont;
}

/**
 * @private
 */
void _insertNodeHT(hashTree *pHT, hashEntry *entry, const size_t entryIndex,
                   const size_t nodeIndex) {

  hashEntry *node = _getIndexNodeHT(pHT, nodeIndex);
  int comp = _compareHashElement(pHT, entry, node);

  if (comp < 0) {
    if (node->left == -1) {
      node->left = entryIndex;
      entry->parent = nodeIndex;
    } else {
      _insertNodeHT(pHT, entry, entryIndex, node->left);
    }
  } else if (comp > 0) {
    if (node->right == -1) {
      node->right = entryIndex;
      entry->parent = nodeIndex;
    } else {
      _insertNodeHT(pHT, entry, entryIndex, node->right);
    }
  }
}

/**
 * @private
 */
void _reinsertHT(hashTree *pHT, const size_t entryIndex) {
  if (entryIndex != -1) {
    hashEntry *entry = _getIndexNodeHT(pHT, entryIndex);
    _insertNodeHT(pHT, entry, entryIndex, _getRootIndexHT(pHT));
  }
}

/**
 * @private
 */
void _deleteHT(hashTree *pHT, hashEntry *entry, const size_t nodeIndex,
               void deleted(const hashTree *pHT, const keyEntry *kEntry,
                            void *value, void *ref),
               void *ref) {

  size_t found = _findNodeIndexHT(pHT, entry, _getRootIndexHT(pHT));

  if (found != -1) {
    hashEntry *delNode = _getIndexNodeHT(pHT, found);

    if (_getRootIndexHT(pHT) != found) {
      hashEntry *parent = _getIndexNodeHT(pHT, delNode->parent);

      parent->left = (parent->left == found) ? -1 : parent->left;
      parent->right = (parent->right == found) ? -1 : parent->right;
    } else {
      _setRootIndexHT(pHT,
                      (delNode->left != -1) ? delNode->left : delNode->right);
    }

    _reinsertHT(pHT, delNode->left);
    _reinsertHT(pHT, delNode->right);

    if (deleted) {
      deleted(pHT, delNode->kEntry, delNode->value, ref);
    }

    size_t lastIndex = pHT->da->size - 1;
    if (lastIndex > 0) {
      hashEntry *lastNode = _getIndexNodeHT(pHT, lastIndex);
      hashEntry *lastParent = _getIndexNodeHT(pHT, lastNode->parent);
      memcpy(delNode, lastNode, pHT->da->elementSize);
      lastParent->left =
          (lastParent->left == lastIndex) ? found : lastParent->left;
      lastParent->right =
          (lastParent->right == lastIndex) ? found : lastParent->right;
      if (_getRootIndexHT(pHT) == lastIndex) {
        _setRootIndexHT(pHT, found);
      }
      hashEntry *foundNode = _getIndexNodeHT(pHT, found);
      if (foundNode->left != -1) {
        _getIndexNodeHT(pHT, foundNode->left)->parent = found;
      }
      if (foundNode->right != -1) {
        _getIndexNodeHT(pHT, foundNode->right)->parent = found;
      }
    }
    pHT->da->size--;
  }
}

/////////////////////////////////
// Exposed methods
/////////////////////////////////

void retainAllHT(hashTree *pHT, hashTree *pOther) {
  size_t limit = pHT->da->size, cnt = 0;
  const keyEntry *orphans[limit];

  for (size_t i = 0; i < limit; i++) {
    const keyEntry *kEntry = _getIndexNodeHT(pHT, i)->kEntry;
    if (!hasEntryHT(pOther, kEntry)) {
      orphans[cnt++] = kEntry;
    }
    orphans[cnt] = NULL;
  }

  for (size_t i = 0; i < limit && orphans[i] != NULL; i++) {
    deleteHT(pHT, orphans[i]);
  }
}

void deleteHT(hashTree *pHT, const keyEntry *kEntry) {
  deleteCallbackHT(pHT, kEntry, NULL, NULL);
}

void deleteCallbackHT(hashTree *pHT, const keyEntry *kEntry,
                      void deleted(const hashTree *pHT, const keyEntry *kEntry,
                                   void *value, void *ref),
                      void *ref) {
  hashEntry entry = (hashEntry){.kEntry = kEntry,
                                .value = NULL,
                                .hash = hashKey(kEntry, 0),
                                .left = -1,
                                .right = -1};
  _deleteHT(pHT, &entry, _getRootIndexHT(pHT), deleted, ref);
}

unsigned int maxDepthHT(const hashTree *pHT, const size_t nodeIndex) {
  unsigned int left = 0, right = 0, depth = 0;

  if (nodeIndex < pHT->da->size) {
    hashEntry *entry = _getIndexNodeHT(pHT, nodeIndex);
    if (entry->left != -1) {
      left = maxDepthHT(pHT, entry->left);
    }
    if (entry->right != -1) {
      right = maxDepthHT(pHT, entry->right);
    }

    depth = 1 + ((left > right) ? left : right);
  }

  return depth;
}

void balanceHT(hashTree *pHT) { _balanceNodeHT(pHT, _getRootIndexHT(pHT), 0); }

hashEntry *getHT(const hashTree *pHT, const keyEntry *kEntry) {
  hashEntry entry = (hashEntry){.kEntry = kEntry,
                                .value = NULL,
                                .hash = hashKey(kEntry, 0),
                                .left = -1,
                                .right = -1};

  return _findNodeHT(pHT, &entry, _getRootIndexHT(pHT));
}

void visitNodesHT(const hashTree *pHT,
                  bool visit(const hashEntry *entry, const size_t entryIndex,
                             void *ref),
                  void *ref) {
  visitSubTreeHT(pHT, _getRootIndexHT(pHT), visit, ref);
}

void visitSubTreeHT(const hashTree *pHT, size_t index,
                    bool visit(const hashEntry *entry, const size_t entryIndex,
                               void *ref),
                    void *ref) {
  _visitNodeHT(pHT, index, visit, ref);
}

hashTree *createHT(int compare(const void *a, const void *b),
                   hashTreeParams *params) {
  hashTree *pHT = _safeCalloc(1, sizeof(hashTree));

  if (params == NULL) {
    params = &((hashTreeParams){.growth = 1.5, .capacity = 10});
  }

  dynArrayParams daParams = (dynArrayParams){.size = 0,
                                             .growth = params->growth,
                                             .capacity = params->capacity,
                                             .filename = params->filename};

  pHT->da = createDA(sizeof(hashEntry), compare, &daParams);
  _setRootIndexHT(pHT, -1);
  return pHT;
}

hashTree *copyHT(hashTree *pHT) {
  hashTree *pOther = _safeCalloc(1, sizeof(hashTree));
  memcpy(pOther, pHT, sizeof(hashTree));
  pOther->da = copyDA(pHT->da);
  return pOther;
}

void setHT(hashTree *pHT, const keyEntry *kEntry, void *value) {
  hashEntry entry = (hashEntry){.kEntry = kEntry,
                                .value = value,
                                .hash = hashKey(kEntry, 0),
                                .left = -1,
                                .right = -1};

  if (pHT->da->size == 0) {
    //  if size zero then add to root
    addDA(pHT->da, &entry);
    // set the root pointer
    _setRootIndexHT(pHT, pHT->da->size - 1);
    // update the parent index pointer to point to itself
    ((hashEntry *)getDA(pHT->da, _getRootIndexHT(pHT)))->parent =
        _getRootIndexHT(pHT);
  } else {
    // update the root node
    _addToNodeHT(pHT, &entry, _getRootIndexHT(pHT));
  }
}

void setAllHT(hashTree *pHT, const hashTree *pOther) {
  size_t limit = pOther->da->size;
  for (size_t i = 0; i < limit; i++) {
    hashEntry *other = _getIndexNodeHT(pOther, i);
    setHT(pHT, other->kEntry, other->value);
  }
}

bool hasEntryHT(const hashTree *pHT, const keyEntry *kEntry) {
  return getHT(pHT, kEntry) != NULL;
}

bool hasAllHT(const hashTree *pHT, const hashTree *pOther) {
  bool has = pHT != NULL && pOther != NULL && pHT->da->size >= pOther->da->size;

  size_t limit = pOther->da->size;
  for (size_t i = 0; has && i < limit; i++) {
    has = hasEntryHT(pHT, _getIndexNodeHT(pOther, i)->kEntry);
  }

  return has;
}

void clearHT(hashTree *pHT) {
  if (pHT) {
    clearDA(pHT->da);
    _setRootIndexHT(pHT, -1);
  }
}

void drawNode(const hashTree *pHT, const size_t nodeIdx, FILE *file) {
  if (file == NULL) {
    file = stdout;
  }
  if (pHT->da->size > 0 && _getRootIndexHT(pHT) != -1) {
    fprintf(file, "\n");
    _drawNode(pHT, _getRootIndexHT(pHT), "", "", file);
  }
  fprintf(file, "\n");
}

void drawTree(const hashTree *pHT, FILE *file) {
  drawNode(pHT, _getRootIndexHT(pHT), file);
}

hashTree *loadHT(const char *filename,
                 int compare(const void *a, const void *b)) {

  dynArray *pDA = loadDA(filename, compare);

  hashTree *pHT = _safeCalloc(1, sizeof(hashTree));
  pHT->da=pDA;
  
  fileHeader header;
  readHeaderDA(pDA, &header);
  
  pHT->root = *(size_t *)(header.buffer);

  return pHT;
}

void freeHT(hashTree *pHT) {
  if (pHT) {
    freeDA(pHT->da);
    free(pHT);
  }
}
