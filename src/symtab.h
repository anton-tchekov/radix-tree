#ifndef __SYMTABL_H__
#define __SYMTABL_H__

#include <stdint.h>
#include <stddef.h>

#define SYMTAB_DEBUG

/* Symbol table interface */
typedef uint32_t symval;
typedef struct SYM_TAB SymTab;

/**
 * @brief Create a Symbol Table
 *
 * @return Pointer to Symbol Table allocated on the heap
 */
SymTab *symtab_create(void);

/**
 * @brief Frees all the memory of a symbol table
 *
 * @param tab Pointer to Symbol Table
 */
void symtab_destroy(SymTab *tab);

/**
 * @brief Inserts/updates the value for a symbol
 *
 * @param tab Symbol table
 * @param ident Symbol identifier
 * @param value Value
 * @return 1 if the symbol already existed, 0 if it is a new symbol
 */
int symtab_put(SymTab *tab, const char *ident, symval value);

/**
 * @brief Removes a symbol
 *
 * @param tab Symbol table
 * @param ident Symbol identifier
 * @return 1 if the symbol existed and was removed, 0 else
 */
int symtab_remove(SymTab *tab, const char *ident);

/**
 * @brief Gets the value for a symbol
 *
 * @param tab Symbol table
 * @param ident Symbol identifier
 * @param value Pointer for the result
 * @return 1 if the symbol exists, 0 if the symbol was not found
 */
int symtab_get(const SymTab *tab, const char *ident, symval *value);

/**
 * @brief Check if a symbol exists
 *
 * @param tab Symbol table
 * @param ident Symbol identifier
 * @return 1 if the symbol exists, 0 if the symbol was not found
 */
int symtab_exists(const SymTab *tab, const char *ident);

/**
 * @brief Autocomplete the given identifier up to the point
 *        where all contained symbols that have ident as a prefix
 *        are the same
 *
 * @param tab Symbol table
 * @param ident Identifer that should be auto-completed. Because it
 *              is modified in place, ident must point to a buffer
 *              that is at least (SYMBOL_MAX_LENGTH + 1) bytes in size
 * @return 1, if ident was changed
 */
int symtab_complete(const SymTab *tab, char *ident);

/**
 * @brief Calls the provied callback function for every symbol
 *        that has a certain prefix
 *
 * @param tab Symbol table
 * @param ident Prefix Identifer that will be completed and passed
 *              to the callback. Because it is modified in place,
 *              ident must point to a buffer that is as large
 *              as the longest entry in the table
 * @param max_results Maximum number of results (0 for unlimited)
 * @param callback Callback function that is called with the completed
 *                 identifier
 * @return The number of times the callback was called
 */
int symtab_prefix_iter(const SymTab *tab, char *ident, int max_results,
	void (*callback)(char *ident));

#ifdef SYMTAB_DEBUG

/**
 * @brief Print the internal representation of a symbol table
 *        (for debugging purposes only)
 *
 * @param tab Pointer to symbol table
 */
void symtab_print(const SymTab *tab);

#endif

#endif /* __SYMTABL_H__ */
