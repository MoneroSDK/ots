#include <check.h>
#include "../include/ots.h"

START_TEST(test_seed_language)
{
}
END_TEST

Suite* seed_language_suite(void)
{
    Suite* s = suite_create("Seed Language");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_seed_language);
    suite_add_tcase(s, tc_core);
    return s;
}

