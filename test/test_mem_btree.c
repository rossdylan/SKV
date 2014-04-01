#include <string.h>
#include <check.h>
#include <stdlib.h>

#include <SKV/btree.h>

START_TEST (test_single_insert)
{
	BTreeNode* root = NewBTreeNode(2);
	root = BTreeNode_insert(root, "A", "Hello There");
	char* result = (char *) BTreeNode_search(root, "A");
	ck_assert_str_eq(result, "Hello There");
	BTreeNode_free(root);
}
END_TEST

Suite* mem_btree_suite(void) {
	Suite* suite = suite_create("MemoryBtrees");
	TCase* tc_single = tcase_create("single");
	tcase_add_test(tc_single, test_single_insert);
	suite_add_tcase(suite, tc_single);
	return suite;
}

int main (void) {
	int num_failed;
	Suite *s = mem_btree_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
