#ifndef HUFFMAN_H
#define HUFFMAN_H
#include "MinMaxHeap.hpp"
#include <stdint.h>
#include <limits>
#include <stack>
#include <bitset> 

struct stHuffmanCode
{
    uint8_t word;
    uint8_t len;
    uint32_t code; // 最长32位，否则要执行修正
};


class HuffmanTree
{
public:
    class HuffmanNode
    {
    public:
        int value;
        double fs;
        uint32_t preCode;
        uint8_t len;
        HuffmanNode*left, *right;
        HuffmanNode() :fs( 0 ), left( nullptr ), right( nullptr ), len(0),preCode(0){}
        inline bool isLeaf()const { return !left && !right; }
    };
    HuffmanTree() {}
    ~HuffmanTree() { Clear(); }
public:
    HuffmanNode* MakeTree( std::vector<HuffmanNode*>list )
    {
        size_t size = list.size();
        // 通过对数值的比较方式的改变，可以得到不同的等价的Huffman码表
        auto lambda = [] ( const HuffmanNode* left, const HuffmanNode* right )
        {
            return left->fs < right->fs;
        };
    
        auto minivar = [] ( const HuffmanNode* left, const HuffmanNode* right )
        {
            if( left->fs < right->fs)return true;
            if ( (left->fs == right->fs) && (left->isLeaf()))return true;
            return false;
        };

        MaxMinHeap<HuffmanNode*, decltype( minivar )> heap;
        for ( size_t i = 0; i < size; i++ )
        {
            HuffmanNode* pNode = new HuffmanNode;
            pNode = list[i];
            heap.Insert( pNode );
        }
        
        while ( heap.size() > 1 )
        {
            HuffmanNode* l = heap.Top();
            heap.Pop();
            HuffmanNode* r = heap.Top();
            heap.Pop();
            HuffmanNode *n = new HuffmanNode;
            n->fs = l->fs + r->fs;
            n->left = l;
            n->right = r;
            heap.Insert( n );
        }
        return heap.Top();
    }
    void Statistics( uint8_t*set, size_t size )
    {
        int bSet[256] = { 0 };
        for ( size_t i = 0; i < size; i++ )
        {
            bSet[set[i]]++;
        }
        std::vector<HuffmanNode*>HuffmanNodes;
        for ( size_t i = 0; i < size;i++ )
        {
            if ( bSet[i]>0 )
            {
                HuffmanNode*pNode = new HuffmanNode;
                pNode->fs = bSet[i];
                pNode->value = i;
                HuffmanNodes.push_back( pNode );
            }
        }
        root_ = MakeTree( HuffmanNodes );
    }
    stHuffmanCode*HuffCodeTable()
    {
        if ( root_ == NULL )
            return nullptr;
        //树非空
        HuffmanNode* p = root_;
        std::stack<HuffmanNode*> s;
        while ( !s.empty() || p )
        {
            //一直遍历到左子树最下边，边遍历边保存根节点到栈中
            while ( p )
            {
                s.push( p );
                if ( p->isLeaf() )
                {
                    table_[p->value].word = p->value;
                    table_[p->value].len = p->len;
                    table_[p->value].code = p->preCode;
                }
                HuffmanNode* pParent = p;
                p = p->left;
                if (p)
                {
                    p->preCode = pParent->preCode << 1 ;
                    p->len = pParent->len + 1;
                }
            }
            if ( !s.empty() )
            {
                p = s.top();
                s.pop();

                //进入右子树，开始新的一轮左子树遍历(这是递归的自我实现)
                HuffmanNode* pParent = p;
                p = p->right;
                if ( p )
                {
                    p->preCode = ( pParent->preCode << 1 ) | 1;
                    p->len = pParent->len+ 1;
                }
            }

            
        }
        return table_;
    }
    stHuffmanCode* CanonicalCode()
    {
        auto minivar = [] ( const stHuffmanCode* left, const stHuffmanCode* right )
        {
            return left->len < right->len;
            
        };

        MaxMinHeap<stHuffmanCode*, decltype( minivar )> heap;
        for ( int i = 0; i < 256;i++ )
        {
            if ( table_[i].len > 0 )
            {
                heap.Insert(&table_[i]);
            }
        }
        stHuffmanCode*preCode = NULL;
        while ( heap.empty() )
        {
            stHuffmanCode* pNode = heap.Top();
            if ( !preCode )
            {
                pNode->code = 0;// 第一个
                preCode = pNode;
            }
            else if (preCode->len == pNode->len)
            {
                pNode->code = preCode->code + 1;
                preCode = pNode;
            }
            else if ( preCode->len < pNode->len )
            {
                pNode->code = (preCode->code + 1) << 2;
                preCode = pNode;
            }
            heap.Pop();
        }
        return table_;
    }
protected:
    void Clear()
    {
        if ( root_ == NULL )
            return ;
        //树非空
        HuffmanNode* p = root_;
        std::stack<HuffmanNode*> s;
        while ( !s.empty() || p )
        {
            //一直遍历到左子树最下边，边遍历边保存根节点到栈中
            while ( p )
            {
                s.push( p );

                p = p->left;

            }
            if ( !s.empty() )
            {
                p = s.top();
                s.pop();

                //进入右子树，开始新的一轮左子树遍历(这是递归的自我实现)
                HuffmanNode* pParent = p;
                p = p->right;
                delete pParent;
            }
        }

    }
    HuffmanNode*root_;// 构造完一颗树只需要保存定点即可。
    stHuffmanCode table_[256];
};
void test_huffman( int argc, char**argv )
{
    HuffmanTree huffman;
    uint8_t set[10] = { 1, 1, 1, 1, 2, 2, 3, 3, 4, 5 };
    huffman.Statistics( set, 10 );
    stHuffmanCode*pTable = huffman.HuffCodeTable();
    for ( int i = 0; i < 256;i++ )
    {
        if ( pTable[i].len>0 )
            std::cout << "the word is : " << (int)pTable[i].word << " the len is : " << (int)pTable[i].len << " the code is : " << std::bitset< sizeof( int ) * 8>( pTable[i].code ) << std::endl;
    }
    pTable = huffman.CanonicalCode();
    for ( int i = 0; i < 256; i++ )
    {
        if ( pTable[i].len>0 )
            std::cout << "the word is : " << (int)pTable[i].word << " the len is : " << (int)pTable[i].len << " the code is : " << std::bitset< sizeof( int ) * 8>( pTable[i].code ) << std::endl;
    }
}
#endif