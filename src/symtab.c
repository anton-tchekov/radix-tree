/**
 * @file    symtab.c
 * @author  Anton Tchekov
 * @version 0.1
 * @date    2023-09-02
 * @brief   Symbol table implementation with a radix tree
 */

#include "symtab.h"

#if SYMTAB_IMPLEMENTATION == SYMTAB_IMPL_TREE

#ifdef SYMTAB_DEBUG
#include <stdio.h>
#endif /* SYMTAB_DEBUG */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * sizeof(SYMTAB):
 *   - 64-bit: 20 bytes
 *   - 32-bit: 12 bytes
 *
 * plus a variable number of bytes for the flexible array member
 */
struct SYMTAB
{
	struct SYMTAB *Next;
	struct SYMTAB *Children;
	int Value;
	char Label[];
};

typedef struct SYMTAB SymNode;

/* --- PRIVATE --- */
static size_t _calc_size(size_t count)
{
	return offsetof(SymNode, Label) + count;
}

static SymNode *_entry_new(size_t label_size)
{
	SymNode *n = malloc(_calc_size(label_size));
	n->Next = NULL;
	n->Children = NULL;
	return n;
}

static inline int _is_leaf(const SymNode *entry)
{
	return entry->Value;
}

static inline int _is_last(const SymNode *entry)
{
	return entry->Next == NULL;
}

static inline int _has_children(const SymNode *entry)
{
	return entry->Children != NULL;
}

static inline int _has_exactly_one_child(const SymNode *entry)
{
	return _has_children(entry) && _is_last(entry->Children);
}

static SymNode *_new_leaf(const char *label, int value)
{
	size_t label_size = strlen(label) + 1;
	SymNode *n = _entry_new(label_size);
	memcpy(n->Label, label, label_size);
	n->Value = value;
	return n;
}

static SymNode *_entry_split(SymNode *entry, char *pos, SymNode **child)
{
	SymNode *second = _new_leaf(pos, entry->Value);
	*pos = '\0';
	second->Children = entry->Children;
	entry->Children = second;
	*child = second;
	return realloc(entry, _calc_size(pos - entry->Label + 1));
}

static SymNode *_entry_split_for_child(
	SymNode *entry, char *pos, const char *label, int value)
{
	SymNode *n = _new_leaf(label, value);
	SymNode *second;
	entry = _entry_split(entry, pos, &second);
	entry->Value = 0;
	second->Next = n;
	return entry;
}

static SymNode *_entry_split_for_prefix(SymNode *entry, char *pos, int value)
{
	SymNode *second;
	entry = _entry_split(entry, pos, &second);
	entry->Value = value;
	return entry;
}

static SymNode *_entry_merge(SymNode *parent, SymNode *child)
{
	size_t parent_len = strlen(parent->Label);
	size_t child_len = strlen(child->Label);
	SymNode *merge = realloc(parent, _calc_size(parent_len + child_len + 1));
	merge->Value = child->Value;
	merge->Children = child->Children;
	memcpy(merge->Label + parent_len, child->Label, child_len + 1);
	free(child);
	return merge;
}

static void _entry_remove(
	SymNode *entry, SymNode **entry_ref,
	SymNode *parent, SymNode **parent_ref)
{
	if(_has_children(entry))
	{
		entry->Value = 0;
		if(_is_last(entry->Children))
		{
			*entry_ref = _entry_merge(entry, entry->Children);
		}
	}
	else
	{
		*entry_ref = entry->Next;
		free(entry);
	}

	if(!_is_leaf(parent) && _has_exactly_one_child(parent))
	{
		*parent_ref = _entry_merge(parent, parent->Children);
	}
}

/* --- PUBLIC --- */
SymNode *symtab_create(int capacity)
{
	SymNode *tab = _entry_new(1);
	tab->Label[0] = '\0';
	tab->Value = 42;
	return tab;
	(void)capacity;
}

void symtab_destroy(SymNode *tab)
{
	if(!tab)
	{
		return;
	}

	symtab_destroy(tab->Next);
	symtab_destroy(tab->Children);
	free(tab);
}

int symtab_put(SymNode *entry, const char *ident, int value)
{
	SymNode **ref = NULL;
	int prev_value = 0;

	assert(value != 0);
	while(entry)
	{
		const char *search = ident;
		char *edge = entry->Label;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search)
			{
				prev_value = entry->Value;
				entry->Value = value;
				entry = NULL;
			}
			else if(!_has_children(entry))
			{
				entry->Children = _new_leaf(search, value);
				entry = NULL;
			}
			else
			{
				ref = &entry->Children;
				entry = entry->Children;
				ident = search;
			}
		}
		else if(edge == entry->Label)
		{
			if(_is_last(entry))
			{
				entry->Next = _new_leaf(search, value);
				entry = NULL;
			}
			else
			{
				ref = &entry->Next;
				entry = entry->Next;
			}
		}
		else
		{
			if(*search)
			{
				*ref = _entry_split_for_child(entry, edge, search, value);
			}
			else
			{
				*ref = _entry_split_for_prefix(entry, edge, value);
			}

			entry = NULL;
		}
	}

	return prev_value;
}

int symtab_remove(SymNode *entry, const char *ident)
{
	int prev_value = 0;
	SymNode *parent = NULL;
	SymNode **entry_ref = NULL;
	SymNode **parent_ref = NULL;
	while(entry)
	{
		const char *search = ident;
		const char *edge = entry->Label;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search && _is_leaf(entry))
			{
				prev_value = entry->Value;
				_entry_remove(entry, entry_ref, parent, parent_ref);
				entry = NULL;
			}
			else
			{
				parent_ref = entry_ref;
				parent = entry;
				entry_ref = &entry->Children;
				entry = *entry_ref;
				ident = search;
			}
		}
		else
		{
			entry_ref = &entry->Next;
			entry = *entry_ref;
		}
	}

	return prev_value;
}

int symtab_get(const SymNode *entry, const char *ident)
{
	int prev_value = 0;
	while(entry)
	{
		const char *search = ident;
		const char *edge = entry->Label;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search && _is_leaf(entry))
			{
				prev_value = entry->Value;
				entry = NULL;
			}
			else
			{
				ident = search;
				entry = entry->Children;
			}
		}
		else
		{
			entry = entry->Next;
		}
	}

	return prev_value;
}

int symtab_complete(const SymNode *entry, char *ident)
{
	int modified = 0;
	while(entry)
	{
		char *search = ident;
		const char *edge = entry->Label;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search)
			{
				entry = NULL;
				modified = 0;
			}
			else
			{
				entry = entry->Children;
				ident = search;
			}
		}
		else if(edge == entry->Label)
		{
			entry = entry->Next;
		}
		else
		{
			while(*edge)
			{
				*search++ = *edge++;
			}

			*search = '\0';
			entry = NULL;
			modified = 1;
		}
	}

	return modified;
}

int symtab_prefix_iter(const SymNode *entry, char *ident, int max_results,
	void *data, void (*callback)(void *data, char *ident))
{
	int num_results = 0;

	/* TODO: Implement prefix iteration functionality */
	(void)entry;
	(void)ident;
	(void)max_results;
	(void)data;
	(void)callback;
	return num_results;
}

#ifdef SYMTAB_DEBUG

static void _nspaces(int n)
{
	while(n--)
	{
		printf(" ");
	}
}

static void _symtab_print(const SymNode *entry, int nesting)
{
	while(entry)
	{
		_nspaces(4 * nesting);
		printf("- %s", entry->Label);
		if(_is_leaf(entry))
		{
			printf(" = %d", entry->Value);
		}

		printf("\n");
		_symtab_print(entry->Children, nesting + 1);
		entry = entry->Next;
	}
}

void symtab_print(const SymNode *tab)
{
	_symtab_print(tab->Children, 0);
}

#endif /* SYMTAB_DEBUG */

#endif /* SYMTAB_IMPLEMENTATION == SYMTAB_IMPL_TREE */
