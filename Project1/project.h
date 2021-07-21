#pragma once

// 启用错误检查（发生错误时会显示弹框）
#define ENABLED_ERROR_CHECK

#define PROJECT_NAME "LittleWrapper"
#define VERSION_TEXT "1.0.8"
#define PROJ_VER PROJECT_NAME " v" VERSION_TEXT

#define PATH_MAX 256

#define MAGIC_HEADER "0123456789abcdefghijkmnlopqrtsuvwxyz|"
#define MAGIC_LEN (sizeof(MAGIC_HEADER) / sizeof(char) - 1)
#define PRESERVE_LEN 1024