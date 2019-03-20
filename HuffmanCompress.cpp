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
		//vextor�е�ÿ���±��е��ַ�����Ӧ�Լ���ASCII
		_charInfo[i]._char = i;
	}
}

void HuffmanCompress::WriteHead(FILE* fOut, const std::string& filepath)
{
	assert(fOut != nullptr);
	string headInfo = "";		//ͷ��Ϣ

	//д���׺
	string  fileSuffix = filepath.substr(filepath.rfind('.'));
	headInfo += fileSuffix;
	headInfo += '\n';

	size_t charTypeCount = 0;  //�ַ���������Ҳ��������
	string charCount = "";		//ÿ���ַ����ֵĴ������ַ�
	
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
	
	//д���ַ�������
	char charTypeNum[32] = { 0 };
	_itoa(charTypeCount, charTypeNum, 10);
	headInfo += charTypeNum;
	headInfo += '\n';

	//д���ַ���Ϣ
	headInfo += charCount;
	fwrite(headInfo.c_str(), 1, headInfo.size(), fOut);
}

std::string HuffmanCompress::Compress(const string& filepath)
{
	FILE* fIn = fopen(filepath.c_str(), "rb");
	if (fIn == nullptr) {
		cout << "error for fin��" << endl;
		exit(0);
	}
	while (1) {
		//1. �����ļ�����ȡÿ���ַ����ֵĴ���
		unsigned char *tempbuf = new unsigned char [HUFFMANSIZE];
		//���ļ��ж�ȡ��ѹ����Ϣ
        size_t rdSize = fread(tempbuf, 1, HUFFMANSIZE, fIn);

		if (rdSize == 0) {
			//�����������ϢΪ0 ˵��������
			break;
		}
		//����  ���ұ�������
		for (size_t i = 0; i < rdSize; i++) {
			//�ַ���Ӧ�ļ�������һ
			//tempbuf[i] ��ȡ�����ַ�   ֱ����ASCII ���Ƕ��ڵ��±�
			_charInfo[tempbuf[i]]._charCount++;
		}
	}

	string headCharInfo;
	//����huffman�� 
	//��_charInfo�й�������256���ַ�����Ϣ
	//����huffman�ڵ�Ӧ���Ǹ����ֹ��Ĵ��������Ӧ�ý���һ���ж�
	//��Ϊÿ�ζ���Ҫȡ������С�� ��� ֱ�ӽ����Ƿ������ȼ�������
	//ֱ�Ӵ�����С��  ȡ�Ϳ���
	HuffmanTree<CharInfo> huffmantree;
	huffmantree.CreateHuffmanTree(_charInfo);
	  
	//�õ��ַ��ı���
	huffmantree.GetHuffmanCode(_charInfo);


	//��ѹ���ļ�
	string filename = filepath.substr(0, filepath.rfind('.')) + ".hip";

	FILE* fOut = fopen(filename.c_str(), "wb");
	if (fIn == nullptr) {
		cout << "error for fout��" << endl;
		exit(0);
	}
	
	//д��ͷ��Ϣ
	WriteHead(fOut, filepath);

	
	//��ͷ�ٶ�ȡ�ļ���ת����ѹ���ļ�
	fseek(fIn, 0, SEEK_SET);

	unsigned char code = 0;		//��¼д��ı���
	size_t leftCount = 0;		//��¼���ƴ���

	//ѹ��
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
			//�����ַ�  �޸ĺ�д���ļ�
			size_t j = 0;
			while (j < _charInfo[inbuffer[i]]._huffmanCode.size()) {
				//����Ӧ�ַ��ı���ȫ��д��
				//��code��û��д��ʱ������д��
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

	//�ر��ļ��� �ͷ���
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
	//��ѹ�����ȶ�ȡ�ļ�ǰ�����Ϣ
	if (filepath.substr(filepath.rfind('.')) != ".hip") {
		cout << "error for file type" << endl;
		exit(0);
	}
	FILE* fIn = fopen(filepath.c_str(), "rb");
	if (fIn == nullptr) {
		cout << "error for fIn" << endl;
		exit(0);
	}

	//��ȡ��׺��Ϣ
	string fileSuffix = "";
	GetLine(fIn, fileSuffix);

	//��ȡ�ַ�������
	string charTypeNum_s = "";
	GetLine(fIn, charTypeNum_s);
	size_t charTypeNum = atoi(charTypeNum_s.c_str());

	//��ȡ�ַ���Ϣ,������Huffman��
	for (size_t i = 0; i < charTypeNum; i++) {
		string getCharInfo = "";
		GetLine(fIn, getCharInfo);
		if (getCharInfo.empty()) {
			getCharInfo += '\n';
			GetLine(fIn, getCharInfo);
		}
		_charInfo[(unsigned char)getCharInfo[0]]._charCount = atoi(getCharInfo.c_str() + 2);
	}

	//����Huffman��
	HuffmanTree<CharInfo> huffmantree;
	huffmantree.CreateHuffmanTree(_charInfo);

	//��ѹ��
	//��ȡ�ļ����ͺ�׺
	string filename = filepath.substr(0, filepath.rfind('.')) + "UnCompress";
	filename += fileSuffix;

	FILE* fOut = fopen(filename.c_str(), "wb");
	if (fOut == nullptr) {
		cout << "error for fOut" << endl;
		exit(0);
	}

	HuffmanNode<CharInfo>* cur = huffmantree.GetRoot();

	//��ʾ�ַ���
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
	//�ر��ļ��� �ͷ���
	fclose(fIn);
	fclose(fOut);
	return filename;
}

HuffmanCompress::~HuffmanCompress()
{
	return;
}