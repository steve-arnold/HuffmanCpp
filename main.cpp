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

	if (!filesystem::exists(inputfile))
	{
		cout << "Error !!! File does not exist.\n";
		cin >> y;
		exit(1);
	}
	// Create output file by replacing the input file extension with "cmp"
	outputfile = inputfile.substr(0, inputfile.find_last_of('.')) + ".cmp";

	try
	{
		const auto fsize = filesystem::file_size(inputfile);
//		std::cout << fsize << '\n';
	}
	catch (const filesystem::filesystem_error& err)
	{
		std::cerr << "filesystem error! " << err.what() << '\n';
		if (!err.path1().empty())
			std::cerr << "path1: " << err.path1().string() << '\n';
		if (!err.path2().empty())
			std::cerr << "path2: " << err.path2().string() << '\n';
		cin >> y;
		exit(1);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "general exception: " << ex.what() << '\n';
		cin >> y;
		exit(1);
	}

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