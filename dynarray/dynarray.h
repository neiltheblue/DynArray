#ifndef DYN_ARRAY
#define DYN_ARRAY

/**
 * @file dynarray.h
 *
 * @brief Dynamic Array header file
 *
 * TODO list:
 *
 * - dirty flag for add
 * - dirty flag for set
 * - add free spare memory
 * - array reverse
 * - array copy
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
} dynArray;

/**
 * @brief Dynamic array type definition
 *
 * This definition should be added to an applications source file, which will
 * generate the method and dynArrayure definitions required to support
 * a new typed dynamic array.
 *
 * For example:
 *
 * DEFINE_DYNARRAY_TYPE(long, dynArrayLong, dynArray)
 *
 * This will generate a new dynamic array condynArrayor method called
 * 'dynArray' that will use the 'dynArrayLong' dynamic array data dynArrayure,
 * to store an array of 'long' values:
 *
 * dynArrayLong *pDA = dynArray((dynArrayParams){.size = 10});
 *
 * This macro will also generate a typed value function implementations:
 *
 * size_t setDA<NAME>(const <dynArray> *pDA, const size_t index, <TYPE> value)
 *
 * size_t addDA<NAME>(<dynArray> *pDA, <TYPE> value)
 *
 * void getDA<NAME>(const <dynArray> *pDA, const size_t index, <TYPE> *value)
 *
 * Where NAME is the name of the new array method and TYPE is the element type.
 *
 * int <NAME>Compare(TYPE *a, TYPE *b)
 *
 * This is a default compare function. Returning <0, 0 or >0 id a is less than,
 * equal to or greater than b.
 *
 * @param TYPE the data type that will be stored in the dynamic array
 * @param NAME the name of the new method that will create a typed dynamic array
 *
 */
#define DEFINE_DYNARRAY_TYPE(TYPE, NAME)                                       \
                                                                               \
  size_t setDA##NAME(const dynArray *pDA, const size_t index, TYPE value) {    \
    return setDA(pDA, index, &value);                                          \
  }                                                                            \
                                                                               \
  void getDA##NAME(const dynArray *pDA, const size_t index, TYPE *value) {     \
    getDA(pDA, index, value);                                                  \
  }                                                                            \
                                                                               \
  size_t addDA##NAME(dynArray *pDA, TYPE value) { return addDA(pDA, &value); } \
                                                                               \
  size_t addAllDA##NAME(dynArray *pDA, const TYPE src[], size_t length) {       \
    return addAllDA(pDA, src, length);                                         \
  }                                                                            \
                                                                               \
  void sortDA##NAME(dynArray *pDA, int cmp(void *a, void *b)) {                \
    sortDA(pDA, cmp);                                                          \
  }                                                                            \
                                                                               \
  dynArray *NAME(dynArrayParams *params) {                                     \
    return _createDynArray(params, sizeof(TYPE));                              \
  }                                                                            \
                                                                               \
  int NAME##Compare(void *a, void *b) {                                        \
    return (*(TYPE *)a < *(TYPE *)b) ? -1 : (*(TYPE *)a > *(TYPE *)b) ? 1 : 0; \
  }

/**
 * @brief Parameter dynArrayure for creating a new dynamic array
 *
 * This dynArray is for use with the helper
 * macro DEFINE_DYNARRAY_TYPE().
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
 * The helper macro DEFINE_DYNARRAY_TYPE uses this call to create a new
 * dynamic array function.
 *
 * @param params a pointer to the dynamic array parameters or NULL for default
 * @param elementSize the element size to reserve
 * @return An initialised dynamic array that
 *          should be freed with freeDA()
 */
dynArray *_createDynArray(dynArrayParams *params, size_t elementSize);

/**
 * @brief Free a dynamic array instance
 * @param pDA the dynamic array pointer to free
 */
void freeDA(void *pDA);

/**
 * @brief Set a dynamic array value
 * @param pDA the array pointer to update
 * @param index the index to set
 * @param value the value to apply
 * @return the updated index
 */
size_t setDA(const void *pDA, const size_t index, const void *value);

/**
 * @brief Add a dynamic array value
 * @param pDA the array pointer to update
 * @param value the value to apply
 * @return the updated index
 */
size_t addDA(void *pDA, const void *value);

/**
 * @brief Add a dynamic array value
 * @param pDA the array pointer to update
 * @param src the source value array
 * @param length the number of elements to copy
 * @return the last updated index
 */
size_t addAllDA(void *pDA, const void *src, size_t length);

/**
 * @brief Get a dynamic array value
 * @param pDA the array pointer to update
 * @param index the index to read
 * @param value the value pointer to set
 */
void getDA(const void *pDA, const size_t index, void *value);

/**
 * @brief Sort the array
 * @param pDA the array pointer to update
 * @param cmp the comparative function to apply
 */
void sortDA(void *pDA, int cmp(void *a, void *b));

#endif // DYN_ARRAY
