#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stdbool.h>
#include <stdio.h>

/**
 * @file dynarray.h
 *
 * @brief Dynamic Array header file
 *
 * Create a dynamic array instance using createDA. For example:
 *
 * dynArray *pDALng = createDA(sizeof(long), NULL);
 *
 * To use the sort or search functions you will need a comparason
 * implementation. These can be generated with the help of a macro. To generate
 * the helper funtion declare an array data type in your header file. For
 * example:
 *
 * DECLARE_COMPARE_TYPE(long)
 *
 * Then in your definition file include the macro to define you helper method.
 * For exmaple:
 *
 * DEFINE_COMPARE_TYPE(long)
 *
 */

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
 * @brief Dynamic array entity
 */
typedef struct DynamicArray {
  size_t elementSize;          ///< the element size
  size_t size;                 ///< the array size
  size_t capacity;             ///< the array capacity
  float growth;                ///< the array growth rate
  void *array;                 ///< the array
  void *temp;                  ///< the temp element store
  struct DynamicArray *parent; ///< the parent array
  int (*compare)(const void *a,
                 const void *b); ///< the default comparator function
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
 * @return <0, 0 or >0 if a is less than, equal to or greater than b
 *
 */
#define DECLARE_COMPARE_TYPE(TYPE)                                             \
  int compareDA##TYPE(const void *a, const void *b);

/**
 * @brief Basic type comparator function definition macro
 *
 * @param TYPE the data type to be compared
 * @return <0, 0 or >0 if a is less than, equal to or greater than b
 *
 */
#define DEFINE_COMPARE_TYPE(TYPE)                                              \
  int compareDA##TYPE(const void *a, const void *b) {                          \
    return (*(TYPE *)a < *(TYPE *)b) ? -1 : (*(TYPE *)a > *(TYPE *)b) ? 1 : 0; \
  }

/**
 * @brief Dynamic array creation parameters
 */
typedef struct DynamicArrayParams {
  size_t size;     ///< the initial array size
  float growth;    ///< the growth factor for the array
  size_t capacity; ///< the initial reserved capacity for the array
} dynArrayParams;

/**
 * @brief Create a new dynamic array
 *
 * @param elementSize the element size to reserve
 * @param compare the default comparator function
 * @param params a pointer to the dynamic array parameters or NULL for default
 * @return An initialised dynamic array that
 *          should be freed with freeDA()
 */
dynArray *createDA(size_t elementSize,
                   int compare(const void *a, const void *b),
                   dynArrayParams *params);

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
 * @return 'true' if the index was in range
 */
bool setDA(dynArray *pDA, const size_t index, const void *value);

/**
 * @brief Add a dynamic array value
 *
 * This will only add values if the array is not a sub-array.
 *
 * @param pDA the array pointer to update
 * @param value the value to apply
 * @return 'true' if the values was added
 */
bool addDA(dynArray *pDA, const void *value);

/**
 * @brief Add a dynamic array value
 *
 * This will only add values if the array is not a sub-array.
 *
 * @param pDA the array pointer to update
 * @param src the source value array
 * @param length the number of elements to copy
 * @return 'true' if the values were added
 */
bool addAllDA(dynArray *pDA, const void *src, size_t length);

/**
 * @brief Get a dynamic array value
 * @param pDA the array pointer to update
 * @param index the index to read
 * @return a pointer to the value or NULL if not found
 */
void* getDA(const dynArray *pDA, const size_t index);

/**
 * @brief Sort the array
 * @param pDA the array pointer to update
 * @param compare the comparative function to apply or NULL to use the default
 */
void sortDA(dynArray *pDA, int compare(const void *a, const void *b));

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

/**
 * @brief Perform a binary search for the value
 *
 * If not already in order, the array will be sorted.
 *
 * @param pDA the array pointer to search
 * @param value the value to search for
 * @param index the retuned index if found
 * @param compare the compare function  or NULL to use the default
 * @return 'true' if the value was found
 */
bool searchDA(dynArray *pDA, void *value, size_t *index,
              int compare(const void *a, const void *b));

/**
 * @brief Create a sub array based on the parent array
 *
 * The sub array will have direct access to the underlying array and can make
 * chagnes to it. The sub array can not be extended with add methods. The sub
 * array should be freed with freeDA(), but this will not free the underlying
 * array.
 *
 * @param pDA the underlying array to access
 * @param min the min array index
 * @param max the max array index
 * @return the new sub array pointer or NULL if the new range is not valid
 */
dynArray *subDA(dynArray *pDA, size_t min, size_t max);

/**
 * @brief Append a source dynamic array
 *
 * Will not append to a sub array, and the source array must have the same
 * element size.
 *
 * @param pDA the array pointer to copy to
 * @param pSrc the source array pointer
 * @return 'true' if the array could be copied
 */
bool appendDA(dynArray *pDA, dynArray *pSrc);

/**
 * @brief Compare two strings
 * @param a the first string
 * @param b the second string
 * @return <0, 0 or >0 if a is less than, equal to or greater than b
 */
int compareString(const void *a, const void *b);

/**
 * @private
 */
void *_safeCalloc(const size_t count, const size_t size);

/**
 * @private
 */
void *_safeReallocarray(void *ptr, const size_t count, const size_t size);

#endif
