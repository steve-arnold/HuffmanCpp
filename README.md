# The Huffman Coding Algorithm
Huffman Coding is a method of constructing minimum redundancy codes. The purpose is to reduce the size of a text file. 
It is described by its author David A. Huffman as -<br>
"An optimum method of coding an ensemble of messages consisting of a finite number of members is developed. A
minimum-redundancy code is one constructed in such a way that the average number of coding digits per message is minimized."

## Encoding process

* Scan the input file to build up a list of symbols contained and the number of times each is present in the file (symbolmap)
* Use symbolmap to build a priority queue containing the Huffman Tree (symbolmap)
* Scan the Huffman tree and find all the leaf nodes and store symbol, weight and binary prefix code (as a string) and store in a tuple map(codetable)
* Sort the tuple with highest appearance characters given highest priority. (this step is not really needed)
* Write a verification tag to the start of the compressed file.
* Write the number of active symbols and the total input file length together with the symbolmap to the start of the output file.
* Scan the input file again, character by character and encode each using the Huffman Tree.

## Decoding process

* Read the file header and verify it is a compatible compressed file. (Code in header is SAC)
* Read original file profile (number of bytes, number and weight of characters)
* Rebuild the Huffman tree
* From the tuple map determine the number of bytes and any extra bits (less than a byte) we need to decode
* Read file one byte at a time and decode to get character by following nodes on the huffman tree
* Read last byte and decode final bits.

## Test Program

The main() function is a test program which takes a file name from the user. This file is then compressed into a file with the same name and .cmp extension.<br>
This file is then expanded into "uncompressed.txt".<br>

At both stages a print of the code table is made.

## Notes
Requires C++ 17 as it uses std::filesystem.<br>
This program build on C++ version 17 under Visual Studio 2022.<br>
Error checking is minimal.

## Source code

This repository contains the following source code and data files:

* `huffman.cpp` - C++ programming language implementation.
* `huffman.h`   - C++ header file.
* `main.cpp`    - C++ programming language test program.
* `12mb.txt`    - A text file for testing. (This code has been tested with a 300Mbyte text file)
