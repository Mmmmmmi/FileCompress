#pragma once
#include <fstream>
#include <string>
#include <assert.h>
#include "HashTable.hpp"

static const USH MIN_LOOKAHEAD = MAX_MATCH + 1;				//����ʣ��δƥ���ַ�
static const USH MAX_DISTANT = WSIZE - MIN_LOOKAHEAD;		//����ƥ�䳤��


/*

		����һ��hash�� ��СΪ64K
		hash���Ϊ������  һ����Ϊ ���һ�����  һ����Ϊǰ�򻺳���
		

		2019.3.4 17��46  ѹ��ģ�鲿�����
		TODO	��ѹ��֮ǰ��ѹ����ص���ϢҲ����
		2019.3.5 17��35  ��̬�����ļ��������
		TODO	������������
		2019.3.8 00:06   ��Ŀ���
*/



class Lz77Compress
{
public:
	Lz77Compress()		//���캯��
		:_hashTable(WSIZE)
		, _buffer(new UCH[2 * WSIZE])
		, _lookAhead(0)
		, _start(0)
	{
		//memset(_buffer, 0, 2 * WSIZE);		//���������ڳ�ʼ��
	}

	void WriteFlag(FILE* fOutFlag, UCH& flag, UCH& fileLeftCOunt, int type)
	{
		//fOutFlag  �ļ�ָ��
		//flag		���ֵ
		//fileLeftCOunt	������ƵĴ���
		//type		��ǵ����� ��Դ�ַ� �����Ǿ��볤�ȶ�  Դ�ַ�Ϊ0  ���볤�ȶ�Ϊ1
		assert(fOutFlag != nullptr);

		flag <<= 1;
		fileLeftCOunt++;

		if (type == 1) {
			//��ʾ�Ǿ��볤�ȶ�
			flag |= 1;
		}

		//���д��8��
		if (fileLeftCOunt == 8) {
			fputc(flag, fOutFlag);
			flag = 0;
			fileLeftCOunt = 0;
		}
	}

	std::string Compress(const std::string& fileName)
	{
		//�����ļ�������
		FILE* fIn = fopen(fileName.c_str(), "rb");
		if (!fIn) {
			std::cout << "fIn error !" << std::endl;
			exit(0);
		}

		//���������һ���жϣ���Ϊ�ڱ��㷨��  ���õ��滻�����Ҫռ��3���ֽ�
		//��ˣ�����ļ��Ĵ�С С�ڵ���3 ���ֽڵĻ� �Ͳ���ѹ��

		fseek(fIn, 0, SEEK_END);
		ULL fileSize = ftell(fIn);		//�����ǰ�ļ�ָ������ļ���ʼ���ֽ���
		if (fileSize <= 3) {
			fclose(fIn);
			return 0;
		}
		
		//���½�ָ��ŵ��ļ���
		fseek(fIn, 0, SEEK_SET);
		//��Դ�ļ��ж�ȡ�ֽ�
		_lookAhead = fread(_buffer, 1, 2 * WSIZE, fIn);

		//�ߵ����� ��˵��Ҫ����ѹ���ˣ���˽���׺������

		std::string fileSuffix = fileName.substr(fileName.rfind('.')) + '\n';
				
		USH hashAddr = 0;		//ͨ��hash���� ����õ���hash��ַ
		USH matchHead = 0;	
		//���ַ������󣬷��ص�����  ���֮ǰ����� ��ô ������һ�����±� ���û�� ����0
		//Ҳ���Ƿ���ƥ����������λ��
		//��ʼ��hash���д洢 ����׼����ѯ
		//��Ϊhash�ǽ��ϴε�hashAddr�ó��� ����Ȼ���������� �Ȱ�ǰ�����ַ���hash��ַ�������
		//  ��ȡ�ļ���Ϣ
		for (size_t i = 0; i < MIN_MATCH - 1; ++i) {
			_hashTable.GetHushAddr(hashAddr, _buffer[i]);
		}

		//�����ļ�д����׼����ѹ���ļ���д��
		//ѹ���ļ�
		std::string compressFileName = fileName.substr(0, fileName.rfind('.')) + ".lzp";
		FILE* fOut = fopen(compressFileName.c_str(), "wb");
		if (!fOut) {
			std::cout << "fOut error !" << std::endl;
			exit(0);
		}

		//�����Ϣ
		//�����Ϣ������Ϊ�����ĳ������λ��Դ�ַ� ���� <���룬����>�ԡ�
		FILE* fOutFlag = fopen("flag.temp", "wb");
		if (!fOutFlag) {
			std::cout << "fOutFlag error !" << std::endl;
			exit(0);
		}


		//�Ƚ��ļ���׺д��
		fwrite(fileSuffix.c_str(), 1, fileSuffix.size(), fOut);


		UCH flag = 0;	//��������flag��ÿ��8λ д��flag�ļ���
		UCH flagLeftCount = 0;	//��¼���ƵĴ���

		// �������ڵ�ʵ�֣� ��Ϊ ��2���ֽ� USH ����ʾ����  ��һ���ֽ�UCH ����ʾƥ�䳤�ȣ���� �ַ���ƥ��ľ���Ϊ��
		// |<---1111111111111111---------------- c -------------11111111------------>|
		//			65535				32K  2 ^15		    				255
		//��ƥ���ʱ�����һֱ��ǰƥ��Ļ����Ϳ��ܻᵼ��ƥ��ʱ���������˽���������32K��
		//ͬʱ��ƥ����ַ����� ����һ���ֽڱ�ʾ�ģ�����������ƥ��255λ
		//��һ��������  δѹ��������Ϊ_lookAhead  
		//���ǿ������ã�����ƥ���ַ�����Ϊ255�� ��ô Ϊ�˱�֤���Դﵽ���ƥ�䳤�ȣ���Ӧ��ʼ�ձ��� ʣ����ַ����� >= 255��
		//�������ƥ���ַ����� ������������Ϊ MAX_MATCH
		//ͬʱ��Ϊ�˱�֤ ����ƥ����ɺ󣬻���������һ���ַ������ ʣ�µ��ַ� ����Ӧ���� 255 + 1 Ҳ���� MAX_MATCH + 1
		//�������ʣ���ַ� ����������Ϊ MIN_LOOKAHEAD

		while (_lookAhead) {

			//�����д�ѹ��������ʱ
			//�ж��Ƿ���в���  ֻ�дﵽһ������ʱ�Ž���ƥ�� Ҳ���ǻ�������
			
			_hashTable.HushInsert(hashAddr, _buffer[_start + 2], _start, matchHead);
			USH maxDistant = 0;
			UCH maxMatch = 0;
			if (matchHead != -1 && _lookAhead > MIN_LOOKAHEAD) {
				
				maxMatch = maxLongMatch(matchHead, maxDistant);
				//Ҳ���� ���ص�ƥ����ͷ ��Ϊ��
				//GetMaxMatch(matchHead, maxDistant, maxMatch);
			}

			if (maxMatch >= MIN_MATCH) {

				fwrite(&maxDistant, 2, 1, fOut);
				fputc(maxMatch, fOut);


				WriteFlag(fOutFlag, flag, flagLeftCount, 1);
				
				//�ߵ�����Ļ����ͰѼ�ֵ��д��ȥ��
				//mnoabczxyuvwabc123456abczxydefgh
				//����Ӧ�����ľ��� ��lookAhead ��С  start ����ƶ� ͬʱ �����Ѿ�ƥ������ַ����� ����������˼ �ǲ����ļ���д ���� ���ð����ǵ���Ϣ����hash��
			
				UCH CountMatch = maxMatch - 1;
				while (CountMatch) {
					++_start;
					_hashTable.HushInsert(hashAddr, _buffer[_start + 2], _start, matchHead);
					CountMatch--;
				}
				_start++;		//����ƥ����ɵ���һ����
				_lookAhead -= maxMatch;

			}
			else {
				fputc(_buffer[_start], fOut);
				WriteFlag(fOutFlag, flag, flagLeftCount, 0);
				--_lookAhead;
				++_start;
			}

			if (_lookAhead < MIN_LOOKAHEAD) {
				//��ȡ����
				if (_start >= WSIZE + MAX_DISTANT) {
				
					//ʣ�µ�Ҫ����MIN_LOOKHEAD
					//С�� �򻬶�����
					//Ų����
					
					memmove(_buffer, _buffer + WSIZE, WSIZE * sizeof(UCH));
					
					//Ų��ȥ֮�󣬺�����ܶ��������ݣ����������ԭ�������� ���뵽���������ʱ�� ���ͻ���϶�����ַ�
					//���Ӧ�ô���
					
					memset(_buffer + WSIZE, 0, WSIZE * sizeof(UCH));
					
					_start -= WSIZE;
					
					//����hash��
					_hashTable.updateHashTable();
					
					//��������
					if (!feof(fIn)) {
						_lookAhead += fread(_buffer + WSIZE, 1, WSIZE, fIn);
					}
				}
			}

		}

		//�ߵ���������������һ��д��ı�ǲ���8λ������Ҫ����
		if (flagLeftCount > 0 && flagLeftCount < 8) {
			flag <<= (8 - flagLeftCount);
			fputc(flag, fOutFlag);
		}

		//�ߵ����� �ļ���ѹ�������
		//������ļ�д��ѹ���ļ��С�
		//�ȹرձ���ļ���д��ָ�룬���������ڻ����� ��û��д��ȥ
		
		fclose(fOutFlag);
		
		FILE* fInFlag = fopen("flag.temp", "rb");
		if (!fInFlag) {
			std::cout << "error for fInFlag" << std::endl;
			exit(0);
		}

		//Ҫ���ļ�������棬�����ǡ��ļ���С �ͱ�Ǵ�С
		//��Ǻ�Huffman����һ��,��λ����ǣ�ÿд�� �ͱ����ȥ
		//���  �����Ƕ������
		//�ļ���С  ��ǰ���Ѿ������
		
		ULL flagSize = 0;
		UCH* flagBuffer = new UCH[1024];
		while (1) {
			size_t rdSize = fread(flagBuffer, 1, 1024, fInFlag);
			if (rdSize == 0) {
				break;
			}

			flagSize += rdSize;
			fwrite(flagBuffer, 1, rdSize, fOut);
		}

		//���ļ���С �� ��Ǵ�С д��
		fwrite(&fileSize, sizeof(fileSize), 1, fOut);
		fwrite(&flagSize, sizeof(fileSize), 1, fOut);

		
		//�ر���ص��ļ�ָ��
		fclose(fIn);
		fclose(fOut);
		fclose(fInFlag);

		//ɾ����������ʱ�ļ�		
		remove("flag.temp");
		return compressFileName;
	}

	void GetMaxMatch(USH & machtListHead, USH & maxDistant, UCH & maxMatch)
	{
		//machtListHead  ƥ������ͷ
		//maxDistant	���볤�ȶԵ� ����
		//maxMatch		���볤�ȶԵ� ����
		//Ŀǰ��
		//���ƥ�䣬û���ߵ�ͷ �ͼ���ƥ��	ƥ��϶����ڴ��ڷ�Χ�ڵ�  ��� ֱ���ڴ����в��Ҿͺ�
		USH nextMatch = machtListHead;		//��ʾ�Ƿ񵽴�ƥ��Ľ�β
		USH limit = _start > MAX_DISTANT ? _start - MAX_DISTANT : 0;
		USH maxMatchCount = 255;				//��ʾ�����Բ�������Ĵ���
		UCH curMatch = 0;		//��¼ ƥ���˼����ַ�
		while (maxMatchCount-- && nextMatch > limit) {		//��û���ߵ�list�����һ����ʱ�� ��һֱ��
			USH curMatchPos = nextMatch;	//��ʾ ƥ�俪ʼ���±�     ������ͬhashaddr ˵�� ǰ�����ַ���ͬ�� ��˿��Զ�ƥ������
			USH curDistant = _start - curMatchPos;	//��ʾ����ƥ���� ����
			USH curStart = _start;				//������ѹ�����ַ� �����
			curMatch = 0;
			while (curMatch <= 255 && _buffer[curMatchPos++] == _buffer[curStart++]) {		//���ƥ���ַ���ͬ ��ô����ƥ�䳤�ȼ�һ
				curMatch++;
			}
			if (curMatch > maxMatch) {
				maxMatch = curMatch;
				maxDistant = curDistant;
				if (curMatch == 255) {
					//�ﵽ���ƥ�䳤�ȣ� ������ƥ�� Ҳ������ƥ����
					return;
				}
			}

			//���е����� ˵�� ����ƥ������ˣ���ʼ��һ��ƥ��
			nextMatch = _hashTable.GetListNext(nextMatch);
		}


	}

	UCH maxLongMatch(USH matchHead, USH & curMatchDist)
	{
		UCH curMatchLen = 0;
		UCH maxlen = 0;
		USH pos = 0;
		UCH Matchchainlen = 255;
		USH limit = _start > MAX_DISTANT ? _start - MAX_DISTANT : 0;
		do
		{
			UCH* pStart = _buffer + _start;
			//�ҵ�ƥ�䴮��λ��
			UCH* pCurStart = _buffer + matchHead;
			UCH* pEnd = pStart + MAX_MATCH;
			curMatchLen = 0;

			//�ҵ�������ƥ�䳤��
			while (pStart < pEnd && *pStart == *pCurStart)
			{
				pStart++;
				pCurStart++;
				curMatchLen++;
			}

			if (curMatchLen > maxlen)
			{
				maxlen = curMatchLen;
				pos = matchHead;
			}

		} while ((matchHead = _hashTable.GetListNext(matchHead)) > limit && Matchchainlen--);

		curMatchDist = _start - pos;
		return maxlen;
	}

	void GetLine(FILE * fIn, std::string & inbuffer)
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

	std::string UnCompress(const std::string & fileName)
	{
		//���ж��ǲ��Ƕ�Ӧ��ѹ���ļ�
		std::string fileSuffix = fileName.substr(fileName.rfind('.'));
		if (fileSuffix != ".lzp") {
			std::cout << "error for file type" << std::endl;
			exit(0);
		}

		std::string unCompressFileName = "unCompress" + fileName.substr(0, fileName.rfind('.'));

		//��ѹ���ļ�
		FILE* fIn = fopen(fileName.c_str(), "rb");
		if (fIn == nullptr) {
			std::cout << "error for fIn" << std::endl;
			exit(0);
		}
		fileSuffix.clear();
		GetLine(fIn, fileSuffix);
		if (fileSuffix.empty()) {
			std::cout << "error for fIn fileSuffinx" << std::endl;
		}
		unCompressFileName += fileSuffix;

		//д��ѹ���ļ�
		FILE* fOut = fopen(unCompressFileName.c_str(), "wb");
		if (fOut == nullptr) {
			std::cout << "error for fOut" << std::endl;
			exit(0);
		}

		//����Ǵ�С
		FILE* fInFlagSize = fopen(fileName.c_str(), "rb");
		if (fOut == nullptr) {
			std::cout << "error for fOut" << std::endl;
			exit(0);
		}
		ULL flagSize = 0;
		int offset = 0 - sizeof(flagSize);
		fseek(fInFlagSize, offset, SEEK_END);
		fread(&flagSize, sizeof(flagSize), 1, fInFlagSize);

		//���ļ���С
		FILE* fInFileSize = fopen(fileName.c_str(), "rb");
		if (fOut == nullptr) {
			std::cout << "error for fOut" << std::endl;
			exit(0);
		}
		ULL fileSize = 0;
		fseek(fInFileSize, 0 - sizeof(flagSize) - sizeof(fileSize), SEEK_END);
		fread(&fileSize, sizeof(fileSize), 1, fInFileSize);

		//�����
		FILE* fInFlag = fopen(fileName.c_str(), "rb");
		if (fOut == nullptr) {
			std::cout << "error for fOut" << std::endl;
			exit(0);
		}
		fseek(fInFlag, 0 - sizeof(flagSize) - sizeof(fileSize) - flagSize, SEEK_END);


		//����ƥ�䣬ǰ���
		FILE* fMatchPrev = fopen(unCompressFileName.c_str(), "rb");
		if (fOut == nullptr) {
			std::cout << "error for fOut" << std::endl;
			exit(0);
		}


		UCH flag = 0;			//��ȡ�ı��
		char flagLeftCount = -1;		//���ƵĴ���
		//11110000
		while (fileSize) {

			//��ȡ��� ͨ������ж���Դ�ַ� ���Ǽ�ֵ�� 
			if (flagLeftCount < 0) {		//
				flag = fgetc(fInFlag);
				flagLeftCount = 7;
			}

			if (((flag >> flagLeftCount) & 1) == 1) {
				//˵���Ǽ�ֵ��
				USH matchDistant = 0;	//����
				UCH matchlong = 0;	//����


				fread(&matchDistant, sizeof(matchDistant), 1, fIn);
				matchlong = fgetc(fIn);
				fflush(fOut);		//����ƥ��ᾭ����ǰ�� ���� abcabcabc

				fseek(fMatchPrev, 0 - matchDistant, SEEK_END);
				//����matchlong �������ȸ�ֵ
				fileSize -= matchlong;
				//mnoabczxyuvwabc123456abczxydefgh
				while (matchlong != 0) {
					UCH ch = fgetc(fMatchPrev);
					fputc(ch, fOut);
					fflush(fOut);
					matchlong--;
				}
			}
			else {
				//˵����Դ�ַ�
				UCH ch = fgetc(fIn);
				fputc(ch, fOut);
				fileSize--;
			}
			flagLeftCount--;
		}
		return unCompressFileName;
	}

	~Lz77Compress()		//��������
	{
		delete[] _buffer;
	}

private:
	HashTable _hashTable;			//�����洢��ѯ�ַ��±�
	UCH* _buffer;					//��������
	size_t _lookAhead;					//��ʾʣ�����Ԫ�ش�ѹ��
	size_t _start;						//��ʾ��ǰѹ�������ڵ��ĸ��±�
};