#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

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
void *_safeMMap(FILE *fp, const size_t count, const size_t size) {
  void *rtn;
  size_t cap = sizeof(fileHeader) + (count * size);
  int fd = fileno(fp);
  ftruncate(fd, cap);
  rtn = mmap(NULL, cap, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
  if (rtn == MAP_FAILED) {
    EXIT_ERROR("Error creating memory map file. Capacity: %lu\n", cap);
  }

  return rtn + sizeof(fileHeader);
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
void *_safeReMMap(FILE *fp, void *ptr, const size_t cap, const size_t count,
                  const size_t size) {
  void *rtn;
  size_t newCap = sizeof(fileHeader) + (count * size);
  size_t oldCap = sizeof(fileHeader) + (cap * size);
  msync(ptr - sizeof(fileHeader), oldCap, MS_SYNC);
  munmap(ptr - sizeof(fileHeader), oldCap);
  int fd = fileno(fp);
  ftruncate(fd, newCap);
  rtn = mmap(NULL, newCap, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
  if (rtn == MAP_FAILED) {
    EXIT_ERROR("Error extending memory map file. Capacity: %lu\n", newCap);
  }

  return rtn + sizeof(fileHeader);
}

/**
 * @private
 */
void _updateFromHeader(dynArray *pDA, const fileHeader *header) {

  if (header->version == 1) {
    pDA->elementSize = header->elementSize;
    pDA->size = header->size;
    pDA->capacity = header->capacity;
    pDA->growth = header->growth;
  } else {
    EXIT_ERROR("Error invalid header version: %lu\n", header->version);
  }
}

/**
 * @private
 */
void _updateMMap(dynArray *pDA) {

  fileHeader header;
  readHeaderDA(pDA, &header);
  header.version=1;
  header.elementSize = pDA->elementSize;
  header.size = pDA->size;
  header.capacity = pDA->capacity;
  header.growth = pDA->growth;

  *(fileHeader *)(pDA->array - sizeof(fileHeader)) = header;
  msync(pDA->array - sizeof(fileHeader), sizeof(fileHeader), MS_SYNC);
}

/**
 * @private
 */
bool _extendCapacityDA(dynArray *pDA) {
  bool extended = false;
  size_t cap = pDA->capacity;
  if (pDA->size >= pDA->capacity) {

    if (pDA->capacity < 1) {
      pDA->capacity = pDA->size;
    }

    if (pDA->growth <= 1.0) {
      pDA->capacity = pDA->size;
    }

    while (pDA->size > pDA->capacity) {
      pDA->capacity = ceil(pDA->capacity * pDA->growth);
    }
    if (pDA->fp == NULL) {
      pDA->array =
          _safeReallocarray(pDA->array, pDA->capacity, pDA->elementSize);
    } else {
      pDA->array = _safeReMMap(pDA->fp, pDA->array, cap, pDA->capacity,
                               pDA->elementSize);
      _updateMMap(pDA);
    }

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

void syncDA(dynArray *pDA) {
  if (pDA->fp == NULL) {
    size_t cap = sizeof(fileHeader) + (pDA->capacity * pDA->size);
    msync(pDA->array - sizeof(fileHeader), cap, MS_SYNC);
  }
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

dynArray *loadDA(const char *filename,
                 int compare(const void *a, const void *b)) {
  dynArray *pDA;

  pDA = _safeCalloc(1, sizeof(dynArray));
  pDA->fp = fopen(filename, "a+");

  pDA->compare = compare;
  pDA->parent = NULL;

  // load the file
  int fd = fileno(pDA->fp);
  void *map =
      mmap(NULL, sizeof(fileHeader), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    EXIT_ERROR("Error creating memory map file. Capacity: %lu\n",
               sizeof(fileHeader));
  }
  _updateFromHeader(pDA, (fileHeader *)map);
  pDA->array = map + sizeof(fileHeader);

  pDA->temp = _safeCalloc(1, pDA->size);

  return pDA;
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
  if (params->filename != NULL) {
    pDA->fp = fopen(params->filename, "w+");
  } else {
    pDA->fp = NULL;
  }

  pDA->capacity = (params->capacity) < 1 ? 1 : params->capacity;
  pDA->growth = params->growth;
  pDA->size = params->size;
  pDA->elementSize = elementSize;
  pDA->compare = compare;
  pDA->temp = _safeCalloc(1, elementSize);
  if (pDA->fp == NULL) {
    pDA->array = _safeCalloc(pDA->capacity, elementSize);
  } else {
    pDA->array = _safeMMap(pDA->fp, pDA->capacity, elementSize);
    _updateMMap(pDA);
  }
  pDA->parent = NULL;
  return pDA;
}

void sortDA(dynArray *pDA, int compare(const void *a, const void *b)) {
  _quickSort(pDA, 0, pDA->size - 1, compare ? compare : pDA->compare);
}

bool addArrayDA(dynArray *pDA, const void *src, const size_t length) {
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
    addArrayDA(pDA, value, 1);
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
    appended = addArrayDA(pDA, pSrc->array, pSrc->size);
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
      if (pDA->fp == NULL) {
        free(pDA->array);
      }
    }
    if (pDA->fp != NULL) {
      _updateMMap(pDA);
      syncDA(pDA);
      fclose(pDA->fp);
    }
    free(pDA);
  }
}

int compareString(const void *a, const void *b) { return strcmp(a, b); }

void forEachDA(dynArray *pDA, bool call(void *entry, void *ref), void *ref) {
  size_t limit = pDA->size;
  bool cont = true;
  for (size_t i = 0; cont && i < limit; i++) {
    cont = call(getDA(pDA, i), ref);
  }
}

bool readHeaderDA(dynArray *pDA, fileHeader *header) {
  bool read = false;

  if (pDA->fp != NULL) {
    const fileHeader *src = (fileHeader *)(pDA->array - sizeof(fileHeader));
    *header = *src;
  }

  return read;
}

bool saveHeaderBufferDA(dynArray *pDA, char buffer[]) {
  bool read = false;

  if (pDA->fp != NULL) {
    fileHeader *header = (fileHeader *)(pDA->array - sizeof(fileHeader));
    memcpy(header->buffer, buffer, FILE_BUFFER);
  }

  return read;
}
