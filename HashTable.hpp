#pragma once

#include <stdlib.h>
#include <string.h>

#define WINDOWSSISZ (32 * 1024)			//hash 表每个单表的大小  为 32K  
#define WSIZE WINDOWSSISZ				//窗口大小

#define UCH unsigned char
#define USH unsigned short
#define ULL unsigned long long

const USH HASH_BITS = 15;							//哈希桶的个数为2^15
const USH HASH_SIZE = (1 << HASH_BITS);				//哈希表的大小
const USH HASH_MASK = HASH_SIZE - 1;				//哈希掩码


static const size_t MIN_MATCH = 3;					//最小匹配长度
// 最大匹配长度: 匹配长度占1个字节，而前三个字符已经匹配过了 因此最大匹配长度为：255+3
//本代码中用的是255   如果需要更改 那么 在压缩算法中 获取最长匹配串 就需要更改
static const size_t MAX_MATCH = 255 + 3;				//最长匹配串长度


class HashTable
{
public:
	HashTable(size_t size)		//构造函数
		:_hashTable(new USH[2 * size])
		,_prev(_hashTable)
		,_head(_prev + size)
	{
		//初始化hashTable, _prev 指向的空间的值会被覆盖，因此只需要初始化_head 指向的空间
		memset(_head, -1, sizeof(USH) * size);
	}

	void GetHushAddr(USH& hashAddr, UCH ch)	
	{
		//计算HashTable中的下标
		hashAddr = (((hashAddr) << H_SHIFT()) ^ (ch)) & HASH_MASK;
	}

	USH H_SHIFT()
	{
		return (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH;
	}

	void HushInsert(USH& hashAddr, UCH ch, USH pos, USH& matchHead)
	{
		//hushAddr	上一次的哈希地址
		//ch		先行缓冲区中的第一个字符
		//pos		ch在滑动窗口中的位置
		//match		如果匹配 保存匹配串的第一个位置，也就是返回匹配链的头
		// 计算哈希地址
		GetHushAddr(hashAddr, ch);
		// 随着压缩的不断进行，pos肯定会大于WSIZE，与上WMASK保证不越界

		_prev[pos & HASH_MASK] = _head[hashAddr];
		matchHead = _head[hashAddr];
		_head[hashAddr] = pos;
	}

	USH GetListNext(USH& matchHead)
	{
		return _prev[matchHead];
	}

	void updateHashTable()
	{
		//00102000
		//将右窗口的匹配位置更新到左窗口
		//更新prev
		for (size_t i = 0; i < WSIZE; ++i)
		{
			if (_prev[i] >= WSIZE)
				_prev[i] -= WSIZE;
			else
				_prev[i] = -1;
		}
		//更新head
		for (size_t i = 0; i < HASH_SIZE; i++)
		{
			if (_head[i] >= WSIZE)
				_head[i] -= WSIZE;
			else
				_head[i] = -1;
		}
	}

	~HashTable()
	{
		if (_hashTable) {
			delete[] _hashTable;
			_hashTable = nullptr;
			_prev = nullptr;
			_head = nullptr;
		}

	}

private:
	USH *_hashTable;		//大小设为32K的两倍 因为有两个表
	USH *_prev;
	USH *_head;
};