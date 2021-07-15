#pragma once
#include "string"

struct app_arguments
{
    // 通用参数
    std::string input = ""; // unused
    std::string output = "";

    // 压缩
    bool pack = false;
    std::string pack_src = "";
    bool pack_no_hash = false;
    std::string pack_exec = "";

    // 解压
    bool extract = false;
    //std::string extract_dist = "";

    // 详情
    bool detail = false;

    // 错误
    bool optarg_required = false;
    bool unknown_opt = false;
    std::string invaild_opt_name = "";

    // 运行过程中不隐藏窗口
    bool always_show_console = false;

    // 帮助
    bool help = false;
};

app_arguments parse_args(int argc, char** argv);