#ifndef _SKV_PAGEMANAGER_H_
#define _SKV_PAGEMANAGER_H_

#define TREEPAGE_HEADER 0xDE110
#define DATAPAGE_HEADER 0xDE111

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include "llist.h"

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

// Page metadata stored at the begining of the file
typedef struct {
	uint64_t page_type; // the type of the page
	uint64_t last_offset; // offset of the last node in this page
	uint64_t page_num; // what page is this
} PageMeta;

/**
 * The PageManager holds bits of global state that need to be passed around to
 * the other functions used to manage pages
 */
typedef struct {
    char* root_path;
    char* index_path;
    blksize_t sector_size;
    ListNode* cached_pages;
    int cache_size;
} PageManager;

// Wraps around the raw page fd and mmaped memory
typedef struct {
	uint8_t* page;
	int fd;
    int size;
    bool empty;
} RawPage;

//Index struct with information on the state of both data and tree pages
typedef struct {
    int page_num; // number of pages
    PageRef tree_root; // we store a ref to the root tree node
} IndexPage;

// constructor/destructor
PageManager* new_page_manager(char* root_path);
void delete_page_manager(PageManager* pm);

// raw page management
RawPage* load_page(char* path, int size);
void unload_page(RawPage* page);
RawPage* new_data_page(PageManager* pm);
PageMeta* load_page_meta(RawPage* page);
int next_page_num(PageManager* pm);
void increment_page_num(PageManager* pm);
RawPage* new_raw_page(PageManager* pm);

// utility functions
int num_places(int n);
void fill_file(int fd, uint8_t b, size_t len);
char* new_page_file_string(PageManager* pm, int num);

//New way for the tree stuff to get pages

// Acquire the given reference to a point in a page
// Eventually lock semantics and caching will be added to this part
RawPage* acquire_ref(PageManager* pm, PageRef* ref);
void release_ref(PageManager* pm, PageRef* ref);


void save_tree_root(PageManager* pm, PageRef* ref);
PageRef* load_tree_root(PageManager* pm);
#endif
