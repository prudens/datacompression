#ifndef MIN_MAX_HEAP_H
#define MIN_MAX_HEAP_H

#include <algorithm>
#include <vector>
#include <type_traits>
template<class T,class Cmp = std::less>
class MaxMinHeap
{
    typedef size_t index;
public:

    MaxMinHeap()
    { }

    ~MaxMinHeap()
    {}


    void BuildHeap( T *arr, size_t size )
    {
        data_.clear();
        data_.insert( data_.begin(), arr, arr + size );
        for ( int i = ((int)size-1) / 2; i >= 0; i-- )
        {
            AdjustHeap( i );
        }
    }
    void BuildHeap( std::vector<T>data, size_t size )
    {
        data_.clear();
        data_.insert( data_.begin(), data.begin(), data.end() );
        for ( size_t i = size / 2; i >= 0; i-- )
        {
            AdjustHeap( i );
        }
    }
    void Pop()
    {
        if ( data_.empty() )
        {
            throw std::range_error( "range_error, data is empty!" );
        }
        data_[0] = std::move(data_.back());
        data_.pop_back();
        AdjustHeap( 0 );// sort again
    }

    T Top()const
    {
        if (data_.empty())
        {
            throw std::range_error( "range_error, data is empty!" );
        }
        return data_[0];
    }

    T& Top()
    {
        if ( data_.empty() )
        {
            throw std::range_error( "range_error, data is empty!" );
        }
        return data_[0];
    }


    void Insert( T &data )
    {
        data_.insert( data_.begin(), data );
        AdjustHeap( 0 );
    }

    void Insert( T&&data )
    {
        data_.insert( data_.begin(), data );
        AdjustHeap( 0 );
    }

     size_t size()const _NOEXCEPT{ return data_.size(); }
    
    bool empty() const _NOEXCEPT{ return  data_.empty(); }
    void clear()_NOEXCEPT{ data_.clear(); }
        
protected:
    void AdjustHeap( index node )
    {
        size_t size = data_.size();
        while ( true )
        {
            index l = Left( node );
            index r = Right( node );
            index next = node;

            if ( l < size && less_(data_[l], data_[node]) )
            {
                next = l;
            }
            if ( r < size && less_(data_[r], data_[next]) )
            {
                next = r;
            }
            if ( next != node )
            {
                std::swap( data_[node], data_[next] );
                node = next;
            }
            else
            {
                break;
            }
        }
    }
    index Left( index node )const _NOEXCEPT{ return node * 2 + 1; }
    index Right( index node )const _NOEXCEPT{ return ( node + 1 ) * 2; }
private:
    std::vector<T> data_;// 第一个数据不要了，方便处理节点关系。
    Cmp less_;
};



void test_maxminheap( int argc, char**argv )
{
    auto func = [=] ( const int &left, const int & right ) { return left > right; };//c++14支持const auto &left的形式
    MaxMinHeap<int, decltype(func)> heap;
    int arr[10] = { 33, 11, 4, 20, 54, 100, 22, 44, 7, 10 };
    heap.BuildHeap( arr, 10 );
    heap.Pop();
    heap.Insert( 5 );
    heap.Insert( 100 );
    heap.Insert( 101 );
    while ( heap.size() > 1  )
    {
        int top = heap.Top();
        heap.Pop();
        std::cout << top << std::endl;
    }
}

#endif