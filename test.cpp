#include "HuffmanCompress.h"
#include "Lz77Compress.hpp"

#include <iostream>
using namespace std;
int main()
{

#if 1
	HuffmanCompress fc;
	fc.Compress("3.png");
	fc.UnCompress("3.hip");
#else
	Lz77Compress lc;
	lc.Compress("3.png");
	lc.UnCompress("3.lzp");
#endif
	return 0;
}

