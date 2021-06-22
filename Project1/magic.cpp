#include "magic.h"
#include "iostream"
#include "fstream"

using namespace std;

uint64_t get_magic_offset(fstream& fs, uint8_t* magic, int magic_len)
{
    uint64_t result = 0;

    fs.clear();
    fs.seekg(0, fs.beg);

    //fs.seekg(0, fs.end);
    //int file_len = fs.tellg();
    //fs.seekg(0, fs.beg);

    //printf("len: %x\n", file_len);

    int buf_len = 32 * 1024;
    //int buf_len = 0x14f10;
    char* buf = new char[buf_len];

    int continueToMatch = 0;
    uint64_t totalRead = 0;
    while (true)
    {
        fs.read(buf, buf_len);
        auto readBytes = fs.gcount();
        totalRead += readBytes;

        for (uint64_t i = 0; i < readBytes; i++)
        {
            uint64_t matchPos = 0;
            uint64_t matchCount = 0;

            for (int j = 0; j < magic_len - continueToMatch; j++)
            {
                if (continueToMatch > 0)
                {
                    //printf("start from: %d, %llx\n", continueToMatch, totalRead - readBytes + i);
                }

                // 读取到chunk边界了
                if (i + j >= readBytes)
                {
                    if (matchPos > 0)
                        continueToMatch = j;
                    //printf("split at: %llx\n", totalRead - readBytes + i);
                    //printf("i: %llx, j: %d\n\n", i , j);
                    break;
                }

                if (buf[i + j] == magic[j + continueToMatch])
                {
                    //printf("find at: %llx, %c\n", totalRead - readBytes + i + j, magic[j + continueToMatch]);
                    //printf("find at: %c => %c\n", magic[j + continueToMatch], buf[i + j]);
                    matchPos = i;
                    matchCount += 1;
                }
                else {
                    matchPos = -1;
                    matchCount = 0;
                    break;
                }

                //printf("s: %llx\n", totalRead - readBytes + i);
            }

            //printf("matcnt: %lld\n", matchCount);

            if (matchCount == magic_len - continueToMatch)
            {
                result = totalRead - readBytes + matchPos - continueToMatch;
                //printf("++++++");
                for (int q = 0; q < magic_len - continueToMatch; q++)
                {
                    //printf("%c", (char)buf[matchPos + q]);
                }
                //printf(" : %llx\n", result);
            }
            else {
                //printf("matchCount: %lld， req: %d\n", matchCount, magic_len - continueToMatch);
            }

            if (continueToMatch != 0)
                break;
        }

        if (readBytes == 0)
            break;
    }

    delete[] buf;

    fs.clear();
    fs.seekg(0, fs.beg);
    return result;
}
