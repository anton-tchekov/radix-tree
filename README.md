# symbol-table

Implementation of a symbol table as a radix tree in C

The goal is to create a datastructure that can do fast lookups, insertions and
deletions, but also fast autocomplete and search suggestions based on a common
prefix.

[Radix Tree on Wikipedia](https://en.wikipedia.org/wiki/Radix_tree)

Memory usage is probably ok, inserting one identfier/value pair allocates at
most one new node that consists of two pointers, one integer and a flexible
array member for the string label of the node. This results in a memory usage
of 20 bytes on a 64-bit, and 12 bytes on a 32-bit system, plus a variable
number of bytes for the string.

- Pointer to the next node on the same level
- Pointer to the first child element
- Integer for the stored value
- Flexible array member for string label

I am somewhat questioning this project since in practice, I am going to use this
for around 1000 entries only, and it doing it as a linear list with a fixed
size for every entry (4 bytes value + 28 bytes identifier for example) is a lot
less complex, uses less memory, is easier to work with and the performance
difference will probably be negligible.

## Command line usage

Type `help` for command list.

## TODO
- Implement `symtab_prefix_iter` functionality
- Finish all tests for 100% coverage
