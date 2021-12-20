#include "archive.h"
#include "iostream"
#include "fstream"
#include "debug.h"
#include "libs/zlib-1.2.11/zlib.h"

//#define CHUNK 64 * 1024

// 压缩
bool deflate_file(std::string file_in, std::string file_out, archive_on_processing callback, long chunk_size)
{
    std::fstream fin(file_in, std::fstream::in | std::fstream::binary);
    error_check(!fin.fail(), "deflate_file: failed to open the in-file to deflate: " + file_in);
    std::fstream fout(file_out, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    error_check(!fout.fail(), "deflate_file: failed to open the out-file to deflate: " + file_out);

    int ret, flush;
    unsigned have;
    z_stream strm;
    uint8_t* in = new uint8_t[chunk_size];
    uint8_t* out = new uint8_t[chunk_size];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int compressionLevel = Z_DEFAULT_COMPRESSION; // -1 ~ 9

    ret = deflateInit(&strm, compressionLevel);
    if (ret != Z_OK)
        return false;

    size_t source_len = get_file_length(file_in);
    size_t bytes_compressed = 0;
    size_t byte_read;

    if (callback)
        callback(0, source_len);

    do {
        fin.read((char*)in, chunk_size);
        error_check(!fin.bad(), "deflate_file: could not read the in-file to deflate: " + file_in);
        byte_read = fin.gcount();
        strm.avail_in = byte_read;
        flush = strm.avail_in > 0 ? Z_NO_FLUSH : Z_FINISH;
        strm.next_in = in;

        do {
            strm.avail_out = chunk_size;
            strm.next_out = out;

            ret = deflate(&strm, flush);
            error_check(ret != Z_STREAM_ERROR, "deflate_file: unexpected deflate() return value: " + std::to_string(ret));

            have = chunk_size - strm.avail_out;
            fout.write((char*)out, have);
            error_check(!fout.bad(), "deflate_file: could not write the out-file to deflate: " + file_out);
        } while (strm.avail_out == 0);

        bytes_compressed += byte_read;

        if (callback)
            callback(bytes_compressed, source_len);

        error_check(strm.avail_in == 0, "deflate_file: unexpected value: strm.avail_in == " + std::to_string(strm.avail_in));
    } while (flush != Z_FINISH);

    deflateEnd(&strm);
    delete[] in;
    delete[] out;
    fin.close();
    fout.close();

    return true;
}

// 解压
bool inflate_file(std::string file_in, std::string file_out, archive_on_processing callback, long chunk_size)
{
    std::fstream fin(file_in, std::fstream::in | std::fstream::binary);
    error_check(!fin.fail(), "failed to open the in-file to inflate: " + file_in);
    std::fstream fout(file_out, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    error_check(!fout.fail(), "failed to open the out-file to inflate: " + file_out);

    int ret;
    unsigned have;
    z_stream strm;
    uint8_t* in = new uint8_t[chunk_size];
    uint8_t* out = new uint8_t[chunk_size];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return false;

    size_t source_len = get_file_length(file_in);
    size_t bytes_decompressed = 0;
    size_t byte_read;

    if (callback)
        callback(0, source_len);

    do {
        fin.read((char*)in, chunk_size);
        byte_read = fin.gcount();
        strm.avail_in = byte_read;
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        do {
            strm.avail_out = chunk_size;
            strm.next_out = out;

            ret = inflate(&strm, Z_NO_FLUSH);
            error_check(ret != Z_STREAM_ERROR, "inflate_file: unexpected inflate() return value: " + std::to_string(ret));

            switch (ret)
            {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                return false;
            }

            have = chunk_size - strm.avail_out;
            fout.write((char*)out, have);
        } while (strm.avail_out == 0);

        bytes_decompressed += byte_read;

        if (callback)
            callback(bytes_decompressed, source_len);

    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    delete[] in;
    delete[] out;
    fin.close();
    fout.close();

    return ret == Z_STREAM_END;
}

// 解压到文件
bool inflate_to_file(std::fstream& file_in, size_t offset, size_t length, std::string file_out, archive_on_processing callback, long chunk_size)
{
    error_check(!file_in.fail(), "failed to open the in-file(binary) to inflate");
    std::fstream fout(file_out, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    error_check(!fout.fail(), "failed to open the out-file to inflate: " + file_out);

    file_in.clear();
    file_in.seekg(offset);
    uint64_t totalBytesRead = 0;

    int ret;
    unsigned have;
    z_stream strm;
    uint8_t* in = new uint8_t[chunk_size];
    uint8_t* out = new uint8_t[chunk_size];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return false;

    if (callback)
        callback(0, length);

    size_t byte_read;

    do {
        auto bulk = length - totalBytesRead >= chunk_size ? chunk_size : length - totalBytesRead;

        file_in.read((char*)in, bulk);
        strm.avail_in = file_in.gcount();
        if (strm.avail_in == 0)
            break;
        totalBytesRead += strm.avail_in;
        strm.next_in = in;

        do {
            strm.avail_out = chunk_size;
            strm.next_out = out;

            ret = inflate(&strm, Z_NO_FLUSH);
            error_check(ret != Z_STREAM_ERROR, "inflate_file: unexpected inflate() return value: " + std::to_string(ret));

            switch (ret)
            {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                return false;
            }

            have = chunk_size - strm.avail_out;
            fout.write((char*)out, have);
        } while (strm.avail_out == 0);

        if (callback)
            callback(totalBytesRead, length);

    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    delete[] in;
    delete[] out;
    fout.close();

    return ret == Z_STREAM_END;
}