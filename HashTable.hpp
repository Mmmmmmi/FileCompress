#pragma once

#include <stdlib.h>
#include <string.h>

#define WINDOWSSISZ (32 * 1024)			//hash ��ÿ������Ĵ�С  Ϊ 32K  
#define WSIZE WINDOWSSISZ				//���ڴ�С

#define UCH unsigned char
#define USH unsigned short
#define ULL unsigned long long

const USH HASH_BITS = 15;							//��ϣͰ�ĸ���Ϊ2^15
const USH HASH_SIZE = (1 << HASH_BITS);				//��ϣ��Ĵ�С
const USH HASH_MASK = HASH_SIZE - 1;				//��ϣ����


static const size_t MIN_MATCH = 3;					//��Сƥ�䳤��
// ���ƥ�䳤��: ƥ�䳤��ռ1���ֽڣ���ǰ�����ַ��Ѿ�ƥ����� ������ƥ�䳤��Ϊ��255+3
//���������õ���255   �����Ҫ���� ��ô ��ѹ���㷨�� ��ȡ�ƥ�䴮 ����Ҫ����
static const size_t MAX_MATCH = 255 + 3;				//�ƥ�䴮����


class HashTable
{
public:
	HashTable(size_t size)		//���캯��
		:_hashTable(new USH[2 * size])
		,_prev(_hashTable)
		,_head(_prev + size)
	{
		//��ʼ��hashTable, _prev ָ��Ŀռ��ֵ�ᱻ���ǣ����ֻ��Ҫ��ʼ��_head ָ��Ŀռ�
		memset(_head, -1, sizeof(USH) * size);
	}

	void GetHushAddr(USH& hashAddr, UCH ch)	
	{
		//����HashTable�е��±�
		hashAddr = (((hashAddr) << H_SHIFT()) ^ (ch)) & HASH_MASK;
	}

	USH H_SHIFT()
	{
		return (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH;
	}

	void HushInsert(USH& hashAddr, UCH ch, USH pos, USH& matchHead)
	{
		//hushAddr	��һ�εĹ�ϣ��ַ
		//ch		���л������еĵ�һ���ַ�
		//pos		ch�ڻ��������е�λ��
		//match		���ƥ�� ����ƥ�䴮�ĵ�һ��λ�ã�Ҳ���Ƿ���ƥ������ͷ
		// �����ϣ��ַ
		GetHushAddr(hashAddr, ch);
		// ����ѹ���Ĳ��Ͻ��У�pos�϶������WSIZE������WMASK��֤��Խ��

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
		//���Ҵ��ڵ�ƥ��λ�ø��µ��󴰿�
		//����prev
		for (size_t i = 0; i < WSIZE; ++i)
		{
			if (_prev[i] >= WSIZE)
				_prev[i] -= WSIZE;
			else
				_prev[i] = -1;
		}
		//����head
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
	USH *_hashTable;		//��С��Ϊ32K������ ��Ϊ��������
	USH *_prev;
	USH *_head;
};