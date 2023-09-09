# symbol-table

Implementation of a symbol table as a radix tree in C

The goal is to create a datastructure that can do fast lookups, insertions and
deletions, but also fast autocomplete and search suggestions based on a common
prefix.

[Radix Tree on Wikipedia](https://en.wikipedia.org/wiki/Radix_tree)

Memory usage is probably ok, inserting one identfier/value pair allocates at
most one new node that consists of three pointers and one integer. This results
in a memory usage of 32 bytes on a 64-bit, and 16 bytes on a 32-bit system,
plus a variable number of bytes for the string.

- Pointer to the next node on the same level
- Pointer to the first child element
- Pointer to the string label of the node
- Integer for the stored value

There is room for improvement, for example storing the string in the node itself
as a flexible array member will save at least one pointer (+ allocator overhead)
and maybe improve performance due to fewer dynamic memory allocations.

I am somewhat questioning this project since in practice, I am going to use this
for around 1000 entries only, and it doing it as a linear list with a fixed
size for every entry (4 bytes value + 28 bytes identifier for example) is a lot
less complex, easier to work with and the performance difference will probably
be negligible.

## Command line usage

Type `help` for command list.

## TODO
- Implement `symtab_prefix_iter` functionality
- Finish all tests for 100% coverage
