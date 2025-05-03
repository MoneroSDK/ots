#include <check.h>
#include "../include/ots.h"

START_TEST(test_seed_jar)
{
}
END_TEST

Suite* seed_jar_suite(void)
{
    Suite* s = suite_create("Seed Jar");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_seed_jar);
    suite_add_tcase(s, tc_core);
    return s;
}

