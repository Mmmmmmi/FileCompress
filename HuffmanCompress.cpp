#include <queue>
#include <assert.h>
#include "HuffmanCompress.h"
#include "HuffmanTree.hpp"
using namespace std;

bool CharInfo::operator>(const CharInfo& c) const
{
	return _charCount > c._charCount;
}

CharInfo CharInfo::operator+(const CharInfo& c) const
{
	CharInfo temp;
	temp._charCount = _charCount + c._charCount;
	return temp;
}

HuffmanCompress::HuffmanCompress()
{
	_charInfo.resize(256);
	for (int i = 0; i < 256; i++) {
		//vextor中的每个下标中的字符都对应自己的ASCII
		_charInfo[i]._char = i;
	}
}

void HuffmanCompress::WriteHead(FILE* fOut, const std::string& filepath)
{
	assert(fOut != nullptr);
	string headInfo = "";		//头信息

	//写入后缀
	string  fileSuffix = filepath.substr(filepath.rfind('.'));
	headInfo += fileSuffix;
	headInfo += '\n';

	size_t charTypeCount = 0;  //字符类型数，也就是行数
	string charCount = "";		//每个字符出现的次数和字符
	
	for (size_t i = 0; i < _charInfo.size(); i++) {
		if (_charInfo[i]._charCount != 0) {
			charTypeCount++;
			charCount += _charInfo[i]._char;
			charCount += ',';
			char temp[32] = { 0 };
			_itoa(_charInfo[i]._charCount, temp, 10);
			charCount += temp;
			charCount += '\n';
		}
	}
	
	//写入字符类型数
	char charTypeNum[32] = { 0 };
	_itoa(charTypeCount, charTypeNum, 10);
	headInfo += charTypeNum;
	headInfo += '\n';

	//写入字符信息
	headInfo += charCount;
	fwrite(headInfo.c_str(), 1, headInfo.size(), fOut);
}

std::string HuffmanCompress::Compress(const string& filepath)
{
	FILE* fIn = fopen(filepath.c_str(), "rb");
	if (fIn == nullptr) {
		cout << "error for fin！" << endl;
		exit(0);
	}
	while (1) {
		//1. 遍历文件，获取每个字符出现的次数
		unsigned char *tempbuf = new unsigned char [HUFFMANSIZE];
		//从文件中读取待压缩信息
        size_t rdSize = fread(tempbuf, 1, HUFFMANSIZE, fIn);

		if (rdSize == 0) {
			//如果读到的信息为0 说明读完了
			break;
		}
		//遍历  并且保存数据
		for (size_t i = 0; i < rdSize; i++) {
			//字符读应的计数器加一
			//tempbuf[i] 是取到的字符   直接用ASCII 就是对于的下标
			_charInfo[tempbuf[i]]._charCount++;
		}
	}

	string headCharInfo;
	//创建huffman树 
	//在_charInfo中共保存了256个字符的信息
	//创建huffman节点应该是给出现过的创建，因此应该进行一次判断
	//因为每次都需要取两个最小的 因此 直接将它们放在优先级队列中
	//直接创建成小树  取就可以
	HuffmanTree<CharInfo> huffmantree;
	huffmantree.CreateHuffmanTree(_charInfo);
	  
	//得到字符的编码
	huffmantree.GetHuffmanCode(_charInfo);


	//打开压缩文件
	string filename = filepath.substr(0, filepath.rfind('.')) + ".hip";

	FILE* fOut = fopen(filename.c_str(), "wb");
	if (fIn == nullptr) {
		cout << "error for fout！" << endl;
		exit(0);
	}
	
	//写入头信息
	WriteHead(fOut, filepath);

	
	//从头再读取文件，转换成压缩文件
	fseek(fIn, 0, SEEK_SET);

	unsigned char code = 0;		//记录写入的编码
	size_t leftCount = 0;		//记录左移次数

	//压缩
	while (1) {
		unsigned char* inbuffer = new unsigned char[HUFFMANSIZE];
		unsigned char* outbuffer = new unsigned char[HUFFMANSIZE];
		size_t outbufferpos = 0;
		//char outBuf[1024] = { 0 };
		size_t rdSize = fread(inbuffer, 1, HUFFMANSIZE, fIn);
		if (rdSize == 0) {
			break;
		}
		for (size_t i = 0; i < rdSize; ++i) {
			//遇见字符  修改后写入文件
			size_t j = 0;
			while (j < _charInfo[inbuffer[i]]._huffmanCode.size()) {
				//将对应字符的编码全部写入
				//当code中没有写满时，进行写入
				leftCount++;
				code <<= 1;
				//00000000
				if (_charInfo[inbuffer[i]]._huffmanCode[j] == '1') {
					code |= 1;
				}
				j++;
				if (leftCount == 8) {
					/*fputc(code, fOut);*/
					outbuffer[outbufferpos++] = code;
					if (outbufferpos == HUFFMANSIZE) {
						fwrite(outbuffer, outbufferpos, 1, fOut);
						outbufferpos = 0;
					}
					leftCount = 0;
					code = 0;
				}
			}
		}
		fwrite(outbuffer, outbufferpos, 1, fOut);
	}

	if (leftCount > 0 && leftCount <= 8) {
		code <<= (8 - leftCount);
		fputc(code, fOut);
	}

	//关闭文件流 释放树
	fclose(fIn);
	fclose(fOut);
	return filename;
}

void HuffmanCompress::GetLine(FILE* fIn, std::string& inbuffer)
{
	assert(fIn != nullptr);
	while (!feof(fIn)) {
		unsigned char c = fgetc(fIn);
		if (c == '\n' || feof(fIn)) {
			return;
		}
		inbuffer += c;
	}
}

std::string HuffmanCompress::UnCompress(const std::string & filepath)
{
	//解压缩，先读取文件前面的信息
	if (filepath.substr(filepath.rfind('.')) != ".hip") {
		cout << "error for file type" << endl;
		exit(0);
	}
	FILE* fIn = fopen(filepath.c_str(), "rb");
	if (fIn == nullptr) {
		cout << "error for fIn" << endl;
		exit(0);
	}

	//读取后缀信息
	string fileSuffix = "";
	GetLine(fIn, fileSuffix);

	//读取字符类型数
	string charTypeNum_s = "";
	GetLine(fIn, charTypeNum_s);
	size_t charTypeNum = atoi(charTypeNum_s.c_str());

	//读取字符信息,并创建Huffman树
	for (size_t i = 0; i < charTypeNum; i++) {
		string getCharInfo = "";
		GetLine(fIn, getCharInfo);
		if (getCharInfo.empty()) {
			getCharInfo += '\n';
			GetLine(fIn, getCharInfo);
		}
		_charInfo[(unsigned char)getCharInfo[0]]._charCount = atoi(getCharInfo.c_str() + 2);
	}

	//创建Huffman树
	HuffmanTree<CharInfo> huffmantree;
	huffmantree.CreateHuffmanTree(_charInfo);

	//解压缩
	//读取文件名和后缀
	string filename = filepath.substr(0, filepath.rfind('.')) + "UnCompress";
	filename += fileSuffix;

	FILE* fOut = fopen(filename.c_str(), "wb");
	if (fOut == nullptr) {
		cout << "error for fOut" << endl;
		exit(0);
	}

	HuffmanNode<CharInfo>* cur = huffmantree.GetRoot();

	//表示字符数
	size_t charNum = cur->_weight._charCount;

	while (1) {
		unsigned char inbuffer[1024] = { 0 };
		size_t reSize = fread(inbuffer, 1, 1024, fIn);
		if (reSize == 0) {
			break;
		}
		for (size_t i = 0; i < reSize; i++) {
			size_t leftCount = 7;
			for (size_t j = 0; j < 8; j++) {
				if (inbuffer[i] & (1 << leftCount)) {
					//1000 0000
					cur = cur->_rightNode;
				}
				else {
					cur = cur->_leftNode;
				}

				if (cur->_leftNode == nullptr && cur->_rightNode == nullptr) {
					fputc(cur->_weight._char, fOut);
					//fputc(cur->_weight._char, stdout);
					cur = huffmantree.GetRoot();
					charNum--;
					if (charNum == 0) {
						break;
					}

				}
				leftCount--;
			}
		}
	}
	//关闭文件流 释放树
	fclose(fIn);
	fclose(fOut);
	return filename;
}

HuffmanCompress::~HuffmanCompress()
{
	return;
}