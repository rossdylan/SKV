#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

#include <SKV/disktree.h>
#include <SKV/pagemanager.h>

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

uint8_t* load_data_from_page(PageManager* pm, PageRef* ref) {
	RawPage* page = acquire_ref(pm, ref);
	DataHeader* header = (DataHeader* )page->page+ref->node_offset;
	uint8_t* data = (uint8_t* )page->page+ref->node_offset+sizeof(DataHeader);
	return data;
}

void remove_data_from_page(RawPage* page, PageRef* ref) {
	// Need to think about this
	// Removing data from a page is going to cause fragmentation
	// Might need to keep a map of freespace
	// more thinking has yielded the idea of periodic compaction/defrag runs
	// do it once on startup and then after some N number of deletions
	return;
}


PageRef* add_tree_to_page(RawPage* page, TreeNode* node) {
	PageMeta* meta = load_page_meta(page);
	int last_offset = meta->last_offset;
	// First we need to setup our header
	TreeHeader* header = (TreeHeader* )page->page+last_offset;
	header->size = node->size;
	header->order = node->order;
	header->parent = node->parent;
	header->num_leaves = node->num_leaves;
	uint64_t end_of_header = (uint64_t)page->page+last_offset+sizeof(TreeHeader)+1;
	// now we need to iterate through our leaf references and store them
	for(int i=0; i<node->num_leaves;i++) {
		PageRef* leaf = (PageRef* )(i * sizeof(PageRef) + end_of_header);
		memcpy(leaf, node->leaves+(i * sizeof(PageRef)), sizeof(PageRef));
	}
	uint64_t end_of_leaves = end_of_header + (sizeof(PageRef) * node->num_leaves) + 1;
	uint64_t latest_offset = end_of_leaves;
	for(int i=0; i<node->size;i++) {
		KeyHeader* kdp = (KeyHeader* )latest_offset;
		kdp->key_size = node->keys[i].key_size;
		kdp->data = node->keys[i].data;
		latest_offset += sizeof(KeyHeader) + 1;
		char* key_ptr = (char* )latest_offset;
		uint64_t key_size = sizeof(char) * node->keys[i].key_size;
		memcpy(key_ptr, node->keys + (sizeof(char) * i), key_size);
		latest_offset += key_size + 1;
	}
	PageRef* result = malloc(sizeof(PageRef));
	result->page_type = TREEPAGE_HEADER;
	result->page_num = meta->page_num;
	result->node_offset = last_offset;
	meta->last_offset = latest_offset + 1;
	return result;
}

TreeNode* load_tree_node(PageManager* pm, PageRef* loc) {
	RawPage* page = acquire_ref(pm, loc);
	TreeHeader* header = (TreeHeader* )page->page + loc->node_offset;
	TreeNode* node = malloc(sizeof(TreeNode));
	node->size = header->size;
	node->order = header->order;
	node->parent = header->parent;
	node->num_leaves = header->num_leaves;
	node->leaves = malloc(sizeof(PageRef) * node->order+1);
	node->keys = malloc(sizeof(KDP) * node->order+1);
	uint64_t leaves_start = sizeof(TreeHeader) + loc->node_offset + 1;
	for(int i=0;i<node->num_leaves;i++) {
		memcpy(node->leaves+(i * sizeof(PageRef)), page->page + leaves_start + i*(sizeof(PageRef)), sizeof(PageRef));
	}
	uint64_t keys_start = leaves_start + node->num_leaves * sizeof(PageRef) + 1;
	uint64_t key_offsets = 0;
	for(int i=0;i<node->size;i++) {
		KeyHeader* kheader = (KeyHeader* )(page->page + keys_start + (sizeof(KeyHeader) * i) + key_offsets);
		memcpy(&node->keys[i].data, &kheader->data, sizeof(PageRef));
		node->keys[i].key_size = kheader->key_size;
		node->keys[i].key = malloc(sizeof(char) * kheader->key_size);
		char* ondisk_key = (char* )(page->page + keys_start + (sizeof(KeyHeader) * i+1) + key_offsets);
		strcpy(node->keys[i].key, ondisk_key);
		key_offsets += kheader->key_size * sizeof(char);
	}
	release_ref(pm, loc);
	return node;
}

RawPage* new_tree_page(PageManager* pm) {
	RawPage* raw = new_raw_page(pm);
	PageMeta* meta = load_page_meta(raw);
	meta->page_type = TREEPAGE_HEADER;
	return raw;
}

RawPage* new_data_page(PageManager* pm) {
	RawPage* raw = new_raw_page(pm);
	PageMeta* meta = load_page_meta(raw);
	meta->page_type = DATAPAGE_HEADER;
	return raw;
}

