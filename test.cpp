#include "HuffmanCompress.h"
#include "Lz77Compress.hpp"

#include <iostream>
using namespace std;

void Compress(string& filePath)
{
	Lz77Compress lz;
	string lzFile = lz.Compress(filePath);
	HuffmanCompress hf;
	string hfFile = hf.Compress(lzFile);

	cout << "压缩完成！" << endl;
}

void UnCompress(string& filePath)
{
	HuffmanCompress hf;
	string hfFile = hf.UnCompress(filePath);
	Lz77Compress lz;
	string lzFile = lz.UnCompress(hfFile);
	cout << "解压缩完成！" << endl;

}


int main(int argc, char* argv[])
{

#if 1
	HuffmanCompress fc;
	fc.Compress("1234.png");
	fc.UnCompress("1234.hip");
#else
	Lz77Compress lc;
	lc.Compress("4.png");
	lc.UnCompress("4.lzp");
#endif


	//#if 0
	//	if (argc < 2) {
	//		cout << "error input" << endl;
	//		exit(0);
	//	}
	//	string filePath = argv[1];
	//#else 
	//
	//	string filePath = "3.png";
	//#endif
	//	string fileSuffix = filePath.substr(filePath.rfind('.'));
	//	if (fileSuffix == ".hip") {
	//		UnCompress(filePath);
	//	}
	//	else {
	//		Compress(filePath);
	//	}

	return 0;
}

