#ifndef _SKV_LLIST_H_
#define _SKV_LLIST_H_

struct _Node {
    struct _Node* prev;
    struct _Node* next;
    struct _Node* first;
    struct _Node* last;
    void* data;
};
typedef struct _Node Node;
typedef Node ListNode;

void new_list_node(Node* n, void* data);
Node* list_first(Node* node);
Node* list_last(Node* node);
void list_append(Node* root, Node* new);
Node* list_remove(Node* root, int index);
Node* list_remove_data(Node* root, void* data);
#endif
