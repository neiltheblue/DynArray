#include "main.h"

#include <setjmp.h>
#include <stdint.h>
#include <zcmocka.h>

int main(void) {

  //  int count_fail_tests = test_array() + test_tree();
  int count_fail_tests = test_array(); printf("Skipping Tree tests!!\n");

  if (count_fail_tests == 0) {
    printf("****************\n  All good!! \n****************\n");
  } else {
    printf("=====================\n  Error count: %d\n=====================\n",
           count_fail_tests);
  }

  return count_fail_tests;
}