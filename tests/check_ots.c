#include <check.h>
#include "generated_suites.h" // This header will list all available suites

int main(void) {
    int number_failed;
    SRunner* sr = srunner_create(suite_create("Total Test Suite"));

// Dynamically add suites
#ifdef INCLUDED_SUITES
    INCLUDED_SUITES
#endif

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return number_failed;
}
