/**
 * @file    symtab.h
 * @author  Anton Tchekov
 * @version 0.1
 * @date    2023-09-02
 * @brief   Symbol table interface
 */

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include <stdint.h>
#include <stddef.h>

#define SYMTAB_DEBUG

#define SYMTAB_IMPL_TREE  1
#define SYMTAB_IMPL_ARRAY 2

#define SYMTAB_IMPLEMENTATION SYMTAB_IMPL_TREE

/* Symbol table interface */
typedef struct SYMTAB SymTab;

/**
 * @brief Create a symbol table
 *
 * @return Pointer to symbol table allocated on the heap
 */
SymTab *symtab_create(int capacity);

/**
 * @brief Frees all the memory of a symbol table
 *
 * @param tab Pointer to Symbol Table
 */
void symtab_destroy(SymTab *tab);

/**
 * @brief Inserts or updates the value for a symbol
 *
 * @param tab Symbol table
 * @param ident Symbol identifier
 * @param value Value
 * @return Previous symbol value if it already existed, 0 if it is new
 */
int symtab_put(SymTab *tab, const char *ident, int value);

/**
 * @brief Removes a symbol
 *
 * @param tab Symbol table
 * @param ident Symbol identifier
 * @return Symbol value if it existed and was removed, 0 otherwise
 */
int symtab_remove(SymTab *tab, const char *ident);

/**
 * @brief Gets the value for a symbol
 *
 * @param tab Symbol table
 * @param ident Symbol identifier
 * @return Symbol value or 0 if the symbol was not found
 */
int symtab_get(const SymTab *tab, const char *ident);

/**
 * @brief Autocomplete the given identifier up to the point
 *        where all contained symbols that have `ident` as a prefix
 *        are the same
 *
 * @param tab Symbol table
 * @param ident Identifer that should be auto-completed. Because it
 *              is modified in place, `ident` should point to a buffer
 *              that is large enough to hold the longest entry in the table
 * @return 1, if `ident` was modified
 */
int symtab_complete(const SymTab *tab, char *ident);

/**
 * @brief Calls the provied callback function for every symbol
 *        that has a certain prefix
 *
 * @param tab Symbol table
 * @param ident Prefix identifer that will be completed and passed
 *              to the callback. Because it is modified in place,
 *              `ident` should point to a buffer that is large enough to hold
 *              the longest entry in the table
 * @param max_results Maximum number of results (0 for unlimited)
 * @param data Pointer to custom data that is passed to the callback
 * @param callback Callback function that is called with the completed
 *                 identifier
 * @return The number of times the callback was called
 */
int symtab_prefix_iter(const SymTab *tab, char *ident, int max_results,
	void *data, void (*callback)(void *data, char *ident));

#ifdef SYMTAB_DEBUG

/**
 * @brief Print the internal representation of a symbol table
 *        (for debugging purposes only)
 *
 * @param tab Pointer to symbol table
 */
void symtab_print(const SymTab *tab);

#endif

#endif /* __SYMTAB_H__ */
