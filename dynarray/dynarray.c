#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "dynarray.h"

/**
 * @private
 */
void *_safeCalloc(const size_t count, const size_t size) {
  void *rtn;
  if ((rtn = calloc(count, size)) == NULL && errno == ENOMEM) {
    EXIT_ERROR("Out of memory while allocating. Count: %lu, Size: %lu\n", count,
               size);
  }
  return rtn;
}

/**
 * @private
 */
void *_safeReallocarray(void *ptr, const size_t count, const size_t size) {
  void *rtn;
  if ((rtn = reallocarray(ptr, count, size)) == NULL && errno == ENOMEM) {
    EXIT_ERROR("Out of memory while reallocating. Count: %lu, Size: %lu\n",
               count, size);
  }
  return rtn;
}

/**
 * @private
 */
bool _extendCapacityDA(dynArray *pDA) {
  bool extended = false;
  if (pDA->size >= pDA->capacity) {

    if (pDA->capacity < 1) {
      pDA->capacity = pDA->size;
    }

    if (pDA->growth <= 1.0) {
      pDA->capacity = pDA->size;
    }

    while (pDA->size > pDA->capacity) {
      pDA->capacity *= pDA->growth;
    }
    pDA->array = _safeReallocarray(pDA->array, pDA->capacity, pDA->elementSize);
    extended = true;
  }
  return extended;
}

/**
 * @private
 */
void _swap(dynArray *pDA, void *a, void *b) {
  if (a != b) {
    memcpy(pDA->temp, a, pDA->elementSize);
    memcpy(a, b, pDA->elementSize);
    memcpy(b, pDA->temp, pDA->elementSize);
  }
}

/**
 * @private
 */
void *_toPtr(const dynArray *pDA, const size_t index) {
  return pDA->array + (index * pDA->elementSize);
}

/**
 * @private
 */
size_t _partition(dynArray *pDA, size_t low, size_t high,
                  int compare(const void *a, const void *b)) {
  void *pivot = _toPtr(pDA, high);
  void *pLow = _toPtr(pDA, low);
  size_t i = low;

  for (void *j = pLow; j < pivot; j += pDA->elementSize) {

    if (compare(j, pivot) < 0) {
      _swap(pDA, _toPtr(pDA, i), j);
      i++;
    }
  }
  _swap(pDA, _toPtr(pDA, i), pivot);
  return i;
}

/**
 * @private
 */
void _quickSort(dynArray *pDA, const size_t low, const size_t high,
                int compare(const void *a, const void *b)) {
  if (low < high) {

    size_t pi = _partition(pDA, low, high, compare);

    if (pi > 0) {
      _quickSort(pDA, low, pi - 1, compare);
    }
    _quickSort(pDA, low + 1, high, compare);
  }
}

/**
 * @private
 */
size_t _binarySearch(const dynArray *pDA,
                     int compare(const void *a, const void *b),
                     const void *value, const size_t min, const size_t max) {
  size_t found = -1;
  size_t diff = max - min;

  if (diff == 0) {
    if (compare(value, _toPtr(pDA, min)) == 0) {
      found = min;
    }
  } else {
    size_t mid = min + (diff / 2);
    int result = compare(value, _toPtr(pDA, mid));
    if (result == 0) {
      found = mid;
    } else if (result == -1) {
      found = _binarySearch(pDA, compare, value, min, mid);
    } else {
      found = _binarySearch(pDA, compare, value, mid + 1, max);
    }
  }

  return found;
}

size_t searchDA(dynArray *pDA, const void *value,
                int compare(const void *a, const void *b)) {
  size_t found = -1;
  if (pDA->size > 0) {
    sortDA(pDA, compare);
    found = _binarySearch(pDA, compare ? compare : pDA->compare, value, 0,
                          pDA->size - 1);
  }
  return found;
}

dynArray *createDA(const size_t elementSize,
                   int compare(const void *a, const void *b),
                   dynArrayParams *params) {
  dynArray *pDA;

  if (params == NULL) {
    params = &((dynArrayParams){.size = 0, .growth = 1.5, .capacity = 10});
  }

  if (params->growth <= 1.0) {
    params->growth = 1.5;
  }
  if (params->size >= params->capacity) {
    params->capacity = params->size;
  }
  pDA = _safeCalloc(1, sizeof(dynArray));
  pDA->capacity = params->capacity;
  pDA->growth = params->growth;
  pDA->size = params->size;
  pDA->elementSize = elementSize;
  pDA->compare = compare;
  pDA->temp = _safeCalloc(1, elementSize);
  pDA->array = _safeCalloc(pDA->capacity, elementSize);
  pDA->parent = NULL;
  return pDA;
}

void sortDA(dynArray *pDA, int compare(const void *a, const void *b)) {
  _quickSort(pDA, 0, pDA->size - 1, compare ? compare : pDA->compare);
}

bool addAllDA(dynArray *pDA, const void *src, const size_t length) {
  bool added = false;
  if (pDA->parent == NULL) {
    size_t lastIndex = pDA->size;
    pDA->size += length;

    _extendCapacityDA(pDA);

    void *dest = _toPtr(pDA, lastIndex);
    memcpy(dest, src, pDA->elementSize * length);

    added = true;
  }

  return added;
}

void *addDA(dynArray *pDA, const void *value) {
  void *rtn;
  if (pDA->parent == NULL) {
    addAllDA(pDA, value, 1);
    rtn = getDA(pDA, pDA->size - 1);
  } else {
    rtn = NULL;
  }
  return rtn;
}

bool setDA(dynArray *pDA, const size_t index, const void *value) {
  bool ok = true;

  if (index >= 0 && index < pDA->size) {
    memcpy(pDA->array + (index * pDA->elementSize), value, pDA->elementSize);
  } else {
    DEBUG_LOG("Index out of range: %ld, array size: %ld\n", index, pDA->size);
    ok = false;
  }

  return ok;
}

void *getDA(const dynArray *pDA, const size_t index) {

  void *entry = NULL;
  if (index >= 0 && index < pDA->size) {
    entry = pDA->array + (index * pDA->elementSize);
  } else {
    DEBUG_LOG("Index out of range: %ld, array size: %ld\n", index, pDA->size);
  }
  return entry;
}

void reverseDA(dynArray *pDA) {
  size_t half = pDA->size / 2;
  for (size_t i = 0; i < half; i++) {
    _swap(pDA, _toPtr(pDA, i), _toPtr(pDA, pDA->size - i - 1));
  }
}

void reduceMemDA(dynArray *pDA) {
  if (pDA && pDA->capacity > pDA->size) {
    pDA->capacity = pDA->size;
    pDA->array = _safeReallocarray(pDA->array, pDA->capacity, pDA->elementSize);
  }
}

dynArray *copyDA(const dynArray *pDA) {
  dynArray *copy =
      createDA(pDA->elementSize, pDA->compare,
               &(dynArrayParams){.size = pDA->size, .growth = pDA->growth});
  memcpy(copy->array, pDA->array, pDA->size * pDA->elementSize);
  return copy;
}

dynArray *subDA(dynArray *pDA, const size_t min, const size_t max) {
  dynArray *sub = NULL;
  if (max > min && max < pDA->size) {
    size_t subSize = max - min + 1;
    sub = _safeCalloc(1, sizeof(dynArray));
    sub->capacity = 0;
    sub->growth = 0;
    sub->size = subSize;
    sub->elementSize = pDA->elementSize;
    sub->temp = _safeCalloc(1, sub->elementSize);
    sub->array = _toPtr(pDA, min);
    sub->parent = pDA;
    sub->compare = pDA->compare;
  }

  return sub;
}

bool appendDA(dynArray *pDA, dynArray *pSrc) {
  bool appended = false;
  if (pDA->elementSize == pSrc->elementSize) {
    appended = addAllDA(pDA, pSrc->array, pSrc->size);
  }
  return appended;
}

void clearDA(dynArray *pDA) {
  if (pDA) {
    pDA->size = 0;
  }
}

void freeDA(dynArray *pDA) {
  if (pDA) {
    free(pDA->temp);
    if (pDA->parent == NULL) {
      free(pDA->array);
    }
    free(pDA);
  }
}

int compareString(const void *a, const void *b) { return strcmp(a, b); }