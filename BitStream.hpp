#include <stdio.h>
#include <stdint.h>
#include <stdexcept>
#include <iostream>
#ifndef BIT_STREAM_H
#define BIT_STREAM_H
typedef uint8_t BIT;
template<size_t buff_size>
class CBitStreamBase
{
public:
    CBitStreamBase()
    {
        file_ = nullptr;
        read_offset_ = 0;
        read_bit_offset_ = 0;
        read_buff_size_ = 0;
        write_bit_offset_ = 0;
        write_buff_size_ = 0;
    }
    ~CBitStreamBase()
    {
        Close();
    }

    bool Open( const char* filename, const char* mode )
    {
        Close();
        int ret = 0;
        if ( ( ret = fopen_s( &file_, filename, mode ) ) != 0 )
        {
            file_ = nullptr;
            return false;
        }
        return true;
    }

    void Close()
    {
        if ( !file_ )
        {
            return;
        }

        while ( write_bit_offset_ > 0 )
        {
            WriteBit( 0 );
        }

        WriteInternal();
        fflush( file_ );
        fclose( file_ );
        file_ = nullptr;
    }

    BIT ReadBit()
    {
        if (read_buff_size_ == 0 )
        {
            if ( ReadInternal() == 0 )
            {
                ASSERT(  "read to eof");
                return 0;
            }
        }
        
        BIT bit = ( readbuf_[read_offset_] >> ( 7 - read_bit_offset_ ) & 1 );
        if (++read_bit_offset_ == 8)
        {
            ++read_offset_;
            read_bit_offset_ = 0;
        }
        if (read_offset_ >= read_buff_size_)
        {
            read_buff_size_ = 0;
        }
        return bit;
    }

    void WriteBit( BIT bit )
    {
        writebuf_[write_buff_size_] = (writebuf_[write_buff_size_] << 1 )| bit;
        if ( ++write_bit_offset_ == 8 )
        {
            write_buff_size_++;
            write_bit_offset_ = 0;
        }
        if ( write_buff_size_ >= buff_size )
        {
            if ( WriteInternal() != write_buff_size_ )
            {
                ASSERT( false && "write error" );
            }
            write_buff_size_ = 0;
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
        return static_cast<size_t>( size * 8 );
    }

protected:
    size_t WriteInternal()
    {
        if ( file_ && write_buff_size_ > 0 )
        {
            return fwrite( writebuf_, 1, write_buff_size_, file_ );
        }
        return 0;
    }

    size_t ReadInternal()
    {
        if ( file_ && read_buff_size_ == 0 )
        {
            read_buff_size_ = fread( readbuf_, 1, buff_size, file_ );
        }
        return read_buff_size_;
    }

private:
    FILE*    file_;                        // 文件句柄
    uint8_t  readbuf_[buff_size];          // 读缓存
    uint8_t  writebuf_[buff_size];         // 写缓存
    uint16_t read_offset_;                 // 读缓存中已读个数
    uint8_t  read_bit_offset_;             // 当前字节已经读取的bit数
    uint16_t read_buff_size_;              // 读缓存大小
    uint8_t  write_bit_offset_;            // 写缓存当前字节的bit位置
    uint16_t write_buff_size_;             // 已经写入writebuf_字节数
};

typedef CBitStreamBase<1024> CBitStream;//根据测试取最佳的缓存buff_size;


void test_bitstream( int argc, char**argv )
{
    CBitStream bs_read,bs_write;
    if ( !bs_read.Open( "E:/a.txt", "rb" ) )
        return;
    if ( !bs_write.Open( "E:/b.txt", "wb+" ) )
        return;
    int size = bs_read.size();
    while ( size-- > 0 )
    {
        bs_write.WriteBit( bs_read.ReadBit() );
        //std::cout << (int)bs.ReadBit() ;
    }

}
#endif