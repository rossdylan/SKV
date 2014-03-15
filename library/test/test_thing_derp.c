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

}
