#include "huffman.h"


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
};

// Build a table of symbols in the file and the number of occurrances (weight)
size_t HuffmanCode::MapSymbols(std::ifstream& fin)
{
	totalcharacters = 0;
	char ch;
	while (fin.get(ch))
	{
		MapSymbol(ch);       // build map of symbols and weight
	}
	// reset stream so that we can read it again
	fin.clear();
	fin.seekg(0);
	return totalcharacters;
};

// Build Huffman Tree
bool HuffmanCode::GrowHuffmanTree()
{
	//   struct TreeNode* leftpointer, * rightpointer;
	//   priority_queue <TreeNode*, vector<TreeNode*>, compare> treeheap;

	// For each character create a leaf node and insert each leaf node in the heap.
	// Traverse the map
	for (const auto& it : symbolmap) {
		treeheap.push(new TreeNode(it.first, it.second, true));
	}

	// Iterate while size of min heap doesn't become 1
	while (treeheap.size() != 1)
	{
		//Extract two nodes from the heap.
		leftpointer = treeheap.top();
		treeheap.pop();

		rightpointer = treeheap.top();
		treeheap.pop();

		// Create a new internal node with weight equal to the sum of two extracted nodes.
		// By default 'leaf' will be false marking it as an internal node.
		// Assign symbol '\0' to this node and make the two extracted nodes as the left and right children. 
		// Add this node to the heap. 
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
	// This data is used to rebuild the Huffman tree for the decode

	// Iterate over the map until end.
	for (const auto& it : symbolmap) {
		// Accessing KEY (symbol) from element pointed by it.
		char symbol = it.first;
		fout.write((const char*)&symbol, sizeof(symbol));
		// Accessing VALUE (weight) from element pointed by it.
		int weight = it.second;
		fout.write((const char*)&weight, sizeof(weight));
	}
};

void HuffmanCode::WriteCompressedFile(std::ifstream& fin, std::ofstream& fout)
{
	string outbyte = "";
	char symbol;
	fin.clear();
	fin.seekg(0);
	while (fin)
	{
		// get character from infile
		fin.get(symbol);
		// look for character match in tuple
		auto it = std::find_if(codetable.begin(), codetable.end(), [&symbol](const std::tuple<unsigned char, int, std::string>& e)
			{
				return std::get<0>(e) == symbol;
			});

		if (it != codetable.end())
		{
			outbyte += std::get<2>(*it);
			if (outbyte.length() >= 8)
			{
				std::bitset<8> b1(outbyte.substr(0, 8));
				outbyte.erase(0, 8);
				fout.write((const char*)&b1, 1);
			}
		}
	}
};

bool HuffmanCode::HuffmanCode::ExpandFile(std::ifstream& fin, std::ofstream& fout)
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
		symbolmap.insert(make_pair(tempsymbol, tempcount));
	}
	return true;
};

// Process the compressed section of the file
void HuffmanCode::ReadCompressedFile(std::ifstream& fin, std::ofstream& fout)
{
	// Reposition start point past the header information
	size_t startpos = (sizeof(filetag) + sizeof(alphabetcount) + sizeof(totalcharacters) + alphabetcount * (sizeof(char) + sizeof(int)));
	fin.seekg(startpos);
	char inchar;
	unsigned char mask = 0x80;
	unsigned char code = 0;
	unsigned long long int totalbytes = GetTotalCodedBits() / 8;  // number of complete bytes in the encoded file
	unsigned char extrabits = GetTotalCodedBits() % 8;       // odd bits at the end og the file
	TreeNode* node = treeheap.top();
	for (unsigned int i = 0; i < totalbytes; i++)
	{
		fin.get(inchar);
		for (unsigned char j = 0; j < 8; j++)
		{
			code = (inchar & mask) ? 1 : 0;
			// look up tree
			node = (code) ? node->rightpointer : node->leftpointer;
			if (node->isleaf)
			{
				fout.write(&node->symbol, 1);
				node = treeheap.top();
			}
			inchar = inchar << 1;
		}
	}
	if (extrabits)  // any spare bits that are not a whole byte
	{
		fin.get(inchar);
		for (unsigned char j = 0; j < extrabits; j++)
		{
			code = (inchar & mask) ? 1 : 0;
			// look up tree
			node = (code) ? node->rightpointer : node->leftpointer;
			if (node->isleaf)
			{
				fout.write(&node->symbol, 1);
				node = treeheap.top();
			}
			inchar = inchar << 1;
		}
	}
}

// Clear character map
void HuffmanCode::ClearSymbolMap()
{
	symbolmap = {};
};

// Clear the tree
void HuffmanCode::ClearHuffmanTree()
{
	treeheap = {};
};

// Clear all tuples
void HuffmanCode::ClearCodeTable()
{
	codetable = {};
};

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
		codetable.push_back(make_tuple(node->symbol, node->weight, str));
	}
	MakePrefixCodes(node->leftpointer, str + "0");
	MakePrefixCodes(node->rightpointer, str + "1");
}

// Add a character to the symbolmap.
// Add entry for new symbols or update count for existing symbols
void HuffmanCode::MapSymbol(char character)
{
	// Traverse the map looking for existing entry
	auto it = symbolmap.find(character);
	if (it != symbolmap.end())  // existing symbol entry, update count
	{
		it->second++;
	}
	else
	{
		// new symbol found, add entry
		symbolmap.insert(make_pair(character, 1));
		alphabetcount++;      // keep count of different symbols
	}
	totalcharacters++;      // keep track of total characters read
	return;
}

// Sort tuple by value of second element
bool sortbysec(const tuple<char, int, string>& a, const tuple<char, int, string>& b)
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
		cout << std::get<0>(it) << "\t: " << std::get<1>(it) << "\t: " << std::get<2>(it) << endl;
	}
}

// Give a copy of CharacterMap when requested, if it has been built
bool HuffmanCode::GetSymbolMap(map<char, int>& tempmap)
{
	bool result = false;

	if (symbolmap.size())
	{
		tempmap = symbolmap;
		result = true;
	}
	return result;
};

// Return number of symbols in the CharacterMap
unsigned int HuffmanCode::GetGetAlphabetCount()
{
	return alphabetcount;
};

// Return total number of characters in the file
uintmax_t HuffmanCode::GetTotalCharacters()
{
	return totalcharacters;
};



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
};
