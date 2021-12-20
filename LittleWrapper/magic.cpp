#include "magic.h"
#include "iostream"
#include "fstream"
#include "debug.h"

#define MAGIC_HEADER_A "0123456789abcdefgh"
#define MAGIC_HEADER_B "ijkmnlopqrtsuvwxyz|"
#define MAGIC_HEADER MAGIC_HEADER_A MAGIC_HEADER_B
#define MAGIC_LEN_A (sizeof(MAGIC_HEADER_A) / sizeof(char))
#define MAGIC_LEN_B (sizeof(MAGIC_HEADER_B) / sizeof(char))
#define MAGIC_LEN (MAGIC_LEN_A + MAGIC_LEN_B - 1)

#define PRESERVE_LEN 1024

using namespace std;

static const uint8_t preserveSection[PRESERVE_LEN] = MAGIC_HEADER "{\"offset\": 0}";

static char* get_magic_string_runtime()
{
    char* magic = new char[MAGIC_LEN];
    memcpy(magic, MAGIC_HEADER_A, MAGIC_LEN_A - 1);
    memcpy(magic + (MAGIC_LEN_A - 1), MAGIC_HEADER_B, MAGIC_LEN_B);
    return magic;
}

static size_t get_magic_offset(fstream& fin, uint8_t* magic, int magic_len, bool clear_state)
{
    if (clear_state)
    {
        fin.clear();
        fin.seekg(0, fstream::beg);
    }

    size_t result = -1;

    int buf_len = 32 * 1024;
    char* buf = new char[buf_len];

    int continue_match = 0; // 当前能匹配到多少个byte/char
    size_t total_read = 0; // 总共读取了多少bytes
    size_t last_read = 0; // 上次读了多少bytes

    do {
        // 读取一部分字节到缓冲区
        fin.read(buf, buf_len);
        error_check(!fin.bad(), "failed to read from stream when locate magic header");
        last_read = (size_t) fin.gcount();
        total_read += last_read;

        // 在缓冲区里不断对比
        for (int i = 0; i < last_read; i++)
        {
            size_t addr = total_read - last_read + i;
            // 如果有byte能匹配
            if (buf[i] == magic[continue_match])
            {
                continue_match += 1;

                // 调试信息，只有匹配度大于5才显示
                /*
                if (continue_match > 5)
                {
                    char* str_matched = new char[continue_match + 1];
                    str_matched[continue_match + 1 - 1] = 0;
                    memcpy(str_matched, magic, continue_match);

                    printf("match at index: %d, addr: 0x%llx str_matched: %s\n", continue_match, addr, str_matched);
                    delete[] str_matched;
                }
                */

                // 如果全部匹配成功
                if (continue_match == magic_len)
                {
                    result = addr - continue_match + 1;
                    break;
                }
            } else {
                continue_match = 0;
            }
        }
    } while (last_read > 0 && result == -1);

    delete[] buf;

    if (clear_state)
    {
        fin.clear();
        fin.seekg(0, fstream::beg);
    }

    return result;
}

size_t get_preserved_data_address(fstream& fin, bool clear_state)
{
    char* magic = get_magic_string_runtime();
    int magic_len = MAGIC_LEN - 1;

    size_t magic_offset = get_magic_offset(fin, (uint8_t*) magic, magic_len, clear_state);
    delete[] magic;
    size_t result = magic_offset != 0 ? magic_offset + MAGIC_LEN - 1 : 0;
    error_check(result != 0, "failed to locate magic header");
    return result;
}

int get_preserved_data_len()
{
    int len = 0;
    uint8_t* ptr = (uint8_t*) get_preserved_data();
    while (true) {
        if (ptr[len] != 0)
            len += 1;
        else
            return len;
    }
}

char* get_preserved_data()
{
    return (char*) preserveSection + MAGIC_LEN - 1;
}