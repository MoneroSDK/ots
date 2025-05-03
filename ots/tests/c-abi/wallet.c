#include <check.h>
#include "../include/ots.h"

START_TEST(test_wallet)
{
}
END_TEST

Suite* wallet_suite(void)
{
    Suite* s = suite_create("Wallet");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_wallet);
    suite_add_tcase(s, tc_core);
    return s;
}

