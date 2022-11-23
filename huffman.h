#if !defined(HUFFMAN_H_INCLUDED)
#define HUFFMAN_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <map>
#include <fstream>
#include <bitset>
#include <chrono>
#include <filesystem>  

using namespace std;

const char filetag[] = "SAC";  // compressed file validation tag

// Structure and constructor for Huffman tree (min heap) node
struct TreeNode
{
	bool isleaf;   // is this a leaf node ('false' marks an internal node)
	char symbol;   // character
	int weight;    // number of characters in file

	TreeNode* leftpointer, * rightpointer; // pointers to left '0' node and right '1' node
	// Constructor
	TreeNode(char symbol, int weight, bool isleaf = false)
	{
		leftpointer = rightpointer = NULL;
		this->isleaf = isleaf;
		this->symbol = symbol;
		this->weight = weight;
	}
};

class HuffmanCode
{
public:
	HuffmanCode() :
		alphabetcount(0),
		totalcharacters(0) {};

	bool CompressFile(std::ifstream& fin, std::ofstream& fout);
	bool ExpandFile(std::ifstream& fin, std::ofstream& fout);
	size_t MapSymbols(std::ifstream&);
	bool GrowHuffmanTree();
	void ClearSymbolMap();
	void ClearHuffmanTree();
	void ClearCodeTable();
	bool GetSymbolMap(std::map<char, int>&);
	void PrintCodeTable();
	unsigned int GetGetAlphabetCount();
	uintmax_t GetTotalCharacters();
	uintmax_t GetTotalCodedBits();
private:
	void MapSymbol(char);
	void MakeCodesFromTree();
	void MakePrefixCodes(TreeNode*, string);
	void SortCodeTable();
	void WriteCompressedFileHeader(std::ofstream&);
	void WriteCompressedFile(std::ifstream&, std::ofstream&);
	bool ReadCompressedFileHeader(std::ifstream&);
	void ReadCompressedFile(std::ifstream&, std::ofstream&);

	// For comparison of two nodes.
	struct compare
	{
		bool operator()(TreeNode* leftnode, TreeNode* rightnode)
		{
			return (leftnode->weight > rightnode->weight);
		}
	};

	// In 'symbolmap' we store each character which appears in the target file together with the
	// number of times (frequency) each character appears. The symbol is used as the map key.
	map<char, int> symbolmap;

	// Note: a Binary Heap can be either minheap or maxheap.
	// In minheap, the tree is complete and the item at root must be minimum among all the items
	// in the heap.This is recursively true for all the other nodes in the binary tree.

	// Create a min heap using STL priority_queue. The 'compare' function, defined above, is ensuring
	// that elements should be arranged according to frequency in the minheap.
	priority_queue <TreeNode*, vector<TreeNode*>, compare> treeheap;

	// create tuple map for final coding/decoding operation
	// symbol, weight and prefix code as a string
	vector<tuple<unsigned char, int, std::string>> codetable;


	// In our minheap we will store a 'TreeNode' which contains two variables, 'character' and 'frequency'.
	// 'Character' represents the character and 'frequency', the number of times the character appears.
	// There are also two pointers, 'leftpointer' and 'rightpointer' which, if we are an internal node,
	// are storing the address of the node which is at the left and the right of the given node.
	struct TreeNode* leftpointer = NULL, * rightpointer = NULL;

	unsigned int alphabetcount = 0;
	uintmax_t totalcharacters = 0;
};


#endif // HUFFMAN_H_INCLUDED
