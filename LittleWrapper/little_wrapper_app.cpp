#include "little_wrapper_app.h"
#include "project.h"
#include "exceptions/exceptions.h"
#include "utils/env_utils.h"
#include "utils/general_utils.h"
#include "archiver.h"
#include "runner.h"
#include <iostream>
#include "debug.h"
#include "dump.h"
#include "pe_resource.h"

using namespace std;

int little_wrapper_app::main(int argc, char** argv)
{
    try {
        // 打印基本信息
        printf("%s\nargument: [", get_application_version());
        for (int i = 0; i < argc; i++)
            printf("%s%s", argv[i], i != argc - 1 ? ", " : "");
        printf("]\n");
                
        app_args args = parse_app_args(argc, argv);
        string work_dir = get_current_work_dir();
        string module_path = get_exe_path();

        printf("suppress-output: %s, show-console: %s\n", 
            args.suppress_output ? "true" : "false",
            args.show_console ? "true" : "false"
        );
        printf("\n\n");

        return little_wrapper_entrance(args, work_dir, module_path);
    }
    catch (pe_resource_reader::pe_resource_not_found_exception& ex)
    {
#ifdef ENABLED_ERROR_DIALOG
        show_dialog("No binaried packed", "This PE file is empty, no any binaries which has been packed in.");
#else
        throw ex;
#endif
    }
    catch (lw_base_exception& e) {
        printf("\n------------------------------------------\nlw_base_exception occurred\n");
        CreateDumpFile(get_exe_filename(true) + ".dmp");
        exception_thrown(e);
    }
    catch (exception& e) {
        printf("\n------------------------------------------\nException: occurred\n");
        CreateDumpFile(get_exe_filename(true) + ".dmp");
        exception_thrown(e);
    }
    catch (...) {
        printf("\n------------------------------------------\nUnknown Error occurred");
        CreateDumpFile(get_exe_filename(true) + ".dmp");
        unknown_exception_thrown();
    }

    return 1;
}

int little_wrapper_app::little_wrapper_entrance(app_args args, string workdir, string executable)
{
    if (args.help) {
        output_help_text();
    } else if (args.optarg_required) {
        printf("require option for arg: %s\n", args.invaild_opt_name.c_str());
        output_help_text();
    } else if (args.unknown_opt) {
        printf("invaild option: %s\n", args.invaild_opt_name.c_str());
        output_help_text();
    } else if (args.pack) {
        printf("packing\n");
        subcommand_pack(args, workdir);
    } else if (args.extract) {
        printf("extracting\n");
        subcommand_extract(args, workdir);
    } else if (args.detail) {
        printf("detailing\n");
        subcommand_detail();
    } else {
        printf("executing\n");
        return run_program(get_exe_path(), args.start_parameters, args.show_console != args.hide_console, args.show_console, args.suppress_output);
    }

    return 0;
}

void little_wrapper_app::output_help_text()
{
    /*
    可选参数的几种写法：
    1. --extract=[output_dir] (带参数，推荐写法)
    2. -e[output_dir] （带参数，简写法）
    3. --extract （不带参数）
    */
    string help_messge =
     "subcommands available:\n"
     "  --help                               - show this help infomation\n"
     "  --pack=<source_dir>                  - pack files.\n"
     "    --exec[=command]                       the option 'command'\n"
     "    [--output=<output_file>]               of --exec can be\n"
     "    [--no-hashing]                         displaced by env _lw_exec=\n"
     "  --extract[=output_dir]               - extract the binaries.\n"
     "  --detail / -d                        - detail the binaries.\n"
     "  --show-console /-s                   - run with console visible.\n"
     "                                           defaultly, prior to hide\n"
     "  --hide-console / -x                  - run with console invisible.\n"
     "  --suppress-output / -u               - suppress the output of decompresing.\n"
     "  --parameter-pass / -a                - pass startup arguments to the\n"
     "                                           program packed inside.";

    printf("%s", help_messge.c_str());
}

int little_wrapper_app::run_program(std::string executable, std::string parameter_additional, bool show_console_set, bool show_console, bool no_output)
{
    string temp_dir = get_temp_directory() + "LW-" + get_string_md5(executable).substr(0, 8);

    return start_subprocess(executable, temp_dir, parameter_additional, show_console_set, show_console, no_output);
}

void little_wrapper_app::subcommand_pack(app_args args, string workdir)
{
    string source_dir = string_replace(args.pack_src, "/", "\\");

    // convert to absolute path
    if (is_relative_path(source_dir))
        source_dir = workdir + (string_starts_with(source_dir, "\\") ? "" : "\\") + source_dir;

    // check whether exist and is not a directory
    if (!file_exists(source_dir) || !is_file_a_dir(source_dir))
        throw source_dir_not_found_exception(source_dir);

    // check existence of --exec
    if (args.pack_exec == "")
        throw app_argument_required_exception("--exec");

    printf("exec: %s\n", args.pack_exec.c_str());
    printf("source_dir: %s\n", source_dir.c_str());
    string output_file = args.output != "" ? (is_relative_path(args.output) ? workdir + "\\" : "") + args.output : workdir + "\\" + get_filename(source_dir) + ".exe";
    string temp_dir = "temp-compressed";
    printf("output: %s\n", output_file.c_str());

    archiver::lw_options options;
    options.check_hash = !args.pack_no_hash;
    options.exec = args.pack_exec;
    options.show_console = args.show_console == args.hide_console ? true : args.show_console;

    archiver::lw_pack(output_file, source_dir, temp_dir, options);

    // 清理临时文件
    if (file_exists(temp_dir))
        remove_dir(temp_dir);

    printf("\little wrapper end\n");
}

void little_wrapper_app::subcommand_extract(app_args args, string workdir)
{
    string dest = args.output != "" ? args.output : args.extract_dest;
    string output_dir = dest != "" ? (is_relative_path(dest) ? workdir + "\\" : "") + dest : workdir + "\\" + get_filename(get_exe_path());

    // 清理已有文件
    if (file_exists(output_dir))
        remove_dir(output_dir);

    archiver::lw_extract(get_exe_path(), output_dir, false, false);
}

void little_wrapper_app::subcommand_detail()
{
    archiver::lw_detail(get_exe_path());
}