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
void *_createDynArray(DynArrayParams params, size_t structSize,
                      size_t elementSize) {
  _dynArrayAny *pADA;

  if (params.capacity < 2) {
    params.capacity = 2;
  }
  if (params.growth <= 1.0) {
    params.growth = 1.5;
  }
  if (params.size < 0) {
    params.size = 0;
  }
  if (params.size >= params.capacity) {
    params.capacity = params.size;
  }
  pADA = _safeCalloc(1, structSize);
  pADA->capacity = params.capacity;
  pADA->growth = params.growth;
  pADA->size = params.size;
  pADA->elementSize = elementSize;
  pADA->temp = _safeCalloc(1, elementSize);
  pADA->array = _safeCalloc(pADA->capacity, elementSize);
  return pADA;
}

/**
 * @private
 */
void _extendCapacity(_dynArrayAny *pADA) {
  if (pADA->size >= pADA->capacity) {
    while (pADA->size >= pADA->capacity) {
      pADA->capacity *= pADA->growth;
    }
    pADA->array =
        _safeReallocarray(pADA->array, pADA->capacity, pADA->elementSize);
  }
}

/**
 * @private
 */
void _swap(_dynArrayAny *pADA, void *a, void *b) {
  if (a != b) {
    memcpy(pADA->temp, a, pADA->elementSize);
    memcpy(a, b, pADA->elementSize);
    memcpy(b, pADA->temp, pADA->elementSize);
  }
}

/**
 * @private
 */
void *_toPtr(_dynArrayAny *pADA, size_t index) {
  return pADA->array + (index * pADA->elementSize);
}

/**
 * @private
 */
size_t _partition(_dynArrayAny *pADA, size_t low, size_t high,
                  int cmp(void *a, void *b)) {
  void *pivot = _toPtr(pADA, high);
  void *pLow = _toPtr(pADA, low);
  size_t i = low;

  for (void *j = pLow; j < pivot; j += pADA->elementSize) {

    if (cmp(j, pivot) < 0) {
      _swap(pADA, _toPtr(pADA, i), j);
      i++;
    }
  }
  _swap(pADA, _toPtr(pADA, i), pivot);
  return i;
}

/**
 * @private
 */
void _quickSort(_dynArrayAny *pADA, size_t low, size_t high,
                int cmp(void *a, void *b)) {
  if (low < high) {

    size_t pi = _partition(pADA, low, high, cmp);

    if (pi > 0) {
      _quickSort(pADA, low, pi - 1, cmp);
    }
    _quickSort(pADA, low + 1, high, cmp);
  }
}

/**
 * @private
 */
void sortDA(void *pDA, int cmp(void *a, void *b)) {
	_dynArrayAny *pADA = pDA;
  _quickSort(pADA, 0, pADA->size - 1, cmp);
}

size_t addAllDA(void *pDA, const void *src, size_t length) {
  _dynArrayAny *pADA = pDA;
  size_t lastIndex = pADA->size;
  pADA->size += length;

  _extendCapacity(pADA);

  void *dest = _toPtr(pADA, lastIndex);
  memcpy(dest, src, pADA->elementSize * length);

  return pADA->size - 1;
}

size_t addDA(void *pDA, const void *value) { return addAllDA(pDA, value, 1); }

size_t setDA(const void *pDA, const size_t index, const void *value) {
  _dynArrayAny *pADA = (_dynArrayAny *)pDA;

  if (index >= 0 && index < pADA->size) {
    memcpy(pADA->array + (index * pADA->elementSize), value, pADA->elementSize);
  } else {
    DEBUG_LOG("Index out of range: %ld, array size: %ld", index, pADA->size);
    return -1;
  }

  return index;
}

void getDA(const void *pDA, const size_t index, void *value) {
  _dynArrayAny *pADA = (_dynArrayAny *)pDA;

  if (index >= 0 && index < pADA->size) {
    memcpy(value, pADA->array + (index * pADA->elementSize), pADA->elementSize);
  } else {
    DEBUG_LOG("Index out of range: %ld, array size: %ld", index, pADA->size);
  }
}

void freeDA(void *pDA) {
  if (pDA) {
    free(((_dynArrayAny *)pDA)->temp);
    free(((_dynArrayAny *)pDA)->array);
    free(pDA);
  }
}
