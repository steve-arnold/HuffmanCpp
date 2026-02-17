#include "huffman.h"
#include <array>

using namespace std;

bool HuffmanCode::CompressFile(std::ifstream& fin, std::ofstream& fout)
{
	bool result = false;
	if (MapSymbols(fin))
	{
		GrowHuffmanTree();
		WriteCompressedFileHeader(fout);
		WriteCompressedFile(fin, fout);
		result = true;
	}
	return result;
}

// Build a table of symbols in the file and the number of occurrances (weight)
uintmax_t HuffmanCode::MapSymbols(std::ifstream& fin)
{
	totalcharacters = 0;
	char ch;
	// canonical read loop
	while (fin.get(ch))
	{
		MapSymbol(static_cast<unsigned char>(ch));       // build map of symbols and weight
	}
	// reset stream so that we can read it again
	fin.clear();
	fin.seekg(0);
	return totalcharacters;
}

// Implement MapSymbol using unsigned char
void HuffmanCode::MapSymbol(unsigned char c)
{
	auto it = symbolmap.find(c);
	if (it == symbolmap.end())
	{
		symbolmap[c] = 1;
		alphabetcount++;
	}
	else
	{
		it->second++;
	}
	totalcharacters++;
}

// Build Huffman Tree
bool HuffmanCode::GrowHuffmanTree()
{
	// For each character create a leaf node and insert each leaf node in the heap.
	for (const auto& it : symbolmap) {
		treeheap.push(new TreeNode(static_cast<char>(it.first), it.second, true));
	}

	if (treeheap.empty()) return false; // nothing to do for empty input

	// Iterate while size of min heap doesn't become 1
	while (treeheap.size() != 1)
	{
		//Extract two nodes from the heap.
		leftpointer = treeheap.top();
		treeheap.pop();

		rightpointer = treeheap.top();
		treeheap.pop();

		TreeNode* tmp = new TreeNode('\0', leftpointer->weight + rightpointer->weight);
		tmp->leftpointer = leftpointer;
		tmp->rightpointer = rightpointer;
		treeheap.push(tmp);
	}
	MakeCodesFromTree();
	SortCodeTable();

	return true;
}

// Write file tag, active symbol count, total character count and contents of charactermap to file
void HuffmanCode::WriteCompressedFileHeader(std::ofstream& fout)
{
	// Write the header verification tag and the character counts
	fout.write((const char*)&filetag, sizeof(filetag));
	fout.write((const char*)&alphabetcount, sizeof(alphabetcount));
	fout.write((const char*)&totalcharacters, sizeof(totalcharacters));

	// Write the symbol and weights map to the file
	for (const auto& it : symbolmap) {
		unsigned char symbol = it.first;
		fout.write((const char*)&symbol, sizeof(symbol));
		int weight = it.second;
		fout.write((const char*)&weight, sizeof(weight));
	}
}

void HuffmanCode::WriteCompressedFile(std::ifstream& fin, std::ofstream& fout)
{
	// Build a fast lookup table from byte value to bitstring code
	std::array<std::string, 256> codeLookup;
	for (const auto& t : codetable)
	{
		unsigned char c = std::get<0>(t);
		codeLookup[c] = std::get<2>(t);
	}

	std::string outbits;
	char inchar;
	fin.clear();
	fin.seekg(0);
	while (fin.get(inchar))
	{
		unsigned char uc = static_cast<unsigned char>(inchar);
		const std::string& code = codeLookup[uc];
		outbits += code;

		// Emit whole bytes
		while (outbits.size() >= 8)
		{
			std::bitset<8> b1(outbits.substr(0, 8));
			unsigned char out_byte = static_cast<unsigned char>(b1.to_ulong());
			fout.put(static_cast<char>(out_byte));
			outbits.erase(0, 8);
		}
	}

	// Write remaining bits as last byte (if any)
	if (!outbits.empty())
	{
		// pad the remaining bits with zeros on the right to make a full byte
		while (outbits.size() < 8) outbits.push_back('0');
		std::bitset<8> b1(outbits.substr(0, 8));
		unsigned char out_byte = static_cast<unsigned char>(b1.to_ulong());
		fout.put(static_cast<char>(out_byte));
	}
}

bool HuffmanCode::ExpandFile(std::ifstream& fin, std::ofstream& fout)
{
	if (!ReadCompressedFileHeader(fin))
	{
		return false;
	};
	GrowHuffmanTree();
	ReadCompressedFile(fin, fout);
	return true;
}

// Recover count numbers from front of file
bool HuffmanCode::ReadCompressedFileHeader(std::ifstream& fin)
{
	int count = 0;
	alphabetcount = 0;
	totalcharacters = 0;
	char temptag[sizeof(filetag)]{};
	// Read verification tag
	fin.read(const_cast<char*>(temptag), sizeof(filetag));
	if (string(temptag) != string(filetag))
	{
		fin.close();
		return false;
	}
	// Read the counters from the file into variables
	fin.read(reinterpret_cast<char*>(&alphabetcount), sizeof(alphabetcount));
	fin.read(reinterpret_cast<char*>(&totalcharacters), sizeof(totalcharacters));

	// Read the symbol and weight data and construct symbolmap
	ClearSymbolMap();
	char tempsymbol = 0;
	int tempcount = 0;
	for (unsigned int count = 0; count < alphabetcount; count++)
	{
		fin.read(reinterpret_cast<char*>(&tempsymbol), sizeof(char));
		fin.read(reinterpret_cast<char*>(&tempcount), sizeof(int));
		symbolmap.insert(make_pair(static_cast<unsigned char>(tempsymbol), tempcount));
	}
	return true;
}

// Process the compressed section of the file
void HuffmanCode::ReadCompressedFile(std::ifstream& fin, std::ofstream& fout)
{
	// Reposition start point past the header information
	size_t startpos = (sizeof(filetag) + sizeof(alphabetcount) + sizeof(totalcharacters) + alphabetcount * (sizeof(char) + sizeof(int)));
	fin.seekg(startpos);

	unsigned char inbyte;
	unsigned char mask = 0x80u;
	unsigned char code = 0;
	uintmax_t totalbytes = GetTotalCodedBits() / 8;		// number of complete bytes in the encoded file
	unsigned char extrabits = GetTotalCodedBits() % 8;  // odd bits at the end of the file
	TreeNode* node = treeheap.top();
	for (uintmax_t index = 0; index < totalbytes; index++)
	{
		int r = fin.get();
		if (r == EOF) break;
		inbyte = static_cast<unsigned char>(r);
		for (unsigned char offset = 0; offset < 8; offset++)
		{
			code = (inbyte & mask) ? 1 : 0;
			// look up tree
			node = (code) ? node->rightpointer : node->leftpointer;
			if (node->isleaf)
			{
				fout.write(&node->symbol, 1);
				node = treeheap.top();
			}
			inbyte = static_cast<unsigned char>(inbyte << 1);
		}
	}
	if (extrabits)  // any spare bits that are not a whole byte
	{
		int r = fin.get();
		if (r != EOF)
		{
			inbyte = static_cast<unsigned char>(r);
			for (unsigned char offset = 0; offset < extrabits; offset++)
			{
				code = (inbyte & mask) ? 1 : 0;
				node = (code) ? node->rightpointer : node->leftpointer;
				if (node->isleaf)
				{
					fout.write(&node->symbol, 1);
					node = treeheap.top();
				}
				inbyte = static_cast<unsigned char>(inbyte << 1);
			}
		}
	}
}

// Clear character map
void HuffmanCode::ClearSymbolMap()
{
	symbolmap = {};
}

// Recursive delete
void HuffmanCode::DeleteTree(TreeNode* node)
{
	if (!node) return;
	DeleteTree(node->leftpointer);
	DeleteTree(node->rightpointer);
	delete node;
}

// Clear the tree (delete full structure)
void HuffmanCode::ClearHuffmanTree()
{
	if (!treeheap.empty())
	{
		TreeNode* root = treeheap.top();
		DeleteTree(root);
	}
	while (!treeheap.empty()) {
		treeheap.pop();
	}
}

// Clear all tuples
void HuffmanCode::ClearCodeTable()
{
	codetable.clear();
}

// Make the Huffman tree
void HuffmanCode::MakeCodesFromTree()
{
	MakePrefixCodes(treeheap.top(), "");
}

// Generate Huffman Prefix Codes as strings
void HuffmanCode::MakePrefixCodes(TreeNode* node, string str)
{
	if (!node)  // If node is Null then return.
	{
		return;
	}
	// If the node's leaf is true, that means it's not an internal node and save the string.
	if (node->isleaf)
	{
		codetable.push_back(make_tuple(static_cast<unsigned char>(node->symbol), node->weight, str));
	}
	MakePrefixCodes(node->leftpointer, str + "0");
	MakePrefixCodes(node->rightpointer, str + "1");
}

// Sort tuple by value of second element
bool sortbysec(const std::tuple<unsigned char, int, std::string>& a, const std::tuple<unsigned char, int, std::string>& b)
{
	return (get<1>(b) < get<1>(a));
}

// Sort tuple based on symbol weight
void HuffmanCode::SortCodeTable()
{
	sort(codetable.begin(), codetable.end(), sortbysec);
}

// Print Huffman Codes
void HuffmanCode::PrintCodeTable()
{
	for (const auto& it : codetable)
	{
		cout << static_cast<char>(std::get<0>(it)) << "\t: " << std::get<1>(it) << "\t: " << std::get<2>(it) << endl;
	}
}

// Give a copy of CharacterMap when requested, if it has been built
bool HuffmanCode::GetSymbolMap(map<unsigned char, int>& tempmap)
{
	bool result = false;

	if (symbolmap.size())
	{
		tempmap = symbolmap;
		result = true;
	}
	return result;
}

// Return number of symbols in the CharacterMap
uint16_t HuffmanCode::GetAlphabetCount()
{
	return alphabetcount;
}

// Return total number of characters in the file
uintmax_t HuffmanCode::GetTotalCharacters()
{
	return totalcharacters;
}

// Return total number of bits in the coded file
uintmax_t HuffmanCode::GetTotalCodedBits()
{
	size_t totalbits = 0;
	for (const auto& it : codetable)
	{
		// multiply number of times symbol appears by the number of bits in the prefix code
		totalbits += (std::get<1>(it) * (std::get<2>(it)).length());
	}
	return totalbits;
}
