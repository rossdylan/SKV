#include "disktree.h"
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>



/** Fill a file with len b bytes */
void fill_file(int fd, uint8_t b, size_t len) {
	uint8_t* mem_bytes = malloc(sizeof(uint8_t) * len);
	memset(mem_bytes, b, sizeof(uint8_t) * len);
	write(fd, mem_bytes, sizeof(uint8_t) * len);
	free(mem_bytes);
	fsync(fd);
}

RawPage* load_page(char* path, int size) {
	int fd;
	if((fd = open(path, O_CREAT|O_RDWR, S_IRWXU)) == -1) {
		perror("Failed open() to load page");
		exit(1);
	}
	struct stat* the_stats = malloc(sizeof(struct stat));
    fstat(fd, the_stats);
	bool empty = false;
	if(the_stats->st_size == 0) {
		fill_file(fd, 0, size);
		empty = true;
	}
	free(the_stats);
	uint8_t* raw = mmap(
			0,
			size,
			PROT_READ|PROT_WRITE,
			MAP_SHARED,
			fd,
			0);
	RawPage* page = malloc(sizeof(RawPage));
	page->page = raw;
	page->fd = fd;
	page->size = size;
	page->empty = empty;
	return page;
}

void unload_page(RawPage* page) {
	if(munmap(page->page, page->size) == -1) {
		perror("Failed to unmap a page in unload_page");
		exit(1);
	}
	if(close(page->fd) == -1) {
		perror("Failed to close fd in unload_page");
		exit(1);
	}
	free(page);
}


PageMeta* load_page_meta(RawPage* page) {
	return (PageMeta* )page->page;
}

PageRef* add_data_to_page(RawPage* page, uint8_t* data, int size) {
	PageMeta* meta = load_page_meta(page);
	int last_offset = meta->last_offset;
	DataHeader* header = (DataHeader* )page->page+last_offset;
	header->data_size = size;
	memcpy(page->page+last_offset+sizeof(DataHeader), data, size);
	PageRef* ref = malloc(sizeof(PageRef));
	ref->page_type = meta->page_type;
	ref->page_num = meta->page_num;
	ref->node_offset = last_offset;
	meta->last_offset = (uint64_t)last_offset+sizeof(DataHeader) + size + 1;
	return ref;
}

uint8_t* load_data_from_page(RawPage* page, PageRef* ref) {
	DataHeader* header = (DataHeader* )page->page+ref->node_offset;
	uint8_t* data = (uint8_t* )page->page+ref->node_offset+sizeof(DataHeader);
	return data;
}

void remove_data_from_page(RawPage* page, PageRef* ref) {
	// Need to think about this
	// Removing data from a page is going to cause fragmentation
	// Might need to keep a map of freespace
	return;
}

RawPage* new_data_page(PageManager* pm) {
	int next_page_num = next_data_page_num(pm);
	increment_data_page_num(pm);
	char* next_page_str = new_page_file_string(pm, next_page_num);
	RawPage* next = load_page(next_page_str, sysconf(_SC_PAGE_SIZE));
	PageMeta* meta = load_page_meta(next);
	meta->page_type = DATAPAGE_HEADER;
	meta->last_offset = (uint64_t)sizeof(PageMeta);
	meta->page_num = next_page_num;
	return next;
}

/* Create a new page manager based out of the gien root path */
PageManager* new_page_manager(char* root_path) {
	PageManager* pm = malloc(sizeof(PageManager));
	memset(pm, 0, sizeof(PageManager));
	pm->root_path = root_path;

	int index_size = strlen(root_path) + strlen("/index.dat")+1;
	pm->index_path = malloc(index_size);
	memset(pm->index_path, 0, index_size);

	printf("NPM Root path: %s\n", root_path);
	strcat(pm->index_path, pm->root_path);
	strcat(pm->index_path, "/index.dat");
	printf("NPM index path: %s\n", pm->index_path);

	struct stat* fsStats = malloc(sizeof(struct stat));
    stat(root_path, fsStats);
	pm->sector_size = fsStats->st_blksize;
	printf("About to free fstats\n");
	free(fsStats);
	printf("Freed fstats\n");
	return pm;
}

/* Delete the given PageManager */
void delete_page_manager(PageManager* pm) {
	free(pm->root_path);
	free(pm->index_path);
	free(pm);
}

IndexPage* load_index(RawPage* page) {
	return (IndexPage* )page->page;
}

/* Return the number of the next unallocated page */
int next_data_page_num(PageManager* pm) {
	RawPage* raw_index = load_page(pm->index_path, sysconf(_SC_PAGE_SIZE));
	IndexPage* index = load_index(raw_index);
	if(raw_index->empty) {
		index->treepage_num = 0;
		index->datapage_num = 0;
	}
	int next = index->datapage_num;
	unload_page(raw_index);
	return next;
}

void increment_data_page_num(PageManager* pm) {
	RawPage* raw_index = load_page(pm->index_path, sysconf(_SC_PAGE_SIZE));
	IndexPage* index = load_index(raw_index);
	index->datapage_num += 1;
	unload_page(raw_index);
}


/* Helper function for use in newPageFileString */
int num_places(int n) {
	int r = 1;
	if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
	while (n > 9) {
		n /= 10;
		r++;
	}
	return r;
}


/* Create a path string for a page based on a root path and a string */
char* new_page_file_string(PageManager* pm, int num) {
	int pnum_size = (num_places(num) + 1)*sizeof(char);
	char* pnum = malloc(pnum_size);
	memset(pnum, 0, pnum_size);
	sprintf(pnum, "%d", num);
	int pathLen = strlen(pnum) + strlen(pm->root_path) + strlen("/.dat") + 1;
	char* path = malloc(pathLen);
	memset(path, 0, pathLen);
	strcat(path, pm->root_path);
	strcat(path, "/");
	strcat(path, pnum);
	strcat(path, ".dat");
	free(pnum);
	return path;
}

