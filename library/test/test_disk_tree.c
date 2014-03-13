#include "disktree.h"
#include <string.h>
#include <check.h>
#include <stdlib.h>
#include <stdio.h>

START_TEST(test_new_page_path)
{
    char* expectedPath = "./1.data";
    char* thePath = newPageFileString(".", 1);
    ck_assert_str_eq(thePath, expectedPath);
    free(thePath);
}
END_TEST

Suite* disktree_suite(void) {
	Suite* suite = suite_create("Disktrees");
	TCase* tc_pagestring = tcase_create("pagestring");
	tcase_add_test(tc_pagestring, test_new_page_path);
	suite_add_tcase(suite, tc_pagestring);
	return suite;
}

int main (void) {
	int num_failed;
	Suite *s = disktree_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
