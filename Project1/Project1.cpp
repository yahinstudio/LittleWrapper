#include <iostream>
#include <stdio.h>
#include "utils.h"
#include "env.h"
#include "cJSON-1.7.14/cJSON.h"
#include "archive.h"
#include "dir_utils.h"
#include "wrapper.h"
#include "runner.h"
#include "project.h"
#include "args_utils.h"
#include "windows.h"

using namespace std;

uint8_t preserveSection[PRESERVE_LEN] = MAGIC_HEADER "{\"offset\":0, \"len\":0}";

void output_help(bool disabled_dialog_in_winmain=true)
{
    string help_messge = "";
    help_messge += "Sub commands available:\n";
    help_messge += "  --help                                          - show this help infomation\n";
    help_messge += "  --pack <source_dir> --exec <command> \n";
    help_messge += "         [--output <output_file>] [--no-hashing]  - pack files into a executable.\n";
    help_messge += "  --extract [--output <output_dir>]               - extract the bundles inside this executable.\n";
    help_messge += "  --extract=[output_dir]                          - extract the bundles inside this executable.\n";
    help_messge += "  -e[output_dir]                                  - extract the bundles inside this executable.\n";
    help_messge += "  --detail                                        - detail the bundles inside this executable.\n";
    help_messge += "  --show-console                                  - run with console visible(default value, higher priority).\n";
    help_messge += "  --hide-console  or  -x                          - run with console invisible.\n";
    help_messge += "  --suppress-output  or  -u                       - suppress the output of decompresing.\n";

#if defined(ENTRANCE_WINMAIN)
    if (!disabled_dialog_in_winmain)
        winmain_dialog("参数不正确", help_messge);
#endif

    printf("%s", help_messge.c_str());
}

int run_prog(string executable, bool show_console_set, bool show_console, bool no_output)
{
    string temp_dir = get_temp_directory() + "LW-" + get_string_md5(executable).substr(0, 8);

    printf("execute\n");
    return run_program(executable, temp_dir, show_console_set, show_console, no_output);
}

int functions(app_arguments args, string workdir, string executable)
{
    printf("suppress-output: %d\n", args.suppress_output);
    printf("show-console: %d\n", args.show_console);

    if(args.help) {
        output_help(false);
    } else if (args.optarg_required) {
        winmain_dialog("参数不正确", string("选项\"") + args.invaild_opt_name + "\"需要参数");
        printf("require option for arg: %s\n", args.invaild_opt_name.c_str());
        output_help();
    } else if (args.unknown_opt) {
        winmain_dialog("参数不正确", string("无效的参数: ") + args.invaild_opt_name);
        printf("invaild option: %s\n", args.invaild_opt_name.c_str());
        output_help();
    } else if (args.pack) {
        // 数据目录
        string source_dir = args.pack_src;
        if (is_relative_path(source_dir))
        {
            bool starts_with_slash = string_starts_with(string_replace(source_dir, "/", "\\"), "\\");
            source_dir = workdir + (starts_with_slash ? "" : "\\") + source_dir;
        }

        // 文件检查
        if (!file_exists(source_dir) || !is_file_a_dir(source_dir))
        {
            winmain_dialog("参数不正确", string("目录找不到或者不是个文件夹: ") + source_dir);
            printf("the source_dir could not be found or was not a directory: %s\n", source_dir.c_str());
            return 1;
        }

        // 命令检查
        if (args.pack_exec == "") 
        {
            winmain_dialog("参数不正确", "需要跟上 --exec 参数");
            printf("the option \"exec\" was required\n");
            return 1;
        }

        printf("<<< %s\n", args.pack_exec.c_str());

        printf("pack with source_dir: %s\n", source_dir.c_str());
        string output_file = args.output != "" ? (is_relative_path(args.output)? workdir + "\\" : "") + args.output : workdir + "\\" + get_filename(source_dir) + ".exe";
        string temp_dir = "temp-compressed";
        printf("output: %s\n", output_file.c_str());

        optiondata arg;
        arg.check_hash = !args.pack_no_hash;
        arg.exec = args.pack_exec;
        arg.show_console = args.show_console == args.hide_console ? true : args.show_console;
        lw_pack(executable, output_file, source_dir, temp_dir, arg);

        // 清理临时文件
        if (file_exists(temp_dir))
            remove_dir(temp_dir);

        winmain_dialog("完成", string("打包完成: ") + output_file);
        printf("\nfinish\n");
    } else if (args.extract) {
        printf("extract\n");
        string dest = args.output != "" ? args.output : args.extract_dest;
        string output_dir = dest != "" ? (is_relative_path(dest) ? workdir + "\\" : "") + dest : workdir + "\\" + get_filename(executable);

        // 清理临时文件
        if (file_exists(output_dir))
            remove_dir(output_dir);

        lw_extract(executable, output_dir, false, false);
        winmain_dialog("完成", string("解压完成: ") + output_dir);
    } else if (args.detail) {
        printf("detail\n");
        string source = executable;
        lw_detail(source);
    } else {
        return run_prog(get_exe_path(), args.show_console != args.hide_console, args.show_console, args.suppress_output);
    }

    return 0;
}

// 窗口主函数
int main(int argc, char** argv);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    // 转换参数
    int argc = 0;
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &argc);

    char** argv = new char* [argc];
    for (int i = 0; i < argc; i++)
        argv[i] = from_wchar_to_char(wargv[i]);

    return main(argc, argv);
}

int main(int argc, char** argv)
{
    printf("%s\n\n", PROJ_VER);
    if(argc > 999999999999999999)
        printf("preserveSection: %s\n\n", (char*)preserveSection + MAGIC_LEN);
    
    try {
        return functions(parse_args(argc, argv), get_current_work_dir(), get_exe_path());
    } catch (jumpdata_not_found_exception& e) {
        winmain_dialog("程序损坏", "无法读取标识数据(MagicHeader)");
        printf("The MagicHeader could not be read.\n");
    } catch (jumpdata_invaild_exception& e) {
        winmain_dialog("程序损坏", "无法读取标识数据(Jumpdata)");
        printf("The Jumpdata could not be read.\n");
    } catch (metadata_not_found_exception& e) {
        winmain_dialog("无法运行", "应用程序内没有任何打包数据");
        printf("The executable did not contain any bundles.\n");
    } catch (metadata_invaild_exception& e) {
        winmain_dialog("程序损坏", "无法读取标识数据(Metadata)");
        printf("The Metadata could not be read.\n");
    } catch (binaries_damaged_exception& e) {
        winmain_dialog("程序损坏", "无法读取对应的数据");
        printf("The bundles inside the executable have been damaged.\n");
    } catch (exception& e) {
        winmain_dialog("无法运行", string("未知错误") + e.what());
        printf("未知错误: %s.\n", e.what());
    } catch (...) {
        winmain_dialog("无法运行", "未知错误");
        printf("未知错误.\n");
    }

    printf("finish");
}
