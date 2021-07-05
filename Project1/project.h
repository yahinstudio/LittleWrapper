#pragma once

// WINDOW_MODE会影响链接器的主入口函数（有了隐藏consles窗口后，一般用不上这个了）
//#define WINDOW_MODE

#define ENABLED_ERROR_CHECK

#define PROJECT_NAME "LittleWrapper"
#define VERSION_TEXT "1.0.2"
#define PROJ_VER PROJECT_NAME " v" VERSION_TEXT

#define PATH_MAX 256

#define MAGIC_HEADER "0123456789abcdefghijkmnlopqrtsuvwxyz|"
#define MAGIC_LEN (sizeof(MAGIC_HEADER) / sizeof(char) - 1)
#define PRESERVE_LEN 1024

/*
1.0.1: 2021年7月2日: 
  1. 会同步返回主程序的exitcode
  2. 增加lasterror信息显示
  3. 增加绝对路径的支持

1.0.2: 2021年7月5日:
  1. 使用路径MD5命名临时文件，放置冲突
  2. 重写参数处理相关代码
  3. 使用command参数代替_start.txt文件
*/