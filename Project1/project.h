#pragma once

// WINDOW_MODE会影响链接器的主入口函数（有了隐藏consles窗口后，一般用不上这个了）
//#define WINDOW_MODE

#define ENABLED_ERROR_CHECK

#define PROJECT_NAME "LittleWrapper"
#define VERSION_TEXT "1.0.0"

#define PATH_MAX 256

#define MAGIC_HEADER "0123456789abcdefghijkmnlopqrtsuvwxyz|"
#define MAGIC_LEN (sizeof(MAGIC_HEADER) / sizeof(char) - 1)
#define PRESERVE_LEN 1024