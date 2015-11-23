#ifndef ARITHMETIC_CODING_HPP
#define ARITHMETIC_CODING_HPP
// 参考《数据压缩导论 》第四章：算术编码
#include <string>
#include <array>
#include <stdint.h>
#include <xutility>
#include <stdlib.h>
#include "common.h"

class ArithmeticCoding
{
public:
    typedef uint8_t value_type;
    typedef uint32_t count_t;
    static const int WORD_BITS = sizeof(value_type)*8;//一字节
    static const int MSB_INDEX = WORD_BITS - 1;
    static const int SSB_INDEX = MSB_INDEX - 1;
    static const int TABLE_SIZE = 256;
    static const int MAX_TABLE_IDX = TABLE_SIZE - 1;
    static const value_type MSB_XOR = 1 << MSB_INDEX;
    std::array<count_t, TABLE_SIZE> table;
    int _encode_bit_len;
    int _total_src_byte;
    ArithmeticCoding() throw() {}
    ~ArithmeticCoding() {}
public:
    enum EncodeMode{
        eStatisticMode,
        eAdaptiveMode,
    };
    std::vector<value_type> Encode( EncodeMode mode,std::vector<uint8_t> src)
    {
        switch ( mode )
        {
        case ArithmeticCoding::eStatisticMode:
            return StatisticEncode( src );
            break;
        case ArithmeticCoding::eAdaptiveMode:
            return AdaptiveEncode( src );
            break;
        default:
            return AdaptiveEncode( src );
            break;
        }
        return std::vector<value_type>();
    }

    std::vector<uint8_t> Decode( EncodeMode mode, std::vector<value_type> src,size_t dst_size )
    {
        switch ( mode )
        {
        case ArithmeticCoding::eStatisticMode:
            return StatisticDecode( src, dst_size );
            break;
        case ArithmeticCoding::eAdaptiveMode:
            return AdaptiveDecode( src, dst_size );
            break;
        default:
            return AdaptiveDecode( src, dst_size );
            break;
        }
        return std::vector<uint8_t>();
    }
    std::vector<value_type>  AdaptiveEncode( std::vector<uint8_t> src )
    {
        std::vector<value_type> dst;

        table.fill( 1 );
        for ( int i = 1; i < TABLE_SIZE; i++ )
        {
            table[i] += table[i - 1];// 累计分布函数
        }

        _total_src_byte = table[MAX_TABLE_IDX];


        value_type lower = 0, upper = -1;
        int Scale3 = 0;
        value_type result = 0;
        int bits = 0;
        for ( auto i : src )
        {
            int lower1 = lower + ( ( (uint64_t)upper - lower + 1 ) * ( i == 0 ? 0 : table[i - 1] ) ) / _total_src_byte;
            int upper1 = lower + ( ( (uint64_t)upper - lower + 1 ) * table[i] ) / _total_src_byte - 1;
            lower = lower1;
            upper = upper1;
            //std::cout << "[" << (int)lower<<"," << (int)upper<<"]"<<std::endl;
            int LMSB = lower >> MSB_INDEX;
            int UMSB = upper >> MSB_INDEX;
            while ( LMSB == UMSB || ( lower >> SSB_INDEX & 1 )
                    && ( ( upper >> SSB_INDEX & 1 ) == 0 ) )
            {
                if ( LMSB == UMSB )
                {
                    result = result << 1 | LMSB;
                    if ( ++bits == WORD_BITS )
                    {
                        dst.push_back( result );
                        bits = 0;
                        result = 0;
                    }
                    lower <<= 1;
                    upper = upper << 1 | 1;
                    while ( Scale3 > 0 )
                    {
                        Scale3--;
                        result = result << 1 | LMSB ^ 1;
                        if ( ++bits == WORD_BITS )
                        {
                            dst.push_back( result );
                            bits = 0;
                            result = 0;
                        }
                    }
                }
                else
                {
                    lower <<= 1;
                    upper = upper << 1 | 1;
                    lower ^= MSB_XOR;
                    upper ^= MSB_XOR;//include upper |= 1;
                    Scale3++;
                }
                LMSB = lower >> MSB_INDEX;
                UMSB = upper >> MSB_INDEX;
                // std::cout << "[" << (int)lower << "," << (int)upper << "]" << std::endl;
            }

            //更新table表
            int index = i;
            while ( index < TABLE_SIZE )
            {
                table[index++]++;
            }
            _total_src_byte++;
        }
        int count = WORD_BITS;
        if ( Scale3 > 0 )
        {
            count--;
            int LMSB = lower >> MSB_INDEX;
            result <<= 1;
            result |= LMSB;//先放进去一个位

            if ( ++bits == WORD_BITS )
            {
                dst.push_back( result );
                bits = 0;
                result = 0;
            }
            LMSB ^= 1;
            while ( Scale3-- )
            {
                result <<= 1;
                result |= LMSB;
                if ( ++bits == WORD_BITS )
                {
                    dst.push_back( result );
                    bits = 0;
                    result = 0;
                }
            }
        }

        //剩余31位

        for ( int i = count - 1; i >= 0; i-- )
        {
            int LMSB = lower >> i & 1;
            result <<= 1;
            result |= LMSB;
            if ( ++bits == WORD_BITS )
            {
                dst.push_back( result );
                bits = 0;
                result = 0;

            }
        }
        int bitlen = bits + WORD_BITS * dst.size();
        if ( bits > 0 )
        {
            result <<= ( WORD_BITS - bits );
            dst.push_back( result );
        }
        _encode_bit_len = bitlen;
        return dst;
    }
private:
    std::vector<value_type> StatisticEncode(std::vector<uint8_t> src)
    {

        std::vector<value_type> dst;
        
        table.fill( 0 );

        for ( auto &c : src )
            table[c]++;
        for ( int i = 1; i < TABLE_SIZE; i++ )
        {
            table[i] += table[i - 1];// 累计分布函数
        }
         _total_src_byte = table[MAX_TABLE_IDX];
//#define test
#ifdef test
        table[1]=40;
        table[2] = 41;
        table[3] = 50;
        _total_src_byte = 50;
#endif // test

        
        value_type lower = 0, upper = -1;
        int Scale3 = 0;
        value_type result = 0;
        int bits=0;
        for ( auto i:src )
        {
            int lower1 = lower + ( ( (uint64_t)upper - lower + 1 ) * ( i == 0 ? 0 : table[i - 1] )) / _total_src_byte;
            int upper1 = lower + ( ( (uint64_t)upper - lower + 1 ) * table[i] ) / _total_src_byte - 1;
            lower = lower1;
            upper = upper1;
            //std::cout << "[" << (int)lower<<"," << (int)upper<<"]"<<std::endl;
            int LMSB = lower >> MSB_INDEX;
            int UMSB = upper >> MSB_INDEX;
            while ( LMSB == UMSB  || (  lower >> SSB_INDEX & 1)
                    && ( ( upper >> SSB_INDEX & 1 ) == 0 ) )
            {
                if ( LMSB == UMSB )
                {
                    result = result << 1 | LMSB;
                    if ( ++bits == WORD_BITS )
                    {
                        dst.push_back( result );
                        bits = 0;
                        result = 0;
                    }
                    lower <<= 1;
                    upper = upper << 1 | 1;
                    while ( Scale3 > 0 )
                    {
                        Scale3--;
                        result = result << 1 | LMSB ^ 1;
                        if ( ++bits == WORD_BITS )
                        {
                            dst.push_back( result );
                            bits = 0;
                            result = 0;
                        }
                    }
                }
                else
                {
                    lower <<= 1;
                    upper = upper<<1|1;
                    lower ^= MSB_XOR;
                    upper ^= MSB_XOR;//include upper |= 1;
                    Scale3++;
                }
                LMSB = lower >> MSB_INDEX;
                UMSB = upper >> MSB_INDEX;
               // std::cout << "[" << (int)lower << "," << (int)upper << "]" << std::endl;
            }
        }
        int count = WORD_BITS;
        if ( Scale3 > 0 )
        {
            count--;
            int LMSB = lower >> MSB_INDEX;
            result <<= 1;
            result |= LMSB;//先放进去一个位
           
            if ( ++bits == WORD_BITS )
            {
                dst.push_back( result );
                bits = 0;
                result = 0;
            }
            LMSB ^= 1;
            while ( Scale3-- )
            {
                result <<= 1;
                result |= LMSB;
                if (++bits==WORD_BITS)
                {
                    dst.push_back( result );
                    bits = 0;
                    result = 0;
                }
            }
        }

        //剩余31位

        for ( int i = count-1; i >= 0;i-- )
        {
            int LMSB = lower >> i & 1;
            result <<= 1;
            result |= LMSB;
            if ( ++bits == WORD_BITS )
            {
                dst.push_back( result );
                bits = 0;
                result = 0;
                
            }
        }
        int bitlen = bits + WORD_BITS * dst.size();
        if ( bits > 0 )
        {
            result <<= (WORD_BITS-bits);
            dst.push_back( result );
        }
        _encode_bit_len = bitlen;
        return dst;
    }
    std::vector<uint8_t> StatisticDecode( std::vector<value_type> src, size_t dst_size  )
    {
        size_t encode_bit_len = _encode_bit_len;;
        std::vector<uint8_t> dst;

       auto srcIt = src.begin();
       auto t = *srcIt;
       value_type readc = 0;
       encode_bit_len -= WORD_BITS;
       int bits_remain = 0;
       if ( encode_bit_len > 0 )
       {
           readc = *( ++srcIt );
           bits_remain = WORD_BITS;
       }
       value_type lower = 0, upper = -1;
       while ( true )
       {
           int range = (uint64_t)upper - lower + 1;
           count_t k = ( (t-lower+1)*_total_src_byte - 1 ) / range;

           auto it = this->find( table.begin(), table.end(), k );

           auto index = 0;
           if ( it != table.end())
           {
               index = std::distance( table.begin(), it );
           }
           else
           {
               index = MAX_TABLE_IDX;// 不应该执行到这里来
           }
           dst.push_back( index );
           
           int tt = ( range * table[index] ) / _total_src_byte - 1;
           value_type lower1 = lower + range * (index == 0?0:table[index-1]) / _total_src_byte;
           value_type upper1 = lower + tt;
           lower = lower1;
           upper = upper1;
          // std::cout << "[" << (int)lower << "," << (int)upper << "]" << std::endl;
           int LMSB = lower >> MSB_INDEX;
           int UMSB = upper >> MSB_INDEX;
           if ( dst.size() == dst_size )
           {
               return dst;
           }
           while ( LMSB == UMSB || ( lower >> SSB_INDEX & 1 )
                   && ( ( upper >> SSB_INDEX & 1 ) == 0 ) )
           {
               upper = upper << 1 | 1;
               lower <<= 1;
               t = t << 1;
               if ( encode_bit_len >0)
               {
                   if ( bits_remain > 0 )
                   {
                       t = t | readc >> MSB_INDEX;
                       bits_remain--;
                       readc <<= 1;
                       encode_bit_len--;
                   }

                   if ( bits_remain == 0 && encode_bit_len > 0 )
                   {
                       readc = *( ++srcIt );
                       bits_remain = WORD_BITS;
                   }
               }

               if ( LMSB != UMSB )
               {
                   lower ^= MSB_XOR;
                   upper ^= MSB_XOR;//include upper |= 1;
                   t ^= MSB_XOR;
               }
               LMSB = lower >> MSB_INDEX;
               UMSB = upper >> MSB_INDEX;
              // std::cout << "[" << (int)lower << "," << (int)upper << "]" << std::endl;
           }
       }
        return dst;
    }
    std::vector<uint8_t> AdaptiveDecode( std::vector<value_type> src, size_t dst_size )
    {
        size_t encode_bit_len = _encode_bit_len;
        std::vector<uint8_t> dst;
        table.fill( 1 );
        for ( int i = 1; i < TABLE_SIZE; i++ )
        {
            table[i] += table[i - 1];// 累计分布函数
        }

        _total_src_byte = table[MAX_TABLE_IDX];

            
        auto srcIt = src.begin();
        auto t = *srcIt;
        value_type readc = 0;
        encode_bit_len -= WORD_BITS;
        int bits_remain = 0;
        if ( encode_bit_len > 0 )
        {
            readc = *( ++srcIt );
            bits_remain = WORD_BITS;
        }
        value_type lower = 0, upper = -1;
        while ( true )
        {
            int range = (uint64_t)upper - lower + 1;
            count_t k = ( ( t - lower + 1 )*_total_src_byte - 1 ) / range;

            auto it = this->find( table.begin(), table.end(), k );

            auto index = 0;
            if ( it != table.end() )
            {
                index = std::distance( table.begin(), it );
            }
            else
            {
                index = MAX_TABLE_IDX;// 不应该执行到这里来
            }
            dst.push_back( index );

            int tt = ( range * table[index] ) / _total_src_byte - 1;
            value_type lower1 = lower + range * ( index == 0 ? 0 : table[index - 1] ) / _total_src_byte;
            value_type upper1 = lower + tt;
            lower = lower1;
            upper = upper1;
            // std::cout << "[" << (int)lower << "," << (int)upper << "]" << std::endl;
            int LMSB = lower >> MSB_INDEX;
            int UMSB = upper >> MSB_INDEX;
            if ( dst.size() == dst_size )
            {
                return dst;
            }
            while ( LMSB == UMSB || ( lower >> SSB_INDEX & 1 )
                    && ( ( upper >> SSB_INDEX & 1 ) == 0 ) )
            {
                upper = upper << 1 | 1;
                lower <<= 1;
                t = t << 1;
                if ( encode_bit_len > 0 )
                {
                    if ( bits_remain > 0 )
                    {
                        t = t | readc >> MSB_INDEX;
                        bits_remain--;
                        readc <<= 1;
                        encode_bit_len--;
                    }

                    if ( bits_remain == 0 && encode_bit_len > 0 )
                    {
                        readc = *( ++srcIt );
                        bits_remain = WORD_BITS;
                    }
                }

                if ( LMSB != UMSB )
                {
                    lower ^= MSB_XOR;
                    upper ^= MSB_XOR;//include upper |= 1;
                    t ^= MSB_XOR;
                }
                LMSB = lower >> MSB_INDEX;
                UMSB = upper >> MSB_INDEX;
                // std::cout << "[" << (int)lower << "," << (int)upper << "]" << std::endl;
            }

            //更新table表
            int temp = index;
            for ( ; temp < TABLE_SIZE; temp++ )
            {
                table[temp]++;
            }
            _total_src_byte++;
        }
        
        return dst;
    }
    
public:
    template<class _FwdIt, class _Ty>
    _FwdIt find( _FwdIt first, _FwdIt last, const _Ty &value )
    {
        for ( ; first != last;first++ )
        {
            if (value < *first)
            {
                return first;
            }
        }

        return last;
    }
};


void test_arithmeticcoding( int argc, char**argv )
{
    // = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 9, 8, 7, 6, 7, 8, 9 };
    for ( int i = 0; i < 100;i++ )
    {
        std::vector<uint8_t> vec;
        srand( i );
        int k = rand()%100;
        for ( int j = 0; j < k; j++ )
        {
            vec.push_back( rand( )%100+1 );
        }
        if (vec.size() == 0)
        {
            continue;
        }

        ArithmeticCoding coding1;
        
        auto dst = coding1.Encode( ArithmeticCoding::eStatisticMode, vec );
        auto src = coding1.Decode( ArithmeticCoding::eStatisticMode, dst, vec.size() );
//              auto dst = coding.Encode( ArithmeticCoding::eStatisticMode, vec );
//              auto src = coding.Decode( ArithmeticCoding::eStatisticMode, dst, vec.size() );
        if ( src != vec )
        {
            
            std::cout << "统计模型的编码前： \n";
            for (auto i : vec)
            {
                std::cout << (int)i <<"\t";
            }
            std::cout <<std::endl<< "编码后： \n";
            for ( auto i : src )
            {
                std::cout << (int)i << "\t";
            }
            std::cout << std::endl;
            //std::cout << "编解码器有错误！！！" << std::endl;
            std::cout << "bit_len:" << coding1._encode_bit_len << std::endl;
        }


//         ArithmeticCoding coding2;
//          dst = coding2.Encode( ArithmeticCoding::eAdaptiveMode, vec );
//          src = coding2.Decode( ArithmeticCoding::eAdaptiveMode, dst, vec.size() );
        //     auto dst = coding.Encode( ArithmeticCoding::eStatisticMode, vec );
        //     auto src = coding.Decode( ArithmeticCoding::eStatisticMode, dst, vec.size() );
//         if ( src != vec )
//         {
// 
//             std::cout << "自适应的编码前： \n";
//             for ( auto i : vec )
//             {
//                 std::cout << (int)i << "\t";
//             }
//             std::cout << std::endl << "编码后：\n";
//             for ( auto i : src )
//             {
//                 std::cout << (int)i << "\t";
//             }
//             std::cout << std::endl;
//             //std::cout << "编解码器有错误！！！" << std::endl;
//             std::cout << "bit_len:" << coding1._encode_bit_len << std::endl;
// 
//         }
    }
    ArithmeticCoding coding;
    std::vector<uint8_t> vec = { 2,2,1,2,3,1,1,2 };
    auto dst = coding.Encode( ArithmeticCoding::eAdaptiveMode, vec );
    auto src = coding.Decode( ArithmeticCoding::eAdaptiveMode, dst,vec.size() );
//     auto dst = coding.Encode( ArithmeticCoding::eStatisticMode, vec );
//     auto src = coding.Decode( ArithmeticCoding::eStatisticMode, dst, vec.size() );
    if ( src != vec )
    {
        std::cout << "编解码器有错误！！！" << std::endl;
    }
    std::vector<uint32_t>v = { 40, 41, 49, 50 };
    std::vector<uint32_t> testv = { 0, 1, 40, 41, 42, 49, 50 };
    auto size = std::distance( v.begin(), v.begin() );
    for ( auto i : testv )
    {
        auto it = coding.find(v.begin(),v.end(),i);
        if (it != v.end())
        {
            std::cout << "the internal value is " << *it << std::endl;
        }
        else
        {
            std::cout << "the internal value is " << *(v.crbegin())<<std::endl;
        }
    }
    system( "pause" );
}
#endif//ARITHMETIC_CODING_HPP