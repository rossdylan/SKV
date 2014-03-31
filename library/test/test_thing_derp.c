#include "disktree.h"
#include <string.h>
#include <check.h>
#include <stdlib.h>
#include <stdio.h>

PageRef* save_string(RawPage* page, char* str) {
	PageRef* ref = add_data_to_page(page, (unsigned char* )str, strlen(str)+1);
	printf("I stored a string at: %d-%d\n", ref->page_num, ref->node_offset);
	return ref;
}

char* load_string(RawPage* page, PageRef* ref) {
	char* gen_data = (char* )load_data_from_page(page, ref);
	printf("I loaded '%s', from %d-%d\n", gen_data, ref->page_num, ref->node_offset);
	return gen_data;
}

PageRef* save_node(RawPage* page, TreeNode* node) {
	PageRef* ref = add_tree_to_page(page, node);
	printf("I stored a node at: %d-%d\n", ref->page_num, ref->node_offset);
	return ref;
}


TreeNode* load_node(RawPage* page, PageRef* ref) {
	TreeNode* node = load_tree_node(page, ref);
	printf("I loaded a node with ['size': %d, 'order': %d, 'num_leaves': %d, 'parent': ['page': %d, 'offset': %d]]", node->size, node->order, node->num_leaves, node->parent.page_num, node->parent.node_offset);
	return node;
}

int main (void) {
	PageManager* pm = new_page_manager(".");
	printf("I Made my pm\n");
	printf("My index path is: %s\n", pm->index_path);
	RawPage* new_page = new_data_page(pm);
	printf("new page fd is %d at memory addr %p\n", new_page->fd, new_page->page);
	// Save some strings
	PageRef* str_1 = save_string(new_page, "Hello There");
	PageRef* str_2 = save_string(new_page, "I am a second string");
	// load some strings
	load_string(new_page, str_1);
	load_string(new_page, str_2);

	//make our tree page
	RawPage* t_page = new_tree_page(pm);
	TreeNode* test_node = malloc(sizeof(TreeNode));
	memset(test_node, 0, sizeof(TreeNode));
	test_node->size = 0;
	test_node->order = 2;
	test_node->num_leaves = 0;
	test_node->parent.page_type = 0xDE110;
	test_node->parent.page_num = 31337;
	test_node->parent.node_offset = 31337;
	PageRef* node1 = save_node(t_page, test_node);
	load_node(t_page, node1);

}
