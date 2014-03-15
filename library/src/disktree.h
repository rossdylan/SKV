#ifndef __DISKTREE_H
#define __DISKTREE_H

#define TREEPAGE_HEADER 0xDE110
#define DATAPAGE_HEADER 0xDE111

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

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

//Index struct with information on the state of both data and tree pages
typedef struct {
    int treepage_num; // number of tree pages
    int datapage_num; // number fo data pages
} IndexPage;


void fill_file(int fd, uint8_t b, size_t len);
RawPage* load_page(char* path, int size);
void unload_page(RawPage* page);
PageMeta* load_page_meta(RawPage* page);
PageRef* add_data_to_page(RawPage* page, uint8_t* data, int size);
void remove_data_from_page(RawPage* page, PageRef* ref);
RawPage* new_data_page(PageManager* pm);
PageManager* new_page_manager(char* root_path);
void delete_page_manager(PageManager* pm);
int next_data_page_num(PageManager* pm);
void increment_data_page_num(PageManager* pm);
int num_places(int n);
char* new_page_file_string(PageManager* pm, int num);
uint8_t* load_data_from_page(RawPage* page, PageRef* ref);
#endif
