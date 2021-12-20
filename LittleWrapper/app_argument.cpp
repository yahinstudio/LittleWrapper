#include "app_argument.h"
#include "libs/wingetopt-0.95/getopt.h"
#include <string>
#include <vector>
#include "utils/env_utils.h"
#include "utils/general_utils.h"

using namespace std;

app_args parse_app_args(int argc, char** argv)
{
    app_args result;

    result.argc = argc;
    result.argv = argv;

    option longops[] = {
        {"help",            no_argument,         0,  'h'},

        {"input",           required_argument,   0,  'i'},
        {"output",          required_argument,   0,  'o'},
        {"no-hashing",      no_argument,         0,  'n'},
        {"exec",            optional_argument,   0,  'c'},

        {"pack",            required_argument,   0,  'p'},
        {"extract",         optional_argument,   0,  'e'},
        {"detail",          no_argument,         0,  'd'},

        {"show-console",    no_argument,         0,  's'},
        {"hide-console",    no_argument,         0,  'x'},
        {"suppress-output", no_argument,         0,  'u'},
        {"parameter-pass",  required_argument,   0,  'a'},
        {0, 0, 0, 0}
    };

    int ch;
    while ((ch = getopt_long(argc, argv, ":hi:o:nc::p:e::dsxua:", longops, nullptr)) != -1)
    {
        switch (ch)
        {
        case 'h': // help
            result.help = true;
            break;
        case 'i': // input
            result.input = optarg;
            break;
        case 'o': // output
            result.output = optarg;
            break;
        case 'n': // no-hashing
            result.pack_no_hash = true;
            break;
        case 'c': // command
            if (optarg)
                result.pack_exec = optarg;
            break;
        case 'p': // pack
            result.pack = true;
            result.pack_src = optarg;
            break;
        case 'e': // extract
            result.extract = true;
            if (optarg)
                result.extract_dest = optarg;
            break;
        case 'd': // detail
            result.detail = true;
            break;
        case 's': // show-console
            result.show_console = true;
            break;
        case 'x': // hide-console
            result.hide_console = true;
            break;
        case 'u': // suppress-output
            result.suppress_output = true;
            break;
        case 'a': // parameter-pass
            if (optarg)
                result.start_parameters = optarg;
            break;
        case ':': // 缺失选项参数
            result.optarg_required = true;
            result.invaild_opt_name = argv[optind - 1];
            break;
        case '?':  // 无效选项
            result.unknown_opt = true;
            result.invaild_opt_name = argv[optind - 1];
        }
    }

    // 从环境变量读取exec
    if (result.pack && result.pack_exec.empty())
    {
        vector<string> envs = get_environments();
        for (int i = 0; i < envs.size(); i++)
        {
            string env = envs[i];

            if (string_starts_with(env, "_lw_exec="))
            {
                result.pack_exec = env.substr(string("_lw_exec=").length());
                printf("read exec from environment variable: %s\n", result.pack_exec.c_str());
                break;
            }
        }
    }

    return result;
}
