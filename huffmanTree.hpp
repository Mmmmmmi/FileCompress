#ifndef __HUFFMANNODE_H__
#define __HUFFMANNODE_H__

#include <vector>
#include <queue>


template <class Weight>
struct HuffmanNode
{
	HuffmanNode()
		:_leftNode(nullptr)
		, _rightNode(nullptr)
		, _parentNode(nullptr)
		, _weight()
	{}
	
	HuffmanNode(const Weight& weight)
		:_leftNode(nullptr)
		, _rightNode(nullptr)
		, _parentNode(nullptr)
		, _weight(weight)
	{}

	bool operator>(const HuffmanNode& node1) const
	{
		return _weight > node1._weight;
	}

	HuffmanNode* _leftNode;         //huffman树节点的左孩子
	HuffmanNode* _rightNode;        //右孩子
	HuffmanNode* _parentNode;       //双亲
	Weight _weight;                 //权重
};


class LessCompare
{
public:
	bool operator() (const HuffmanNode<CharInfo>* node1, const HuffmanNode<CharInfo>* node2) const
	{
		return *node1 > *node2;
	}
};


template <class Weight>
class HuffmanTree
{
public:
	HuffmanTree()
		:_root(NULL)
	{}

	void CreateHuffmanTree(std::vector<Weight>& nodeInfo)
	{
		std::priority_queue<HuffmanNode<Weight>*, std::vector<HuffmanNode<Weight>*>, LessCompare> pqhuffman;
		for (int i = 0; i < 256; i++) {
			if (nodeInfo[i]._charCount != 0) {
				//只要计数不为0 就说明有效
				char charNum[32] = { 0 };
				_itoa(nodeInfo[i]._charCount, charNum, 10);
				HuffmanNode<CharInfo>* temp = new HuffmanNode<CharInfo>(nodeInfo[i]);
				pqhuffman.push(temp);
			}
		}

		while (pqhuffman.size() > 1) {
			HuffmanNode<CharInfo>* left = pqhuffman.top();
			pqhuffman.pop();
			HuffmanNode<CharInfo>* right = pqhuffman.top();
			pqhuffman.pop();
			Weight tempweight = left->_weight + right->_weight;
			HuffmanNode<Weight>* temp = new HuffmanNode<Weight>(tempweight);
			temp->_leftNode = left;
			temp->_leftNode->_parentNode = temp;
			temp->_rightNode = right;
			temp->_rightNode->_parentNode = temp;
			pqhuffman.push(temp);
		}

		_root = pqhuffman.top();
		pqhuffman.pop();
	}

	void GetHuffmanCode(std::vector<Weight>& info)
	{
		GetCode(_root, info);
	}

	HuffmanNode<Weight>* GetRoot()
	{
		return _root;
	}

	void DestroyHuffmanTree(HuffmanNode<CharInfo>* root)
	{
		if (root == nullptr) {
			return;
		}
		DestroyHuffmanTree(root->_leftNode);
		DestroyHuffmanTree(root->_rightNode);
		delete root;
		root = nullptr;
	}

	~HuffmanTree()
	{
		if (_root) {
			DestroyHuffmanTree(_root);
		}
	}

private:
	void GetCode(HuffmanNode<Weight>* root, std::vector<Weight>& info)
	{
		if (root == nullptr) {
			return;
		}
		if (root->_leftNode == nullptr && root->_rightNode == nullptr) {
			//叶子节点
			HuffmanNode<Weight>* pcur = root;
			HuffmanNode<Weight>* pparent = root->_parentNode;
			while (pparent != nullptr) {
				//当节点的双亲节点不为空 就说明 没有走到根节点
				if (pcur == pparent->_leftNode) {
					info[root->_weight._char]._huffmanCode += '0';
				}
				else {
					info[root->_weight._char]._huffmanCode += '1';
				}
				pcur = pparent;
				pparent = pparent->_parentNode;
			}
			reverse(info[root->_weight._char]._huffmanCode.begin(), info[root->_weight._char]._huffmanCode.end());
			return;
		}
		GetCode(root->_leftNode, info);
		GetCode(root->_rightNode, info);
	}
	
	HuffmanNode<Weight>* _root;
};


#endif //__HUFFMANNODE_H__