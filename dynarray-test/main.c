#include "main.h"


#include <setjmp.h>
#include <stdint.h>
#include <zcmocka.h>


int main(void) {
 
  int count_fail_tests = test_array() + test_dict();

  return count_fail_tests;
}