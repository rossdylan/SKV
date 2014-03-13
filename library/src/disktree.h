#ifndef __DISKTREE_H
#define __DISKTREE_H

#define TREEPAGE_HEADER 0xDE110
#define DATAPAGE_HEADER 0xDE111

#include <sys/types.h>

typedef struct {
    int last_page;
} PageIndex;

typedef struct {
    int header_type;
} PageHeader;

typedef struct {
    int data_num; // The number of data entries in this page
    int last_offset; // the next open block
    int size;
} DataPage;

typedef struct {
    int tree_num; // The number of tree nodes in this page
} TreePage;

typedef struct {
    int size;
    char* raw_data;
} DataNode;

typedef struct {
    int page_num; //the page to load
    int node_offset; //the offset in the page to load
} PageRef;


blksize_t SectorSize(const char* path);
int NextPageNum(const char* rootPath);
char* newPageFileString(const char* root, int num);
PageRef* NewDataPage(const char* rootPath);
#endif
