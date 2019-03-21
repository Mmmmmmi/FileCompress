#pragma once
#include <fstream>
#include <string>
#include <assert.h>
#include "HashTable.hpp"

static const USH MIN_LOOKAHEAD = MAX_MATCH + 1;				//最少剩余未匹配字符
static const USH MAX_DISTANT = WSIZE - MIN_LOOKAHEAD;		//最大可匹配长度


/*

		创建一个hash表 大小为64K
		hash表分为两部分  一部分为 查找缓冲区  一部分为前向缓冲区
		

		2019.3.4 17：46  压缩模块部分完成
		TODO	在压缩之前将压缩相关的信息也保存
		2019.3.5 17：35  静态窗口文件处理完成
		TODO	滑动窗口问题
		2019.3.8 00:06   项目完成
*/



class Lz77Compress
{
public:
	Lz77Compress()		//构造函数
		:_hashTable(WSIZE)
		, _buffer(new UCH[2 * WSIZE])
		, _lookAhead(0)
		, _start(0)
	{
		//memset(_buffer, 0, 2 * WSIZE);		//将滑动窗口初始化
	}

	void WriteFlag(FILE* fOutFlag, UCH& flag, UCH& fileLeftCOunt, int type)
	{
		//fOutFlag  文件指针
		//flag		标记值
		//fileLeftCOunt	标记左移的次数
		//type		标记的类型 是源字符 或者是距离长度对  源字符为0  距离长度对为1
		assert(fOutFlag != nullptr);

		flag <<= 1;
		fileLeftCOunt++;

		if (type == 1) {
			//表示是距离长度对
			flag |= 1;
		}

		//如果写够8次
		if (fileLeftCOunt == 8) {
			fputc(flag, fOutFlag);
			flag = 0;
			fileLeftCOunt = 0;
		}
	}

	std::string Compress(const std::string& fileName)
	{
		//创建文件读对象
		FILE* fIn = fopen(fileName.c_str(), "rb");
		if (!fIn) {
			std::cout << "fIn error !" << std::endl;
			exit(0);
		}

		//在这里进行一个判断，因为在本算法中  采用的替换标记需要占用3个字节
		//因此，如果文件的大小 小于等于3 个字节的话 就不用压缩

		fseek(fIn, 0, SEEK_END);
		ULL fileSize = ftell(fIn);		//求出当前文件指针距离文件开始的字节数
		if (fileSize <= 3) {
			fclose(fIn);
			return 0;
		}
		
		//重新将指针放到文件首
		fseek(fIn, 0, SEEK_SET);
		//从源文件中读取字节
		_lookAhead = fread(_buffer, 1, 2 * WSIZE, fIn);

		//走到这里 就说明要进行压缩了，因此将后缀保存了

		std::string fileSuffix = fileName.substr(fileName.rfind('.')) + '\n';
				
		USH hashAddr = 0;		//通过hash函数 计算得到的hash地址
		USH matchHead = 0;	
		//当字符组插入后，返回的数据  如果之前插入过 那么 返回上一个的下标 如果没有 返回0
		//也就是返回匹配链的链首位置
		//开始往hash表中存储 并且准备查询
		//因为hash是将上次的hashAddr拿出来 左移然后操作，因此 先把前两个字符的hash地址计算出来
		//  读取文件信息
		for (size_t i = 0; i < MIN_MATCH - 1; ++i) {
			_hashTable.GetHushAddr(hashAddr, _buffer[i]);
		}

		//创建文件写对象，准备向压缩文件中写入
		//压缩文件
		std::string compressFileName = fileName.substr(0, fileName.rfind('.')) + ".lzp";
		FILE* fOut = fopen(compressFileName.c_str(), "wb");
		if (!fOut) {
			std::cout << "fOut error !" << std::endl;
			exit(0);
		}

		//标记信息
		//标记信息的作用为：标记某个比特位是源字符 还是 <距离，长度>对。
		FILE* fOutFlag = fopen("flag.temp", "wb");
		if (!fOutFlag) {
			std::cout << "fOutFlag error !" << std::endl;
			exit(0);
		}


		//先将文件后缀写入
		fwrite(fileSuffix.c_str(), 1, fileSuffix.size(), fOut);


		UCH flag = 0;	//用来保存flag，每满8位 写入flag文件中
		UCH flagLeftCount = 0;	//记录左移的次数

		// 滑动窗口的实现： 因为 用2个字节 USH 来表示距离  用一个字节UCH 来表示匹配长度，因此 字符能匹配的距离为：
		// |<---1111111111111111---------------- c -------------11111111------------>|
		//			65535				32K  2 ^15		    				255
		//在匹配的时候，如果一直向前匹配的话，就可能会导致匹配时间过长，因此将其限制在32K内
		//同时，匹配的字符个数 是用一个字节表示的，因此做多可以匹配255位
		//在一个窗口中  未压缩的数据为_lookAhead  
		//我们可以设置，最大可匹配字符个数为255个 那么 为了保证可以达到最大匹配长度，就应该始终保持 剩余的字符个数 >= 255个
		//这个最大可匹配字符个数 我们在这设置为 MAX_MATCH
		//同时，为了保证 本次匹配完成后，还能跳到下一个字符，因此 剩下的字符 最少应该是 255 + 1 也就是 MAX_MATCH + 1
		//这个最少剩余字符 在这里设置为 MIN_LOOKAHEAD

		while (_lookAhead) {

			//当还有待压缩的数据时
			//判断是否进行插入  只有达到一定条件时才进行匹配 也就是滑动窗口
			
			_hashTable.HushInsert(hashAddr, _buffer[_start + 2], _start, matchHead);
			USH maxDistant = 0;
			UCH maxMatch = 0;
			if (matchHead != -1 && _lookAhead > MIN_LOOKAHEAD) {
				
				maxMatch = maxLongMatch(matchHead, maxDistant);
				//也就是 返回的匹配链头 不为空
				//GetMaxMatch(matchHead, maxDistant, maxMatch);
			}

			if (maxMatch >= MIN_MATCH) {

				fwrite(&maxDistant, 2, 1, fOut);
				fputc(maxMatch, fOut);


				WriteFlag(fOutFlag, flag, flagLeftCount, 1);
				
				//走到这里的话，就把键值对写进去了
				//mnoabczxyuvwabc123456abczxydefgh
				//现在应该做的就是 将lookAhead 减小  start 向后移动 同时 ，将已经匹配过的字符跳过 ，跳过的意思 是不往文件中写 但是 还得把它们的信息存入hash表
			
				UCH CountMatch = maxMatch - 1;
				while (CountMatch) {
					++_start;
					_hashTable.HushInsert(hashAddr, _buffer[_start + 2], _start, matchHead);
					CountMatch--;
				}
				_start++;		//跳到匹配完成的下一个上
				_lookAhead -= maxMatch;

			}
			else {
				fputc(_buffer[_start], fOut);
				WriteFlag(fOutFlag, flag, flagLeftCount, 0);
				--_lookAhead;
				++_start;
			}

			if (_lookAhead < MIN_LOOKAHEAD) {
				//读取数据
				if (_start >= WSIZE + MAX_DISTANT) {
				
					//剩下的要大于MIN_LOOKHEAD
					//小于 则滑动窗口
					//挪数据
					
					memmove(_buffer, _buffer + WSIZE, WSIZE * sizeof(UCH));
					
					//挪过去之后，后面可能读不到数据，如果不处理原来的数据 插入到最后两个的时候 ，就会带上额外的字符
					//因此应该处理
					
					memset(_buffer + WSIZE, 0, WSIZE * sizeof(UCH));
					
					_start -= WSIZE;
					
					//更新hash表
					_hashTable.updateHashTable();
					
					//补充数据
					if (!feof(fIn)) {
						_lookAhead += fread(_buffer + WSIZE, 1, WSIZE, fIn);
					}
				}
			}

		}

		//走到这里来，如果最后一次写入的标记不够8位，则需要处理
		if (flagLeftCount > 0 && flagLeftCount < 8) {
			flag <<= (8 - flagLeftCount);
			fputc(flag, fOutFlag);
		}

		//走到这里 文件就压缩完成了
		//将标记文件写入压缩文件中。
		//先关闭标记文件的写入指针，否则数据在缓冲区 还没有写进去
		
		fclose(fOutFlag);
		
		FILE* fInFlag = fopen("flag.temp", "rb");
		if (!fInFlag) {
			std::cout << "error for fInFlag" << std::endl;
			exit(0);
		}

		//要在文件的最后面，保存标记、文件大小 和标记大小
		//标记和Huffman树中一样,用位来标记，每写满 就保存进去
		//因此  将它们定义出来
		//文件大小  在前面已经求过了
		
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

		//将文件大小 和 标记大小 写入
		fwrite(&fileSize, sizeof(fileSize), 1, fOut);
		fwrite(&flagSize, sizeof(fileSize), 1, fOut);

		
		//关闭相关的文件指针
		fclose(fIn);
		fclose(fOut);
		fclose(fInFlag);

		//删除创建的零时文件		
		remove("flag.temp");
		return compressFileName;
	}

	void GetMaxMatch(USH & machtListHead, USH & maxDistant, UCH & maxMatch)
	{
		//machtListHead  匹配链的头
		//maxDistant	距离长度对的 距离
		//maxMatch		距离长度对的 长度
		//目前：
		//如果匹配，没有走到头 就继续匹配	匹配肯定是在窗口范围内的  因此 直接在窗口中查找就好
		USH nextMatch = machtListHead;		//表示是否到达匹配的结尾
		USH limit = _start > MAX_DISTANT ? _start - MAX_DISTANT : 0;
		USH maxMatchCount = 255;				//表示最多可以查找最长串的次数
		UCH curMatch = 0;		//记录 匹配了几个字符
		while (maxMatchCount-- && nextMatch > limit) {		//当没有走到list的最后一个的时候 就一直走
			USH curMatchPos = nextMatch;	//表示 匹配开始的下标     能是相同hashaddr 说明 前三个字符相同了 因此可以多匹配三个
			USH curDistant = _start - curMatchPos;	//表示本次匹配中 距离
			USH curStart = _start;				//从正在压缩的字符 向后走
			curMatch = 0;
			while (curMatch <= 255 && _buffer[curMatchPos++] == _buffer[curStart++]) {		//如果匹配字符相同 那么本次匹配长度加一
				curMatch++;
			}
			if (curMatch > maxMatch) {
				maxMatch = curMatch;
				maxDistant = curDistant;
				if (curMatch == 255) {
					//达到最大匹配长度， 就算能匹配 也不进行匹配了
					return;
				}
			}

			//运行到这里 说明 本次匹配结束了，开始下一次匹配
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
			//找到匹配串的位置
			UCH* pCurStart = _buffer + matchHead;
			UCH* pEnd = pStart + MAX_MATCH;
			curMatchLen = 0;

			//找单挑连的匹配长度
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
		//先判断是不是对应的压缩文件
		std::string fileSuffix = fileName.substr(fileName.rfind('.'));
		if (fileSuffix != ".lzp") {
			std::cout << "error for file type" << std::endl;
			exit(0);
		}

		std::string unCompressFileName = "unCompress" + fileName.substr(0, fileName.rfind('.'));

		//读压缩文件
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

		//写解压缩文件
		FILE* fOut = fopen(unCompressFileName.c_str(), "wb");
		if (fOut == nullptr) {
			std::cout << "error for fOut" << std::endl;
			exit(0);
		}

		//读标记大小
		FILE* fInFlagSize = fopen(fileName.c_str(), "rb");
		if (fOut == nullptr) {
			std::cout << "error for fOut" << std::endl;
			exit(0);
		}
		ULL flagSize = 0;
		int offset = 0 - sizeof(flagSize);
		fseek(fInFlagSize, offset, SEEK_END);
		fread(&flagSize, sizeof(flagSize), 1, fInFlagSize);

		//读文件大小
		FILE* fInFileSize = fopen(fileName.c_str(), "rb");
		if (fOut == nullptr) {
			std::cout << "error for fOut" << std::endl;
			exit(0);
		}
		ULL fileSize = 0;
		fseek(fInFileSize, 0 - sizeof(flagSize) - sizeof(fileSize), SEEK_END);
		fread(&fileSize, sizeof(fileSize), 1, fInFileSize);

		//读标记
		FILE* fInFlag = fopen(fileName.c_str(), "rb");
		if (fOut == nullptr) {
			std::cout << "error for fOut" << std::endl;
			exit(0);
		}
		fseek(fInFlag, 0 - sizeof(flagSize) - sizeof(fileSize) - flagSize, SEEK_END);


		//进行匹配，前面的
		FILE* fMatchPrev = fopen(unCompressFileName.c_str(), "rb");
		if (fOut == nullptr) {
			std::cout << "error for fOut" << std::endl;
			exit(0);
		}


		UCH flag = 0;			//读取的标记
		char flagLeftCount = -1;		//右移的次数
		//11110000
		while (fileSize) {

			//读取标记 通过标记判断是源字符 还是键值对 
			if (flagLeftCount < 0) {		//
				flag = fgetc(fInFlag);
				flagLeftCount = 7;
			}

			if (((flag >> flagLeftCount) & 1) == 1) {
				//说明是键值对
				USH matchDistant = 0;	//距离
				UCH matchlong = 0;	//长度


				fread(&matchDistant, sizeof(matchDistant), 1, fIn);
				matchlong = fgetc(fIn);
				fflush(fOut);		//可能匹配会经过当前的 比如 abcabcabc

				fseek(fMatchPrev, 0 - matchDistant, SEEK_END);
				//下面matchlong 会减因此先赋值
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
				//说明是源字符
				UCH ch = fgetc(fIn);
				fputc(ch, fOut);
				fileSize--;
			}
			flagLeftCount--;
		}
		return unCompressFileName;
	}

	~Lz77Compress()		//析构函数
	{
		delete[] _buffer;
	}

private:
	HashTable _hashTable;			//用来存储查询字符下标
	UCH* _buffer;					//滑动窗口
	size_t _lookAhead;					//表示剩余多少元素待压缩
	size_t _start;						//表示当前压缩到窗口的哪个下标
};