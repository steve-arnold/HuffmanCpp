#include "huffman.h"

int main()
{
	int y;
	HuffmanCode tree;
	ifstream fin;
	ofstream fout;
	string inputfile, outputfile;
	cout << "\n\t====================================";
	cout << "\n\t\t  Text File Compressor\n";
	cout << "\t====================================";
	cout << "\n\nEnter Name of File to Compress : ";
	cin >> inputfile;
	// Create output file by replacing the input file extension with "cmp"
	outputfile = inputfile.substr(0, inputfile.find_last_of('.')) + ".cmp";
	fin.open(inputfile);
	if (!fin.is_open())
	{
		cout << "Error !!! Cannot open Source file.\n";
		cin >> y;
		exit(1);
	}
	fout.open(outputfile, ios::binary);
	if (!fout.is_open())
	{
		fin.close();
		cout << "Error !!! Cannot open Destination file.\n";
		cin >> y;
		exit(1);
	}

	tree.CompressFile(fin, fout);

	tree.PrintCodeTable();
	cout << "----------------------------------------------------------------------------------- " << endl;
	cout << "Active characters: " << tree.GetGetAlphabetCount() << "\t\tTotal characters: " << tree.GetTotalCharacters() << endl;
	cout << "----------------------------------------------------------------------------------- " << endl;
	fin.close();
	fout.close();

	// Reset the file date
	tree.ClearCodeTable();
	tree.ClearHuffmanTree();
	// Open compressed file
	fin.open(outputfile, ios::binary);
	if (!fin.is_open())
	{
		cout << "Error !!! Cannot open Source file.\n";
		cin >> y;
		exit(1);
	}
	// Open destination file
	fout.open("uncompressed.txt", ios::binary);
	if (!fout.is_open())
	{
		fin.close();
		cout << "Error !!! Cannot create Output file.\n";
		cin >> y;
		exit(1);
	}
	// Decode it
	if (!tree.ExpandFile(fin, fout))
	{
		fin.close();
		fout.close();
		cout << "Error !!! Input file, incorrect file format file.\n";
		cin >> y;
		exit(1);
	};

	tree.PrintCodeTable();
	cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ " << endl;
	cout << "Active characters: " << tree.GetGetAlphabetCount() << "\t\tTotal characters: " << tree.GetTotalCharacters() << endl;
	cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ " << endl;

	fin.close();
	fout.close();
	return 0;
}