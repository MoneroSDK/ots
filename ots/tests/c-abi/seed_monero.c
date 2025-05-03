#include <check.h>
#include "../include/ots.h"

START_TEST(test_seed_monero)
{
}
END_TEST

Suite* seed_monero_suite(void)
{
    Suite* s = suite_create("Monero Seed");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_seed_monero);
    suite_add_tcase(s, tc_core);
    return s;
}

