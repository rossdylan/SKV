#ifndef _SKV_BTREE_H_
#define _SKV_BTREE_H_

typedef struct {
	const char* Key;
	void* Data;
} KDP;


struct _BTreeNode {
	int order;
	int size;
    bool leaf; // is this node a leaf
	KDP* keys;
	struct _BTreeNode** leaves;
	struct _BTreeNode* parent;
};

typedef struct _BTreeNode BTreeNode;

typedef struct {
	BTreeNode* left;
	KDP middle;
	BTreeNode* right;
} Split_t;


KDP* NewKDP(const char* key, void* data);
BTreeNode* NewBTreeNode(int order);
void BTreeNode_free(BTreeNode* node);
void BTreeNode_freeRecursively(BTreeNode* node);
void BTreeNode_print(BTreeNode* node);
bool BTreeNode_isEmpty(BTreeNode* node);
BTreeNode* BTreeNode_descend(BTreeNode* root, const char* key);
void* BTreeNode_getData(BTreeNode* node, const char* key);
int BTreeNode_addKey(BTreeNode* node, KDP kdp);
void* BTreeNode_search(BTreeNode* node, const char* key);
Split_t* BTreeNode_split(BTreeNode* node);
BTreeNode* BTreeNode_findRoot(BTreeNode* node);
BTreeNode* BTreeNode_promote(BTreeNode* node);
BTreeNode* BTreeNode_insert(BTreeNode* root, const char* key, void* data);
BTreeNode* BTreeNode_delete(BTreeNode* node, const char* key);
void BTreeNode_fillGaps(BTreeNode* node, int index);

#endif
