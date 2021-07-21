#include "archive.h"
#include "iostream"
#include "fstream"
#include "debug.h"
#include "zlib-1.2.11/zlib.h"

#define CHUNK 64 * 1024

// 压缩
bool deflate_file(std::string fileIn, std::string fileOut)
{
    std::fstream fin(fileIn, std::fstream::in | std::fstream::binary);
    std::fstream fout(fileOut, std::fstream::out | std::fstream::binary | std::fstream::trunc);

    error_check(!fin.fail(), "deflate_file: could not open the in-file to deflate: " + fileIn);
    error_check(!fout.fail(), "deflate_file: could not open the out-file to deflate: " + fileOut);

    int ret, flush;
    unsigned have;
    z_stream strm;
    uint8_t* in = new uint8_t[CHUNK];
    uint8_t* out = new uint8_t[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int compressionLevel = Z_DEFAULT_COMPRESSION; // -1 ~ 9

    ret = deflateInit(&strm, compressionLevel);
    if (ret != Z_OK)
        return false;

    do {
        fin.read((char*)in, CHUNK);
        error_check(!fin.bad(), "deflate_file: could not read the in-file to deflate: " + fileIn);
        strm.avail_in = fin.gcount();
        flush = strm.avail_in > 0 ? Z_NO_FLUSH : Z_FINISH;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = deflate(&strm, flush);
            error_check(ret != Z_STREAM_ERROR, "deflate_file: unexpected deflate() return value: " + std::to_string(ret));

            have = CHUNK - strm.avail_out;
            fout.write((char*)out, have);
            error_check(!fout.bad(), "deflate_file: could not write the out-file to deflate: " + fileOut);
        } while (strm.avail_out == 0);

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
bool inflate_file(std::string fileIn, std::string fileOut)
{
    std::fstream fin(fileIn, std::fstream::in | std::fstream::binary);
    std::fstream fout(fileOut, std::fstream::out | std::fstream::binary | std::fstream::trunc);

    error_check(!fin.fail(), "inflate_file: could not open the in-file to inflate: " + fileIn);
    error_check(!fout.fail(), "inflate_file: could not open the out-file to inflate: " + fileOut);

    int ret;
    unsigned have;
    z_stream strm;
    uint8_t* in = new uint8_t[CHUNK];
    uint8_t* out = new uint8_t[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return false;

    do {
        fin.read((char*)in, CHUNK);
        strm.avail_in = fin.gcount();
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
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

            have = CHUNK - strm.avail_out;
            fout.write((char*)out, have);
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    delete[] in;
    delete[] out;
    fin.close();
    fout.close();

    return ret == Z_STREAM_END;
}

// 解压到文件
bool inflate_to_file(std::fstream& fileIn, size_t offset, size_t length, std::string fileOut)
{
    std::fstream fout(fileOut, std::fstream::out | std::fstream::binary | std::fstream::trunc);

    error_check(!fileIn.fail(), "inflate_to_file: could not open the in-file(binary) to inflate");
    error_check(!fout.fail(), "inflate_to_file: could not open the out-file to inflate: " + fileOut);

    fileIn.clear();
    fileIn.seekg(offset);
    uint64_t totalBytesRead = 0;

    int ret;
    unsigned have;
    z_stream strm;
    uint8_t* in = new uint8_t[CHUNK];
    uint8_t* out = new uint8_t[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return false;

    do {
        auto bulk = length - totalBytesRead >= CHUNK ? CHUNK : length - totalBytesRead;

        fileIn.read((char*)in, bulk);
        strm.avail_in = fileIn.gcount();
        if (strm.avail_in == 0)
            break;
        totalBytesRead += strm.avail_in;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
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

            have = CHUNK - strm.avail_out;
            fout.write((char*)out, have);
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    delete[] in;
    delete[] out;
    fout.close();

    return ret == Z_STREAM_END;
}