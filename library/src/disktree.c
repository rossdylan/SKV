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


blksize_t SectorSize(const char* path) {
    blksize_t blksize;
	struct stat* fsStats = malloc(sizeof(struct stat));
    stat(path, fsStats);
	blksize = fsStats->st_blksize;
	free(fsStats);
	return blksize;
}

int NextPageNum(const char* rootPath) {
	int fd;
	char* indexPath = malloc(strlen(rootPath) + strlen("/index.dat")+1);
	strcat(indexPath, rootPath);
	strcat(indexPath, "/index.dat");
	if((fd = open(indexPath, O_CREAT|O_RDWR)) == -1) {
		perror("open");
		exit(1);
	}
    PageIndex* index = (PageIndex* )mmap(
            (caddr_t)0,
            sizeof(PageIndex),
            PROT_READ|PROT_WRITE,
            MAP_SHARED,
            fd,
            0);
    int last_page = index->last_page;
    munmap(index, sizeof(index));
    fsync(fd);
    close(fd);
    return last_page + 1;
}


int numPlaces(int n) {
	int r = 1;
	if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
	while (n > 9) {
		n /= 10;
		r++;
	}
	return r;
}

char* newPageFileString(const char* root, int num) {
    char* pnum = malloc((numPlaces(num) + 1)*sizeof(char));
	sprintf(pnum, "%d", num);
    char* path = malloc(strlen(pnum) + strlen(root) + strlen("/.data") + 1);
    strcat(path, root);
    strcat(path, "/");
    strcat(path, pnum);
    strcat(path, ".data");
    free(pnum);
    return path;
}

PageRef* NewDataPage(const char* rootPath) {
    int pageNumber = NextPageNum(rootPath);
	int fd;
	size_t ssize;
	ssize = SectorSize("./");
    char* pagePath = newPageFileString(rootPath, pageNumber);
	if((fd = open(pagePath, O_CREAT|O_RDWR)) == -1) {
		perror("open");
		exit(1);
	};
	PageHeader* dataPageHeader = (PageHeader* )mmap(
		(caddr_t)0,
		sizeof(PageHeader),
		PROT_READ|PROT_WRITE,
		MAP_SHARED,
		fd,
		0);
	dataPageHeader->header_type = DATAPAGE_HEADER;
	DataPage* dataPage = (DataPage* )mmap(
			(caddr_t)0,
			sizeof(DataPage),
			PROT_READ|PROT_WRITE,
			MAP_SHARED,
			fd,
			sizeof(PageHeader)+1);
	dataPage->data_num = 0;
	dataPage->last_offset = sizeof(PageHeader) + sizeof(DataPage) + 1;
	dataPage->size = sizeof(PageHeader) + sizeof(DataPage);

	PageRef* pr = (PageRef* )malloc(sizeof(PageRef));
	pr->page_num = pageNumber;
	return pr;
}
