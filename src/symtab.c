/* Symbol table implementation with a radix tree */
#include "symtabl.h"
#include <stdlib.h>
#include <string.h>

typedef struct SYM_TAB
{
	struct SYM_TAB *Next;
	struct SYM_TAB *Children;
	char *Piece;
	symval Value;
} SymEntry;

/* --- PRIVATE --- */
static SymEntry *_new_entry(void)
{
	SymEntry *n;

	n = malloc(sizeof(SymEntry));
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
	size_t len;
	char *p;

	len = strlen(s);
	p = malloc(len + 1);
	strcpy(p, s);
	return p;
}

static void _append(char *dst, const char *src)
{
	size_t dst_len;

	dst_len = strlen(dst);
	dst = realloc(dst, dst_len + strlen(src) + 1);
	strcpy(dst + dst_len, src);
}

static inline int _is_leaf(const SymEntry *entry)
{
	return entry->Value & 1;
}

static inline int _is_no_leaf(const SymEntry *entry)
{
	return !_is_leaf(entry);
}

static inline symval _get_value(const SymEntry *entry)
{
	return entry->Value >> 1;
}

static inline void _set_leaf_value(SymEntry *entry, symval value)
{
	entry->Value = (value << 1) | 1;
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

static void _new_single_leaf(SymEntry *parent, const char *label, symval value)
{
	SymEntry *n;

	n = _new_entry();
	n->Piece = _strdup(label);
	_set_leaf_value(n, value);
	parent->Children = n;
}

static void _new_last_leaf(SymEntry *prev, const char *label, symval value)
{
	SymEntry *n;

	n = _new_entry();
	n->Piece = _strdup(label);
	_set_leaf_value(n, value);
	prev->Next = n;
}

static SymEntry *_entry_split(SymEntry *entry, char *pos)
{
	SymEntry *second;

	second = _new_entry();
	second->Value = entry->Value;
	second->Piece = _strdup(pos);
	second->Children = entry->Children;
	entry->Children = second;
	*pos = '\0';
	entry->Piece = realloc(entry->Piece, pos - entry->Piece + 1);
	return second;
}

static void _entry_split_for_child(
	SymEntry *entry, char *pos, const char *label, symval value)
{
	SymEntry *n, *second;

	second = _entry_split(entry, pos);
	entry->Value = 0;
	n = _new_entry();
	n->Piece = _strdup(label);
	_set_leaf_value(n, value);
	second->Next = n;
}

static void _entry_split_for_prefix(SymEntry *entry, char *pos, symval value)
{
	_entry_split(entry, pos);
	_set_leaf_value(entry, value);
}

static void _merge_entry(SymEntry *parent, SymEntry *child)
{
	parent->Value = child->Value;
	_append(parent->Piece, child->Piece);
	_free_entry(child);
	parent->Children = NULL;
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
SymTab *symtab_create(void)
{
	SymEntry *tab;

	tab = _new_entry();
	tab->Piece = "";
	tab->Value = 0;
	return tab;
}

void symtab_destroy(SymTab *tab)
{
	_symtab_destroy(tab->Children);
	free(tab);
}

int symtab_put(SymEntry *entry, const char *ident, symval value)
{
	char *edge;
	const char *search;

	while(entry)
	{
		search = ident;
		edge = entry->Piece;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search)
			{
				_set_leaf_value(entry, value);
				return 1;
			}

			if(_has_no_children(entry))
			{
				_new_single_leaf(entry, search, value);
				return 0;
			}

			entry = entry->Children;
			ident = search;
		}
		else if(edge == entry->Piece)
		{
			if(_is_last(entry))
			{
				_new_last_leaf(entry, search, value);
				return 0;
			}

			entry = entry->Next;
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

			return 0;
		}
	}

	return 0;
}

int symtab_remove(SymEntry *entry, const char *ident)
{
	SymTab *parent, *prev;
	const char *edge, *search;

	parent = NULL;
	prev = NULL;
	while(entry)
	{
		search = ident;
		edge = entry->Piece;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search && _is_leaf(entry))
			{
				_remove_entry(entry, parent, prev);
				return 1;
			}

			prev = NULL;
			parent = entry;
			entry = entry->Children;
			ident = search;
		}
		else
		{
			prev = entry;
			entry = entry->Next;
		}
	}

	return 0;
}

int symtab_get(const SymEntry *entry, const char *ident, symval *value)
{
	const char *edge, *search;

	while(entry)
	{
		search = ident;
		edge = entry->Piece;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search && _is_leaf(entry))
			{
				*value = _get_value(entry);
				return 1;
			}

			entry = entry->Children;
			ident = search;
		}
		else
		{
			entry = entry->Next;
		}
	}

	return 0;
}

int symtab_exists(const SymTab *tab, const char *ident)
{
	symval dummy;
	return symtab_get(tab, ident, &dummy);
}

int symtab_complete(const SymTab *entry, char *ident)
{
	char *search;
	const char *edge;

	while(entry)
	{
		search = ident;
		edge = entry->Piece;
		while(*edge && *search && *edge == *search)
		{
			++edge;
			++search;
		}

		if(!*edge)
		{
			if(!*search)
			{
				return 0;
			}

			entry = entry->Children;
			ident = search;
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
			return 1;
		}
	}

	return 0;
}

int symtab_prefix_iter(const SymTab *entry, char *ident, int max_results,
	void (*callback)(char *ident))
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

#endif
