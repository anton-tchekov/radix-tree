/**
 * @file    symtab.h
 * @author  Anton Tchekov
 * @version 0.1
 * @date    2023-09-02
 * @brief   Symbol table implementation as a linear list
 */

#include "symtab.h"

#if SYMTAB_IMPLEMENTATION == SYMTAB_IMPL_ARRAY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct NODE
{
	int Value;
	char Identifer[28];
} Node;

typedef struct SYMTAB
{
	Node *Buffer;
	int Count;
	int Capacity;
} SymTab;

/* --- PRIVATE --- */
static inline int _node_match(Node *node, const char *ident)
{
	return !strcmp(node->Identifer, ident);
}

static Node *_tab_find_free(SymTab *tab)
{
	int i;
	int capacity = tab->Capacity;

	if(tab->Count >= capacity)
	{
		return NULL;
	}

	for(i = 0; i < capacity; ++i)
	{
		Node *node = tab->Buffer + i;
		if(!node->Value)
		{
			return node;
		}
	}

	return NULL;
}

static Node *_tab_find(const SymTab *tab, const char *ident)
{
	int i = 0;
	int checked = 0;
	int count = tab->Count;
	int capacity = tab->Capacity;

	while((i < capacity) && (checked < count))
	{
		Node *node = tab->Buffer + i;
		if(node->Value)
		{
			if(_node_match(node, ident))
			{
				return node;
			}

			++checked;
		}

		++i;
	}

	return NULL;
}

static inline void _ident_common(char *ident, const char *stored)
{
	while(*ident == *stored)
	{
		++ident;
		++stored;
	}

	*ident = '\0';
}

static inline int _starts_with_count(const char *str, const char *prefix)
{
	int count = 0;
	while(*str == *prefix)
	{
		++str;
		++prefix;
		++count;
	}

	return !*prefix ? count : 0;
}

static Node *_find_prefix(Node *nodes, int len, const char *prefix)
{

}

/* --- PUBLIC --- */
SymTab *symtab_create(int capacity)
{
	SymTab *tab = malloc(sizeof(*tab));
	tab->Count = 0;
	tab->Capacity = capacity;
	tab->Buffer = calloc(capacity, sizeof(*tab->Buffer));
	return tab;
}

void symtab_destroy(SymTab *tab)
{
	free(tab->Buffer);
	free(tab);
}

int symtab_put(SymTab *tab, const char *ident, int value)
{
	int prev_value = 0;
	Node *node = _tab_find(tab, ident);
	if(node)
	{
		prev_value = node->Value;
	}
	else
	{
		node = _tab_find_free(tab);
		strcpy(node->Identifer, ident);
		++tab->Count;
	}

	node->Value = value;
	return prev_value;
}

int symtab_remove(SymTab *tab, const char *ident)
{
	int prev_value = 0;
	Node *node = _tab_find(tab, ident);
	if(node)
	{
		prev_value = node->Value;
		node->Value = 0;
		--tab->Count;
	}

	return prev_value;
}

int symtab_get(const SymTab *tab, const char *ident)
{
	int value = 0;
	Node *node = _tab_find(tab, ident);
	if(node)
	{
		value = node->Value;
	}

	return value;
}

int symtab_complete(const SymTab *tab, char *ident)
{
#if 0
	int i = 0;
	int processed = 0;
	int count = tab->Count;
	int capacity = tab->Capacity;

	while((i < capacity) && (processed < count))
	{
		Node *node = &tab->Buffer[i];
		if(node->Value)
		{
			_ident_common(ident, node->Identifer);
			++processed;
		}

		++i;
	}
#endif

	return 0;
}

int symtab_prefix_iter(const SymTab *tab, char *ident, int max_results,
	void *data, void (*callback)(void *data, char *ident))
{
	int i = 0;
	int checked = 0;
	int num_results = 0;
	int count = tab->Count;
	int capacity = tab->Capacity;

	while((i < capacity) && (checked < count) && (num_results < max_results))
	{
		Node *node = tab->Buffer + i;
		if(node->Value)
		{
			int common = _starts_with_count(node->Identifer, ident);
			if(common)
			{
				strcpy(ident + common, node->Identifer + common);
				callback(data, ident);
				ident[common] = '\0';
				++num_results;
			}

			++checked;
		}

		++i;
	}

	return num_results;
}

#ifdef SYMTAB_DEBUG

void symtab_print(const SymTab *tab)
{
	int i = 0;
	int printed = 0;
	int count = tab->Count;
	int capacity = tab->Capacity;

	while((i < capacity) && (printed < count))
	{
		Node *node = &tab->Buffer[i];
		int value = node->Value;
		if(value)
		{
			printf("- %s = %d\n", node->Identifer, value);
			++printed;
		}

		++i;
	}
}

#endif /* SYMTAB_DEBUG */

#endif /* SYMTAB_IMPLEMENTATION == SYMTAB_IMPL_ARRAY */
