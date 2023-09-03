/**
 * @file    symtab.c
 * @author  Anton Tchekov
 * @version 0.1
 * @date    2023-09-02
 * @brief   Symbol table implementation with a radix tree
 */

#include "symtab.h"

#if SYMTAB_IMPLEMENTATION == SYMTAB_IMPL_TREE

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct SYMTAB
{
	struct SYMTAB *Next;
	struct SYMTAB *Children;
	char *Piece;
	int Value;
} SymEntry;

/* --- PRIVATE --- */
static SymEntry *_new_entry(void)
{
	SymEntry *n = malloc(sizeof(SymEntry));
	n->Next = NULL;
	n->Children = NULL;
	return n;
}

static void _free_entry(SymEntry *entry)
{
	free(entry->Piece);
	free(entry);
}

static char *_strdup(const char *s)
{
	size_t len = strlen(s);
	char *p = malloc(len + 1);
	strcpy(p, s);
	return p;
}

static void _append(char *dst, const char *src)
{
	size_t dst_len = strlen(dst);
	dst = realloc(dst, dst_len + strlen(src) + 1);
	strcpy(dst + dst_len, src);
}

static inline int _is_leaf(const SymEntry *entry)
{
	return entry->Value;
}

static inline int _is_no_leaf(const SymEntry *entry)
{
	return !_is_leaf(entry);
}

static inline int _get_value(const SymEntry *entry)
{
	return entry->Value;
}

static inline void _set_leaf_value(SymEntry *entry, int value)
{
	entry->Value = value;
}

static inline int _is_last(const SymEntry *entry)
{
	return entry->Next == NULL;
}

static inline int _has_children(const SymEntry *entry)
{
	return entry->Children != NULL;
}

static inline int _has_no_children(const SymEntry *entry)
{
	return !_has_children(entry);
}

static inline int _has_exactly_one_child(const SymEntry *entry)
{
	return _has_children(entry) && _is_last(entry->Children);
}

static void _new_single_leaf(SymEntry *parent, const char *label, int value)
{
	SymEntry *n = _new_entry();
	n->Piece = _strdup(label);
	_set_leaf_value(n, value);
	parent->Children = n;
}

static void _new_last_leaf(SymEntry *prev, const char *label, int value)
{
	SymEntry *n = _new_entry();
	n->Piece = _strdup(label);
	_set_leaf_value(n, value);
	prev->Next = n;
}

static SymEntry *_entry_split(SymEntry *entry, char *pos)
{
	SymEntry *second = _new_entry();
	second->Value = entry->Value;
	second->Piece = _strdup(pos);
	second->Children = entry->Children;
	entry->Children = second;
	*pos = '\0';
	entry->Piece = realloc(entry->Piece, pos - entry->Piece + 1);
	return second;
}

static void _entry_split_for_child(
	SymEntry *entry, char *pos, const char *label, int value)
{
	SymEntry *n, *second;

	second = _entry_split(entry, pos);
	entry->Value = 0;
	n = _new_entry();
	n->Piece = _strdup(label);
	_set_leaf_value(n, value);
	second->Next = n;
}

static void _entry_split_for_prefix(SymEntry *entry, char *pos, int value)
{
	_entry_split(entry, pos);
	_set_leaf_value(entry, value);
}

static void _merge_entry(SymEntry *parent, SymEntry *child)
{
	parent->Value = child->Value;
	parent->Children = child->Children;
	_append(parent->Piece, child->Piece);
	_free_entry(child);
}

static void _remove_entry(SymEntry *entry, SymEntry *parent, SymEntry *prev)
{
	/* TODO: Fix Bugs */
	if(_has_children(entry))
	{
		entry->Value = 0;
		if(_is_last(entry->Children))
		{

		}
	}
	else
	{
		if(prev)
		{
			prev->Next = entry->Next;
		}
		else
		{
			parent->Children = entry->Next;
		}

		_free_entry(entry);
	}

	if(_has_exactly_one_child(parent) && _is_no_leaf(parent))
	{
		_merge_entry(parent, parent->Children);
	}
}

static void _symtab_destroy(SymTab *tab)
{
	if(!tab)
	{
		return;
	}

	_symtab_destroy(tab->Next);
	_symtab_destroy(tab->Children);
	_free_entry(tab);
}

/* --- PUBLIC --- */
SymTab *symtab_create(int capacity)
{
	SymEntry *tab = _new_entry();
	tab->Piece = "";
	tab->Value = 0;
	return tab;
	(void)capacity;
}

void symtab_destroy(SymTab *tab)
{
	_symtab_destroy(tab->Children);
	free(tab);
}

int symtab_put(SymEntry *entry, const char *ident, int value)
{
	int val = 0;
	assert(value != 0);

	while(entry)
	{
		const char *search = ident;
		char *edge = entry->Piece;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search)
			{
				val = _get_value(entry);
				_set_leaf_value(entry, value);
				entry = NULL;
			}
			else if(_has_no_children(entry))
			{
				_new_single_leaf(entry, search, value);
				entry = NULL;
			}
			else
			{
				entry = entry->Children;
				ident = search;
			}
		}
		else if(edge == entry->Piece)
		{
			if(_is_last(entry))
			{
				_new_last_leaf(entry, search, value);
				entry = NULL;
			}
			else
			{
				entry = entry->Next;
			}
		}
		else
		{
			if(*search)
			{
				_entry_split_for_child(entry, edge, search, value);
			}
			else
			{
				_entry_split_for_prefix(entry, edge, value);
			}

			entry = NULL;
		}
	}

	return val;
}

int symtab_remove(SymEntry *entry, const char *ident)
{
	int val = 0;
	SymTab *parent = NULL;
	SymTab *prev = NULL;
	while(entry)
	{
		const char *search = ident;
		const char *edge = entry->Piece;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search && _is_leaf(entry))
			{
				val = _get_value(entry);
				_remove_entry(entry, parent, prev);
				entry = NULL;
			}
			else
			{
				prev = NULL;
				parent = entry;
				entry = entry->Children;
				ident = search;
			}
		}
		else
		{
			prev = entry;
			entry = entry->Next;
		}
	}

	return val;
}

int symtab_get(const SymEntry *entry, const char *ident)
{
	int val = 0;
	while(entry)
	{
		const char *search = ident;
		const char *edge = entry->Piece;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search && _is_leaf(entry))
			{
				val = _get_value(entry);
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

	return val;
}

int symtab_complete(const SymTab *entry, char *ident)
{
	int modified = 0;
	while(entry)
	{
		char *search = ident;
		const char *edge = entry->Piece;
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
		else if(edge == entry->Piece)
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

int symtab_prefix_iter(const SymTab *entry, char *ident, int max_results,
	void *data, void (*callback)(void *data, char *ident))
{
	int num_results = 0;

	/* TODO: Implement prefix iteration functionality */
	(void)entry;
	(void)ident;
	(void)max_results;
	(void)callback;
	return 0;
}

#ifdef SYMTAB_DEBUG

#include <stdio.h>

static void _nspaces(int n)
{
	while(n--)
	{
		printf(" ");
	}
}

static void _symtab_print(const SymEntry *entry, int nesting)
{
	while(entry)
	{
		_nspaces(4 * nesting);
		printf("- %s", entry->Piece);
		if(_is_leaf(entry))
		{
			printf(" = %d", _get_value(entry));
		}

		printf("\n");
		_symtab_print(entry->Children, nesting + 1);
		entry = entry->Next;
	}
}

void symtab_print(const SymTab *tab)
{
	_symtab_print(tab->Children, 0);
}

#endif /* SYMTAB_DEBUG */

#endif /* SYMTAB_IMPLEMENTATION == SYMTAB_IMPL_TREE */