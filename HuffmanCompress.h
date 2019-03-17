#ifndef __HUFFMANCOMPRESS_H__
#define __HUFFMANCOMPRESS_H__





#include <iostream>
#include <string>
#include <vector>

struct CharInfo
{
	bool operator>(const CharInfo& c) const;
	CharInfo operator+(const CharInfo& c) const;
	unsigned char _char;						 //字符
	long long _charCount;						     //出现的次数
	std::string _huffmanCode;					 //对应的huffman编码
};

class HuffmanCompress
{
public:
	HuffmanCompress();
	void Compress(const std::string& filepath);
	void WriteHead(FILE *fOut, const std::string& filename);
	void GetLine(FILE* fIn, std::string& inbuffer);
	void UnCompress(const std::string& filepath);
	~HuffmanCompress();
private:
	std::vector<CharInfo> _charInfo;
};

#endif //__HUFFMANCOMPRESS_H__