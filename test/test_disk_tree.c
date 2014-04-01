#include <string.h>
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SKV/llist.h>
#include <SKV/pagemanager.h>
#include <SKV/disktree.h>

START_TEST(test_new_page_path)
{
	PageManager* pm = new_page_manager(".");
	char* expected_path = malloc(strlen("./1.dat")+1);
	memset(expected_path, 0, strlen("./1.dat")+1);
	sprintf(expected_path, "./1.dat");
	char* the_path = new_page_file_string(pm, 1);
	ck_assert_str_eq(the_path, expected_path);
	free(the_path);
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
