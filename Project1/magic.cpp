#include "magic.h"
#include "iostream"
#include "fstream"

#define MAGIC_HEADER "0123456789abcdefghijkmnlopqrtsuvwxyz|"
#define MAGIC_LEN (sizeof(MAGIC_HEADER) / sizeof(char) - 1)
#define PRESERVE_LEN 1024

using namespace std;

uint8_t preserveSection[PRESERVE_LEN] = MAGIC_HEADER "{\"offset\":0, \"len\":0}";

static size_t get_magic_offset(fstream& fin)
{
    uint8_t* magic = (uint8_t*) MAGIC_HEADER;
    int magic_len = MAGIC_LEN;

    size_t result = 0;

    fin.clear();
    fin.seekg(0, fin.beg);

    //fs.seekg(0, fs.end);
    //int file_len = fs.tellg();
    //fs.seekg(0, fs.beg);

    //printf("len: %x\n", file_len);

    int buf_len = 32 * 1024;
    //int buf_len = 0x14f10;
    char* buf = new char[buf_len];

    int continueToMatch = 0;
    size_t totalRead = 0;
    while (true)
    {
        fin.read(buf, buf_len);
        auto readBytes = (size_t)fin.gcount();
        totalRead += readBytes;

        for (size_t i = 0; i < readBytes; i++)
        {
            size_t matchPos = 0;
            size_t matchCount = 0;

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

    fin.clear();
    fin.seekg(0, fin.beg);
    return result;
}

size_t get_jumpdata_address(fstream& fin)
{
    size_t magic_offset = get_magic_offset(fin);
    return magic_offset != 0 ? magic_offset + MAGIC_LEN : 0;
}

string get_preserved_data()
{
    return string((char*) preserveSection);
}