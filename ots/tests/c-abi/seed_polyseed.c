#include <check.h>
#include "../include/ots.h"

START_TEST(test_seed_polyseed)
{
}
END_TEST

Suite* seed_polyseed_suite(void)
{
    Suite* s = suite_create("Polyseed");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_seed_polyseed);
    suite_add_tcase(s, tc_core);
    return s;
}

