#include "dict.h"

#include <string.h>
#include <setjmp.h>
#include <stdint.h>
#include <time.h>
#include <zcmocka.h>


void test_hash(void **state) {
	char* input = "this is my test key that needs to be longer than 32 chars";
	int len = strlen(input);
	
	uint32_t h =  hash(input, len, 0);
	assert_int_equal(h, 3685744406);
	
	h =  hash(input, len, 1);
	assert_int_equal(h, 3849301065);	
}

int setupHD(void **state) {
  return 0;
}

int teardownHD(void **state) {
  return 0;
}

int test_dict(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_hash),

  };

  int count_fail_tests = cmocka_run_group_tests(tests, setupHD, teardownHD);

  return count_fail_tests;
}