#include <iostream>
#include <stdio.h>
#include "utils.h"
#include "env.h"
#include "wingetopt-0.95/getopt.h"
#include "cJSON-1.7.14/cJSON.h"
#include "archive.h"
#include "dir_utils.h"
#include "wrapper.h"
#include "runner.h"
#include "project.h"
#include "windows.h"

// 程序读取自身而不是-attach.exe文件
#define RELEASE_MODE

using namespace std;

uint8_t preserveSection[PRESERVE_LEN] = MAGIC_HEADER "{\"offset\":0, \"len\":0}";

void output_help()
{
    printf("Sub commands available:\n");
    printf("  - help\n");
    printf("  - attach <data_folder>\n");
    printf("  - attach-no-hash <data_folder>\n");
    printf("  - detach\n");
    printf("  - detail\n");
    printf("  - export\n");
}

#if defined(WINDOW_MODE)
int main(int argc, char** argv);
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    // 转换参数
    int argc = 0;
    wchar_t** wargv;
    char** argv = new char* [argc];

    wargv = CommandLineToArgvW(GetCommandLineW(), &argc);

    for (int i = 0; i < argc; i++)
        argv[i] = from_wchar_to_char(wargv[i]);

    main(argc, argv);
}
#endif

int main(int argc, char** argv)
{
    //parseArguments(argc, argv);
    printf("%s v%s\n\n", PROJECT_NAME, VERSION_TEXT);
    printf("preserveSection: %s\n", (char*)preserveSection + MAGIC_LEN);

    string executable = get_exe_path();
    string workdir = get_current_work_dir();

    option longops[] = {
        {"help",            no_argument      , 0,  0},
        {"attach",          required_argument, 0,  1},
        {"attach-no-hash",  required_argument, 0,  2},
        {"detach",          no_argument,       0,  3},
        {"detail",          no_argument,       0,  4},
        {"export",          no_argument,       0,  5},
        {0, 0, 0, 0}
    };
    
    int ch;
    while ((ch = getopt_long_only(argc, argv, ":", longops, 0)) != -1)
    {
        switch (ch)
        {
        case 0: // help
            output_help();
            break;
        case 1: // attach
        case 2: // attach-no-hash
        {
            if (!check_path(optarg))
            {
                printf("the data_dir path could not contain . or ..\ndata_dir inputed: %s\n", optarg);
                return 1;
            }
            bool starts_with_slash = string_starts_with(string_replace(optarg, "/", "\\"), "\\");
            string data_dir = workdir + (starts_with_slash ? "" : "\\") + optarg;
            if (!file_exists(data_dir))
            {
                printf("the data_dir could not be found: %s\n", data_dir.c_str());
                return 1;
            }
            if(!is_file_a_dir(data_dir)) 
            {
                printf("the data_dir was not a directory: %s\n", data_dir.c_str());
                return 1;
            }
            printf("attach data_dir: %s\n", optarg);
            string ex_file = get_dir_name(executable) + "\\" + get_filename(executable) + "-attached.exe";
            string temp_compressed_dir = get_dir_name(executable) + "\\" + get_filename(executable) + "-compressed";
            bool check_hash = ch == 1;

            // 清理上次的临时文件
            if (file_exists(temp_compressed_dir))
                remove_dir(temp_compressed_dir);

            attach_binaries(executable, ex_file, data_dir, temp_compressed_dir, check_hash);
            printf("\nfinish\n");
            break;
        }
        case 3: // detach
        {
            printf("detach data\n");
#if defined(RELEASE_MODE)
            string source = executable;
#else
            string source = workdir + "\\" + get_filename(executable) + "-attached.exe";
            printf("executable file: %s\n", source.c_str());
#endif
            string decompressed_dir = workdir + "\\" + get_filename(executable) + "-decompressed";

            // 清理上次的临时文件
            if (file_exists(decompressed_dir))
                remove_dir(decompressed_dir);

            int r = extract_binaries(source, decompressed_dir);

            switch (r)
            {
            case 1:
                show_dialog(PROJECT_NAME, "程序损坏，无法读取标识数据(MagicHeader)");
                return 1;
            case 2:
                show_dialog(PROJECT_NAME, "应用程序内没有任何打包数据");
                return 1;
            case 3:
                show_dialog(PROJECT_NAME, "程序损坏，无法读取对应的数据");
                return 1;
            case 4:
                show_dialog(PROJECT_NAME, "程序损坏，无法读取对应的数据(Jumpdata)");
                return 1;
            case 5:
                show_dialog(PROJECT_NAME, "程序损坏，无法读取对应的数据(Metadata)");
                return 1;
            }

            break;
        }
        case 4: // detail
        {
            printf("detail\n");
#if defined(RELEASE_MODE)
            string source = executable;
#else
            string source = workdir + "\\" + get_filename(executable) + "-attached.exe";
            printf("executable file: %s\n", source.c_str());
#endif
            detail_binaries(source);
            break;
        }
        case 5: // export
        {
            printf("export\n");
            string ex_file = get_dir_name(executable) + "\\" + get_filename(executable) + "-export.json";
#if defined(RELEASE_MODE)
            string source = executable;
#else
            string source = workdir + "\\" + get_filename(executable) + "-attached.exe";
            printf("executable file: %s\n", source.c_str());
#endif
            detail_binaries(source, ex_file);
            break;
        }
        case ':': // 缺失选项参数
            printf("require option arg: %s\n", argv[optind - 1]);
            output_help();
            return 1;
        case '?':  // 无效选项
            printf("invaild option: %s\n", argv[optind - 1]);
            output_help();
            return 1;
        }
    }

    if (argc == 1)
    {
#if defined(RELEASE_MODE)
        string source = executable;
        string decompressed_dir = get_temp_directory();
        //string decompressed_dir = workdir + "\\test";
        if (decompressed_dir == "")
            decompressed_dir = workdir + "\\" + get_filename(executable) + "-exec-temp";
#else
        string source = workdir + "\\" + get_filename(executable) + "-attached.exe";
        string decompressed_dir = workdir + "\\" + get_filename(executable) + "-exec-temp";
#endif
        printf("execute\n");
        run_program(source, decompressed_dir);
    }
    
    return 0;
}
