# symbol-table

Implementation of a symbol table as a radix tree in C

The goal is to create a datastructure that can do fast lookups, insertions and deletions,
but also fast autocomplete and search suggestions based on a common prefix.

[Radix Tree on Wikipedia](https://en.wikipedia.org/wiki/Radix_tree)

Memory usage is ok probably, inserting one identfier/value pair introduces 1 new node
maximum, that consists of 3 pointers and 1 integer (the stored value)

32 bytes on 64-bit

16 bytes on 32-bit

plus a variable number of bytes for the end part of the identifier that is not
a shared prefix.

There is room for improvement, for example storing the string in the node itself
as a flexible array member will save at least 4 bytes (+ allocator overhead) and
improve performance due to fewer dynamic memory allocations.

## Command line usage

Type `help` for command list.

## TODO
- Fix the very broken `symtab_remove` function
- Implement `symtab_prefix_iter` functionality (maybe change interface)
- Finish all tests for 100% coverage
