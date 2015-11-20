#include <stdio.h>
#include <stdint.h>
#include <stdexcept>
#include <iostream>
#ifndef BIT_STREAM_H
#define BIT_STREAM_H
typedef uint8_t BIT;
class CBitStream
{
public:
    CBitStream()
    :file_( nullptr ), point_( -1 ), readcount_(0){}
    ~CBitStream()
    {
        if ( file_ )
        {
            if (outputpt>0)
            {
                fwrite(  output_,1,(outputpt + 7 )/8,file_ );
            }
            fflush( file_ );
            fclose( file_ );
        }
    }
    bool Open( char*filename )
    {
        int ret = 0;
        if ( file_ ) fclose( file_ );
        if ( (ret =fopen_s( &file_, filename, "wb+" )) != 0)
        {
            file_ = nullptr;
            return false;
        }
        
        return true;
    }
    BIT ReadBit()
    {
        if ( point_ == -1 || point_ >= readcount_*8 )
        {
            readcount_ = fread( buf_, sizeof( uint8_t ), 1024, file_ );
            point_ = 0;
        }
        if (readcount_ == 0)
        {
            throw std::length_error( "已读到最后一bit,超出了读取范围" );
        }
        BIT ret = (buf_[point_ / 8] >> (point_ % 8)) & 1;
        point_++;
        return ret;
    }
    void WriteBits( char bits, size_t bitCount )
    {
        if ( outputpt >= 1023 * 8 )
        {
            int count = outputpt / 8;
            fwrite(output_,1,count,file_);
            memset( output_, 0, count );
            if (outputpt%8 > 0)
            {
                output_[0] = output_[count + 1];
            }
            
            outputpt %= 8;
        }
        size_t cur = outputpt/8;
        size_t notbitFill = 8 - outputpt % 8;
        outputpt += bitCount;
        output_[cur] = output_[cur] | (bits << ( notbitFill-bitCount));
        if (notbitFill < bitCount)
        {
            cur++;
            notbitFill += 8 - bitCount;
            output_[cur] |= bits << ( 8 - notbitFill );
        }
    }
    size_t size()
    {
        if (!file_)
        {
            return 0;
        }
        fseek( file_, 0L, SEEK_END );
        long size = ftell( file_ );
        fseek( file_, 0L, SEEK_SET );
        return static_cast<uint32_t>(size*8);
    }
protected:
private:
    FILE*file_;
    uint8_t buf_[1024];//
    uint8_t output_[1024];
    int point_;
    int readcount_;
    int outputpt;
};

void test_bitstream( int argc, char**argv )
{
    CBitStream bs;
    bs.Open("E:/a.txt");
    int size = bs.size();
    while ( size-- > 0 )
    {
        std::cout << (int)bs.ReadBit() ;
    }

}
#endif