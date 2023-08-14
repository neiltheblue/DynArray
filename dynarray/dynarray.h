#ifndef DYN_ARRAY
#define DYN_ARRAY

/**
 * @file dynarray.h
 *
 * @brief Dynamic Array header file
 *
 * TODO list:
 *
 * - array copy
 * - add binary search
 * - subarray
 * - add DA (will also deep copy if applied to empty array)
 *
 */

#include <stdio.h>

#ifdef DEBUG
/**
 * @brief Debug logging helper macro
 */
#define DEBUG_LOG(MSG, ...) fprintf(stderr, MSG, __VA_ARGS__)
#else
/**
 * @brief Debug logging helper macro
 */
#define DEBUG_LOG(MSG, ...)
#endif // DEBUG

/**
 * @brief Exit error helper macro
 */
#define EXIT_ERROR(MSG, ...)                                                   \
  fprintf(stderr, MSG, __VA_ARGS__);                                           \
  exit(EXIT_FAILURE);

/**
 * @brief Dynamic array dynArray declaration
 */
typedef struct DynamicArray {
  size_t elementSize;
  size_t size;
  size_t capacity;
  float growth;
  void *array;
  void *temp;
  unsigned int dirtyAdd : 1;
  unsigned int dirtySort : 1;
} dynArray;

/**
 * @brief Basic type comparator function declaration macro
 *
 * These helper functions are intended for use with the sortDA() function.
 *
 * Each generated function name will be of the form compareDA<TYPE>(void *a,
 * void *b).
 *
 * @param TYPE the data type to be compared
 * @return -1, 0 or 1 if a is less than, equal to or greater than b
 *
 */
#define DECLARE_COMPARE_TYPE(TYPE) int compareDA##TYPE(void *a, void *b);

/**
 * @brief Basic type comparator function definition macro
 *
 * @param TYPE the data type to be compared
 * @return -1, 0 or 1 if a is less than, equal to or greater than b
 *
 */
#define DEFINE_COMPARE_TYPE(TYPE)                                              \
  int compareDA##TYPE(void *a, void *b) {                                      \
    return (*(TYPE *)a < *(TYPE *)b) ? -1 : (*(TYPE *)a > *(TYPE *)b) ? 1 : 0; \
  }

/**
 * @brief Dynamic array creation parameters
 *
 * If the size is greater than 0 the initial capacity will be extended.
 *
 * The growth factor must be greater than 1.0 as it is used to extend the
 * current capacity.
 */
typedef struct DynamicArrayParams {
  size_t size;     ///< the initial array size
  float growth;    ///< the growth factor for the array
  size_t capacity; ///< the initial reserved capacity for the array
} dynArrayParams;

/**
 * @brief Create a new dynamic array.
 *
 * @param elementSize the element size to reserve
 * @param params a pointer to the dynamic array parameters or NULL for default
 * @return An initialised dynamic array that
 *          should be freed with freeDA()
 */
dynArray *createDA(size_t elementSize, dynArrayParams *params);

/**
 * @brief Free a dynamic array instance
 * @param pDA the dynamic array pointer to free
 */
void freeDA(dynArray *pDA);

/**
 * @brief Free extra allocated memory
 *
 * The memory is reallocated if capacity > size.
 *
 * @param pDA the dynamic array pointer to reduce
 */
void reduceMemDA(dynArray *pDA);

/**
 * @brief Set a dynamic array value
 * @param pDA the array pointer to update
 * @param index the index to set
 * @param value the value to apply
 * @return the updated index
 */
size_t setDA(dynArray *pDA, const size_t index, const void *value);

/**
 * @brief Add a dynamic array value
 * @param pDA the array pointer to update
 * @param value the value to apply
 * @return the updated index
 */
size_t addDA(dynArray *pDA, const void *value);

/**
 * @brief Add a dynamic array value
 * @param pDA the array pointer to update
 * @param src the source value array
 * @param length the number of elements to copy
 * @return the last updated index
 */
size_t addAllDA(dynArray *pDA, const void *src, size_t length);

/**
 * @brief Get a dynamic array value
 * @param pDA the array pointer to update
 * @param index the index to read
 * @param value the value pointer to set
 */
void getDA(const dynArray *pDA, const size_t index, void *value);

/**
 * @brief Sort the array
 * @param pDA the array pointer to update
 * @param cmp the comparative function to apply
 */
void sortDA(dynArray *pDA, int cmp(void *a, void *b));

/**
 * @brief Reverse the array
 * @param pDA the array pointer to reverse
 */
void reverseDA(dynArray *pDA);

/**
 * @brief Create a shallow copy of the array
 *
 * The created copy will need to be freeded with freeeDA()
 *
 * @param pDA the array pointer to copy
 * @return the copied array
 */
dynArray *copyDA(dynArray *pDA);

#endif // DYN_ARRAY
