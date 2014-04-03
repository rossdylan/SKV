#include "btree.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Create and fill a new KDP struct
KDP* NewKDP(const char* key, void* data) {
	KDP* newKDP = malloc(sizeof(KDP));
	newKDP->Key = key;
	newKDP->Data = data;
	return newKDP;
}

// Create a new tree node struct and fill it with default values
BTreeNode* NewBTreeNode(int order) {
	BTreeNode* newNode = malloc(sizeof(BTreeNode));
	newNode->order = order;
	newNode->size = 0;
	newNode->leaf = true;

	newNode->keys = malloc(sizeof(KDP)*(order+1));
	memset(newNode->keys, 0, sizeof(KDP)*(order+1));

	newNode->leaves = malloc(sizeof(BTreeNode*)*(order+1));
	memset(newNode->leaves, 0, sizeof(BTreeNode*)*(order+1));

	newNode->parent = NULL;
	return newNode;
}

// free a single btree nodes memory
void BTreeNode_free(BTreeNode* node) {
	free(node->keys);
	free(node->leaves);
	free(node);
}

// recursively free all nodes below and including the given node
void BTreeNode_freeRecursive(BTreeNode* node) {
	if(node == NULL) {
		return;
	}
	printf("\n---------\nDeleting:");
	BTreeNode_print(node);
	if(node->size == 0) {
		BTreeNode_free(node);
		return;
	}
	else {
		for(int i=0; i < node->size+1; i++) {
			BTreeNode_freeRecursive(node->leaves[i]);
		}
		BTreeNode_free(node);
		return;
	}
}

// check if a btree node is empty
bool BTreeNode_isEmpty(BTreeNode* node) {
	return node->size == 0;
}

// print out the metadata stored in a btree node
void BTreeNode_print(BTreeNode* node) {
	printf("Size: %d\n", node->size);
	printf("Order: %d\n", node->order);
	printf("Keys:\n");
	for(int i=0; i < node->size; i++) {
		printf("\tKDP: %s %s\n", node->keys[i].Key, node->keys[i].Data);
	}
	printf("Leaves:\n");
	for(int i=0; i < node->size+1; i++) {
		printf("\t Leaf: %p\n", node->leaves[i]);
	}
}

// find the next closest node to the node containing the give key
// if the key is not in our tree, or we found it return the same node
BTreeNode* BTreeNode_descend(BTreeNode* root, const char* key) {
	if(BTreeNode_isEmpty(root)) {
		return root;
	}
	KDP kdp;
	for(int i = 0; i < root->size; i++) {
		kdp = root->keys[i];
		int cmp = strcmp(key, kdp.Key);
		if(cmp == 0) {
			return root;
		}
		if(cmp < 0) {
			return root->leaves[i];
		}
	}
	BTreeNode* farNode = root->leaves[root->size];
	if(farNode == NULL) {
		return root;
	}
	return farNode;
}

// get the data stored in a node with the given key
// or return null if it doesn't exist
void* BTreeNode_getData(BTreeNode* node, const char* key) {
	int cmp;
	for(int i = 0; i< node->size; i++) {
		cmp = strcmp(key, node->keys[i].Key);
		if(cmp == 0) {
			return node->keys[i].Data;
		}
	}
	return NULL;
}

// add a new kdp to the given node
int BTreeNode_addKey(BTreeNode* node, KDP kdp) {
	node->keys[node->size] = kdp;
	node->size++;
	return node->size-1;
}

// return the value paired with the given key by searching through the tree
void* BTreeNode_search(BTreeNode* node, const char* key) {
	BTreeNode* last = node;
	BTreeNode* next;
	while(true) {
		next = BTreeNode_descend(last, key);
		if(next == last) {
			return BTreeNode_getData(next, key);
		}
		if(BTreeNode_isEmpty(node)) {
			return NULL;
		}
		else {
			last = next;
		}
	}
}

// used in the btree promotion logic
// take a single node and split it into two nodes each containing half the original nodes KDPs
Split_t* BTreeNode_split(BTreeNode* node) {
	Split_t* split = malloc(sizeof(Split_t));
	int middleIndex = node->size / 2;
	BTreeNode* left = NewBTreeNode(node->order);
	BTreeNode* right = NewBTreeNode(node->order);
	for(int i = 0; i < node->size; i++) {
		if(i < middleIndex) {
			BTreeNode_addKey(left, node->keys[i]);
		}
		if(i > middleIndex) {
			BTreeNode_addKey(right, node->keys[i]);
		}
	}
	split->left = left;
	split->middle = node->keys[middleIndex];
	split->right = right;
	return split;
}

// climb up the tree until we find the root node
BTreeNode* BTreeNode_findRoot(BTreeNode* node) {
	BTreeNode* current = node;
	while(true) {
		if(current->parent == NULL) {
			return current;
		} else {
			current = current->parent;
		}
	}
}

// when we fill up a node we need to rebalance the tree via promotion
// we split the overflown node and add the resulting two nodes to the parent node
// if this operation overflows that parent node we run promote again
BTreeNode* BTreeNode_promote(BTreeNode* node) {
	BTreeNode* parent = node->parent;
	BTreeNode* current = node;
	while(true) {
		if(parent == NULL) {
			// We need to make a new root node
			parent = NewBTreeNode(node->order);
		}
		Split_t* splitup = BTreeNode_split(current);
		int index = BTreeNode_addKey(parent, splitup->middle);
		parent->leaves[index] = splitup->left;
		parent->leaves[index]->parent = parent;
		parent->leaves[index+1] = splitup->right;
		parent->leaves[index+1]->parent = parent;
		parent->leaf = false;
		free(splitup);
		if(parent->size <= parent->order) {
			break;
		}
		else {
			current = parent;
			parent = current->parent;
		}
	}
	BTreeNode_free(current);
	return BTreeNode_findRoot(parent);
}

// climb down the tree and find the right place to put a new KDP
// if we overflow a node's storage we run the promote function
BTreeNode* BTreeNode_insert(BTreeNode* root, const char* key, void* data) {
	KDP* newKDP = NewKDP(key, data);
	BTreeNode* last = root;
	BTreeNode* next;
	while(true) {
		next = BTreeNode_descend(last, key);
		if(next == last) {
			BTreeNode_addKey(next, *newKDP);
			free(newKDP);
			// max size for a node is 2n - 1
			// min size for a node is n-1
			if(next->size <= (2 * next->order) - 1) {
				return root;
			}
			else {
				return BTreeNode_promote(next);
			}
		}
		last = next;
	}
	return root;
}


//find where the node is and memset it to 0
// we will fill in the gap later if we need to
int BTreeNode_removeData(BTreeNode* node, const char* key) {
	int cmp;
	int index = -1;
	for(int i=0; i < node->size; i++) {
		cmp = strcmp(key, node->keys[i].Key);
		if(cmp == 0) {
			index = i;
			break;
		}
	}
	if(index >= 0) {
		memset(node->keys+(index * sizeof(KDP)), 0, sizeof(KDP));
		node->size--;
	}
	return index;
}

// fill in the gaps in a nodes key array
// used to compact things after a delete operation
void BTreeNode_fillGaps(BTreeNode* node, int index) {
	for(int i=index+1;i<node->size+1;i++) {
		node->keys[i-1] = node->keys[i];
	}
}

//delete a node from the tree
// this is super complicated due to all the self balancing edge cases
BTreeNode* BTreeNode_delete(BTreeNode* root, const char* key) {
	BTreeNode* last = root;
	BTreeNode* next;
	while(true) {
		next = BTreeNode_descend(root, key);
		// this node is either the node where our key is, or there is no key
		if(next == last) {
			int removedIndex = BTreeNode_removeData(next, key);
			if(next->leaf) {
				// we just delete since this is a leaf node
				BTreeNode_fillGaps(next, removedIndex);
				return root;
			}
			else {
				//well we need to do a lot of work now
				if(next->size < next->order-1) {

				}
				else {
					// we can pull a kdp from a node below us
					return root;
				}
			}
		}
	}
}

/*
int main(int argc, char* argv[]) {
	BTreeNode* root = NewBTreeNode(2);
	root = BTreeNode_insert(root, "A", "1");
	BTreeNode_print(root);
	printf("------\n");
	root = BTreeNode_insert(root, "B", "2");
	BTreeNode_print(root);
	printf("------\n");
	root = BTreeNode_insert(root, "C", "3");
	BTreeNode_print(root);
	printf("------\n");

	char* aval = (char *) BTreeNode_search(root, "A");
	printf("Found %s: %s\n", "A", aval);

	char* bval = (char *) BTreeNode_search(root, "B");
	printf("Found %s: %s\n", "B", bval);

	char* cval = (char *) BTreeNode_search(root, "C");
	printf("Found %s: %s\n", "B", cval);
	BTreeNode_freeRecursive(root);
	printf("Root node is: %p", root);
}
*/
