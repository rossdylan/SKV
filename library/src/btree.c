#include "btree.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


KDP* NewKDP(const char* key, void* data) {
	KDP* newKDP = malloc(sizeof(KDP));
	newKDP->Key = key;
	newKDP->Data = data;
	return newKDP;
}

BTreeNode* NewBTreeNode(int order) {
	BTreeNode* newNode = malloc(sizeof(BTreeNode));
	newNode->order = order;
	newNode->size = 0;

	newNode->keys = malloc(sizeof(KDP)*(order+1));
	memset(newNode->keys, 0, sizeof(KDP)*(order+1));

	newNode->leaves = malloc(sizeof(BTreeNode*)*(order+1));
	memset(newNode->leaves, 0, sizeof(BTreeNode*)*(order+1));

	newNode->parent = NULL;
	return newNode;
}

void BTreeNode_free(BTreeNode* node) {
	free(node->keys);
	free(node->leaves);
	free(node);
}

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

bool BTreeNode_isEmpty(BTreeNode* node) {
	return node->size == 0;
}

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

int BTreeNode_addKey(BTreeNode* node, KDP kdp) {
	node->keys[node->size] = kdp;
	node->size++;
	return node->size-1;
}

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

BTreeNode* BTreeNode_insert(BTreeNode* root, const char* key, void* data) {
	KDP* newKDP = NewKDP(key, data);
	BTreeNode* last = root;
	BTreeNode* next;
	while(true) {
		next = BTreeNode_descend(last, key);
		if(next == last) {
			BTreeNode_addKey(next, *newKDP);
			free(newKDP);
			if(next->size <= next->order) {
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

BTreeNode* BTreeNode_delete(BTreeNode* root, const char* key) {
	// Not Implemented yet
	return root;

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
