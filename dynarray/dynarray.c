#include <errno.h>
#include <stdlib.h>

#include "dynarray.h"

/**
 * @brief Safe calloc with out-of-memory handling
 * @param count the number of elements
 * @param size the size of each element
 */
void *safeCalloc(const size_t count, const size_t size) {
  void *rtn;
  if ((rtn = calloc(count, size)) == NULL && errno == ENOMEM) {
    printf("Out of memory\n");
    exit(EXIT_FAILURE);
  }
  return rtn;
}

/**
 * @brief Safe reallocarray with out-of-memory handling
 * @param ptr the pointer to reallocate
 * @param count the number of elements to allocate
 * @param size the size of each element
 */
void *safeReallocarray(void *ptr, const size_t count, const size_t size) {
  void *rtn;
  if ((rtn = reallocarray(ptr, count, size)) == NULL && errno == ENOMEM) {
    printf("Out of memory\n");
    exit(EXIT_FAILURE);
  }
  return rtn;
}

void *_createDynArray(
    DynArrayParams params, size_t structSize, size_t elementSize,
    void (*setter)(const void *pDA, const size_t index, const void *value),
    void (*getter)(const void *pDA, const size_t index, void *value)) {
  _dynArrayVoid *pVDA;

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
  pVDA = safeCalloc(1, structSize);
  pVDA->capacity = params.capacity;
  pVDA->growth = params.growth;
  pVDA->size = params.size;
  pVDA->elementSize = elementSize;
  pVDA->set = setter;
  pVDA->get = getter;
  pVDA->array = safeCalloc(pVDA->capacity, elementSize);
  return pVDA;
}

size_t addDA(void *pDA, const void *value) {
  _dynArrayVoid *pVDA = pDA;
  size_t index = pVDA->size++;

  while (index >= pVDA->capacity) {
    pVDA->capacity *= pVDA->growth;
    pVDA->array = safeReallocarray(pVDA->array, pVDA->capacity, pVDA->elementSize);
  }

  return setDA(pDA, index, value);
}

size_t setDA(const void *pDA, const size_t index, const void *value) {
  _dynArrayVoid *pVDA = (_dynArrayVoid *)pDA;
  size_t idx = index;
  if (index >= 0 && index < pVDA->size) {
    pVDA->set(pDA, index, value);
  } else {
#ifdef DEBUG
    fprintf(stderr, "Index out of range: %ld, array size: %ld", index,
            pVDA->size);
#endif // DEBUG
    idx = -1;
  }
  return idx;
}

void getDA(const void *pDA, const size_t index, void *value) {
  _dynArrayVoid *pVDA = (_dynArrayVoid *)pDA;
  if (index >= 0 && index < pVDA->size) {
    pVDA->get(pDA, index, value);
  } else {
#ifdef DEBUG
    fprintf(stderr, "Index out of range: %ld, array size: %ld", index,
            pVDA->size);
    dumpDA(pDA, stderr);
#endif // DEBUG
  }
}

void freeDA(void *pDA) {
  if (pDA) {
    free(((_dynArrayVoid *)pDA)->array);
    free(pDA);
  }
}

void dumpDA(const void *pDA, FILE *stream) {
  _dynArrayVoid *pVDA = (_dynArrayVoid *)pDA;
  fprintf(stream,
          "DynArray:\n\tSize:%ld\n\tCapacity:%ld\n\tGrowth:%f\n\tArray:%p\n",
          pVDA->size, pVDA->capacity, pVDA->growth, pVDA->array);
}
