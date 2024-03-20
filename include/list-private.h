#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "entry.h"
#include "list.h"

struct node_t {
	struct entry_t *entry;
	struct node_t  *next; 
};

struct list_t {
	int size;
	struct node_t *head;
};

/**
 * Creates a new node, allocating necessary memory. Returns NULL if an error has occurred
*/
struct node_t* create_node(struct entry_t *entry);


/**
 * Deletes a node, and its contents, freeing up allocated memory
*/
int delete_node(struct node_t* node);

/**
 * Auxiliary function for finding an entry with a  given key
*/
struct entry_t* find_entry(struct node_t* node, char* key);

/**
 * Auxiliary function for adding an entry. Starting node must be tested before. Returns -1 in case of an error,
 * 0 if a new node has been created or 1 if an already existing entry has been replaced 
*/
int add_entry (struct node_t* node, struct entry_t *entry);

/**
 * Auxiliary function for removing an entry. Starting node must be tested before.
 * Returns -1 in case of error, 0 if the entry has been removed or 1 if no entry with the given
 * key was found
*/
int remove_node(struct node_t* node, char* key);

/**
 * Inserts an entry between two nodes. Returns 0 if successfull and -1 if an error
 * occurred
*/
int insert(struct node_t* current, struct node_t* next, struct entry_t* entry);

#endif
