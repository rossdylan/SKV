#include "pagemanager.h"
#include "llist.h"
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


/*
 * Utilities
 */

/** Fill a file with len b bytes */
void fill_file(int fd, uint8_t b, size_t len) {
	uint8_t* mem_bytes = malloc(sizeof(uint8_t) * len);
	memset(mem_bytes, b, sizeof(uint8_t) * len);
	write(fd, mem_bytes, sizeof(uint8_t) * len);
	free(mem_bytes);
	fsync(fd);
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


/*
 * RawPage management
 */

/*
 * Given a path and a size make a RawPage mmaping that file into memory
 */
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

/*
 * Given a pointer to a RawPage munmap it and close the file descriptor
 */
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

/*
 * PageManager functions
 */

/**
 * Create a new raw page using he next available page number
 */
RawPage* new_raw_page(PageManager* pm) {
	int next_page = next_page_num(pm);
	increment_page_num(pm);
	char* next_page_str = new_page_file_string(pm, next_page);
	RawPage* next = load_page(next_page_str, sysconf(_SC_PAGE_SIZE));
	PageMeta* meta = load_page_meta(next);
	meta->last_offset = (uint64_t)sizeof(PageMeta) + 1;
	meta->page_num = next_page;
	return next;
}

PageMeta* load_page_meta(RawPage* page) {
	return (PageMeta* )page->page;
}

/**
 * Index management
 */

IndexPage* load_index(RawPage* page) {
	return (IndexPage* )page->page;
}

/* Return the number of the next unallocated page */
int next_page_num(PageManager* pm) {
	RawPage* raw_index = load_page(pm->index_path, sysconf(_SC_PAGE_SIZE));
	IndexPage* index = load_index(raw_index);
	if(raw_index->empty) {
		index->page_num = 0;
	}
	int next = index->page_num;
	unload_page(raw_index);
	return next;
}

void increment_page_num(PageManager* pm) {
	RawPage* raw_index = load_page(pm->index_path, sysconf(_SC_PAGE_SIZE));
	IndexPage* index = load_index(raw_index);
	index->page_num += 1;
	unload_page(raw_index);
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

/**
 * Create and Destroy PageManagers
 */


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
	pm->cache_size = 0;
	return pm;
}

/* Delete the given PageManager */
void delete_page_manager(PageManager* pm) {
	free(pm->root_path);
	free(pm->index_path);
	free(pm);
}

RawPage* acquire_ref(PageManager* pm, PageRef* ref) {
	if(pm->cached_pages == NULL) {
		RawPage* new_page = load_page(new_page_file_string(pm, ref->page_num), ref->page_num);
		pm->cached_pages = malloc(sizeof(ListNode));
		new_list_node(pm->cached_pages, new_page);
		pm->cache_size = 1;
		return new_page;
	}
	ListNode* prev = pm->cached_pages;
	PageMeta* meta = load_page_meta(prev->data);
	if(meta->page_num == ref->page_num) {
		return prev->data;
	}
	ListNode* next;
	while((next = prev->next) != NULL) {
		RawPage* cached_page = next->data;
		meta = load_page_meta(cached_page);
		if(meta->page_num == ref->page_num) {
			return cached_page;
		}
		prev = next;
	}
	RawPage* new_page = load_page(new_page_file_string(pm, ref->page_num), ref->page_num);
	ListNode* new_node = malloc(sizeof(ListNode));
	new_list_node(new_node, new_page);
	list_append(pm->cached_pages, new_node);
	pm->cache_size += 1;
	if(pm->cache_size >= 20) {
		ListNode* removed = list_remove(pm->cached_pages, 9);
		unload_page(removed->data);
		pm->cache_size -= 1;
	}
	return new_page;

}

void release_ref(PageManager* pm, PageRef* ref) {
	// This function will do more once locking is added in.
	return;
}
