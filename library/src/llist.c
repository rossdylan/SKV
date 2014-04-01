#include "llist.h"
#include "string.h"


void new_list_node(Node* n, void* data) {
	memset(n, 0, sizeof(Node));
	n->prev = NULL;
	n->next = NULL;
	n->first = NULL;
	n->last = n;
	n->data = data;
}

Node* list_first(Node* root) {
	return root->first;
}

Node* list_last(Node* root) {
	return root->last;
}

void list_append(Node* root, Node* new) {
	Node* old_last = root->last;
	old_last->next = new;
	new->prev = old_last;
	new->first = root;
	root->last = new;
}

Node* list_remove(Node* root, int index) {
	Node* prev = root;
	Node* next;
	int count = 0;
	Node* to_remove = NULL;
	while((next = prev->next) != NULL) {
		if(count == index) {
			to_remove = next;
			break;
		}
		prev = next;
		count++;
	}
	if(to_remove == NULL) {
		return NULL;
	}
	Node* before = to_remove->prev;
	Node* after = to_remove->next;
	if(after != NULL) {
		after->prev = before;
	}
	before->next = after;
	return to_remove;
}

Node* list_remove_data(Node* root, void* data) {
	Node* prev = root;
	Node* next;
	Node* to_remove = NULL;
	while((next = prev->next) != NULL) {
		if(next->data == data) {
			to_remove = next;
			break;
		}
	}
	if(to_remove == NULL) {
		return NULL;
	}
	Node* before = to_remove->prev;
	Node* after = to_remove->next;
	if(after != NULL) {
		after->prev = before;
	}
	before->next = after;
	return to_remove;
}
