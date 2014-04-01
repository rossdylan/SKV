#ifndef _SKV_DISKTREE_H_
#define _SKV_DISKTREE_H_

#define TREEPAGE_HEADER 0xDE110
#define DATAPAGE_HEADER 0xDE111

#include "pagemanager.h"
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t key_size;
    PageRef data;
    char* key;
} KDP;

typedef struct {
    uint64_t key_size;
	PageRef data;
} KeyHeader;

// Header prepended to a block of tree metadata
// We need to refer to our tree nodes by PageRef data
typedef struct {
    int size; // number of kdp's
    int order; // the order of this btree
    PageRef parent;
    int num_leaves;
} TreeHeader;

// in memory combination of TreeHeader and all its data/leaves
typedef struct {
    int size;
    int order;
    int num_leaves;
    PageRef parent;
    PageRef* leaves;
    KDP* keys;
} TreeNode;

// Header prepended to a block of data stored in a data page
typedef struct {
	int data_size;
} DataHeader;





// data page
PageRef* add_data_to_page(RawPage* page, uint8_t* data, int size);
uint8_t* load_data_from_page(PageManager* pm, PageRef* ref);
void remove_data_from_page(RawPage* page, PageRef* ref);
RawPage* new_data_page(PageManager* pm);

// Tree page
RawPage* new_tree_page(PageManager* pm);
TreeNode* load_tree_node(PageManager* pm, PageRef* loc);
PageRef* add_tree_to_page(RawPage* page, TreeNode* node);


//Utility functions
int num_places(int n);
void fill_file(int fd, uint8_t b, size_t len);
char* new_page_file_string(PageManager* pm, int num);
#endif

/*
 * NOTES:
 * There should totally be a function that uses PageManager called grab_page
 * it would find the page in the cache, or grab it from disk and put it into the cache
 *
 * all save/load functions would then only need to use the PageManager and grab_page to get everything they need
 *
 * We could also have a per page ref locking system with functions like acquire_ref and release_ref which would add easy to use locking
 * stuff to our paging system.
 * this in turn would make it easier to put some threads in this.
 *
 * index page should also be dealt with using a WAL
 * thats going to need to change some point in the future
 */
