#include "args_utils.h"
#include "wingetopt-0.95/getopt.h"

app_arguments parse_args(int argc, char** argv)
{
    app_arguments result;

    result.argc = argc;
    result.argv = argv;

    option longops[] = {
        {"help",            no_argument,         0,  'h'},

        {"input",           required_argument,   0,  'i'},
        {"output",          required_argument,   0,  'o'},
        {"no-hashing",      no_argument,         0,  'n'},
        {"exec",            required_argument,   0,  'c'},

        {"pack",            required_argument,   0,  'p'},
        {"extract",         optional_argument,   0,  'e'},
        {"detail",          no_argument,         0,  'd'},

        {"show-console",    no_argument,         0,  's'},
        {"hide-console",    no_argument,         0,  'x'},
        {0, 0, 0, 0}
    };

    int ch;
    while ((ch = getopt_long(argc, argv, ":hi:o:nc:p:e::dsx", longops, nullptr)) != -1)
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
            result.pack_exec = optarg;
            break;
        case 'p': // pack
            result.pack = true;
            result.pack_src = optarg;
            break;
        case 'e': // extract
            result.extract = true;
            //printf("<<< %s >>>\n", optarg?"not null":"null");
            if(optarg)
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
        case ':': // 缺失选项参数
            result.optarg_required = true;
            result.invaild_opt_name = argv[optind - 1];
            break;
        case '?':  // 无效选项
            result.unknown_opt = true;
            result.invaild_opt_name = argv[optind - 1];
        }
    }

    return result;
}