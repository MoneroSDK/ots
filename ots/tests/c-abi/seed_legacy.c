#include <check.h>
#include "../../include/ots.h"
#include "../data.h"

START_TEST(test_seed_legacy)
{
    ots_result_t* result = ots_legacy_seed_decode(
        get_legacy_seed_test_case_phrase(2),
        get_legacy_seed_test_case_height(2),
        get_legacy_seed_test_case_time(2),
        get_legacy_seed_test_case_network(2)
    );
    ck_assert(ots_is_result(result));
    ck_assert(ots_result_is_seed(result));
    ots_handle_t* seed1 = ots_result_handle(result);
    ots_free_result(&result);
    ck_assert_ptr_null(result);
    ck_assert_ptr_nonnull(seed1);
    result = ots_seed_indices(seed1, "");
    ck_assert(ots_result_is_seed_indices(result));
    ots_handle_t* indices = ots_result_handle(result);
    ots_free_result(&result);
}
END_TEST

Suite* seed_legacy_suite(void)
{
    Suite* s = suite_create("Legacy Seed");
    // Core functionality
    TCase* tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_seed_legacy);
    suite_add_tcase(s, tc_core);
    return s;
}

