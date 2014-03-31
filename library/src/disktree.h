#ifndef _SKV_DISKTREE_H_
#define _SKV_DISKTREE_H_

#define TREEPAGE_HEADER 0xDE110
#define DATAPAGE_HEADER 0xDE111

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * A PageRef is a reference to a location in a page. It specifies the page
 * number and the offset which is used to mmap data from that page.
 * A PageRef that has an offset of 0 points to the page as a whole. This is
 * used when adding data to a page, and returned when a new page is made
 */
typedef struct {
    int page_type;
    int page_num; //the page to load
    int node_offset; //the offset in the page to load
} PageRef;

// Wraps around the raw page fd and mmaped memory
typedef struct {
	uint8_t* page;
	int fd;
    int size;
    bool empty;
} RawPage;

// Page metadata stored at the begining of the file
typedef struct {
	uint64_t page_type; // the type of the page
	uint64_t last_offset; // offset of the last node in this page
	uint64_t page_num; // what page is this
} PageMeta;

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

/**
 * The PageManager holds bits of global state that need to be passed around to
 * the other functions used to manage pages
 */
typedef struct {
    char* root_path;
    char* index_path;
    blksize_t sector_size;
} PageManager;


//Index struct with information on the state of both data and tree pages
typedef struct {
    int page_num; // number of pages
} IndexPage;

// PageManagers
PageManager* new_page_manager(char* root_path);
void delete_page_manager(PageManager* pm);

// raw pages
RawPage* load_page(char* path, int size);
void unload_page(RawPage* page);
RawPage* new_data_page(PageManager* pm);
PageMeta* load_page_meta(RawPage* page);
int next_page_num(PageManager* pm);
void increment_page_num(PageManager* pm);
RawPage* new_raw_page(PageManager* pm);

// data page
PageRef* add_data_to_page(RawPage* page, uint8_t* data, int size);
uint8_t* load_data_from_page(RawPage* page, PageRef* ref);
void remove_data_from_page(RawPage* page, PageRef* ref);
RawPage* new_data_page(PageManager* pm);

// Tree page
RawPage* new_tree_page(PageManager* pm);
TreeNode* load_tree_node(RawPage* page, PageRef* loc);
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
 */
