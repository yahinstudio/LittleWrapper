#pragma once

// WINDOW_MODE会影响链接器的主入口函数（有了隐藏consles窗口后，一般用不上这个了）
//#define WINDOW_MODE

#define ENABLED_ERROR_CHECK

#define PROJECT_NAME "LittleWrapper"
#define VERSION_TEXT "1.0.5"
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

1.0.3: 2021年7月5日:
  1. 修正一些调试信息的文本错误
  2. 修复不能为输出文件自动创建父目录的问题
  3. 加大error_check()的缓冲区大小

1.0.4: 2021年7月8日
  1. 修复打包时--output参数不能直接填文件名的问题
  2. 增加持久显示console的选项

1.0.5: 2021年7月15日
  1. 修复文件校验判断错误的情况
  2. 优化帮助文本的输出
  3. 支持使用命令行设置窗口是否隐藏
  4. 缩短临时目录的路径

*/