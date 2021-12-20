#include "little_wrapper_app.h"
#include "project.h"
#include "exceptions/exceptions.h"
#include "utils/env_utils.h"
#include "utils/general_utils.h"
#include "archiver.h"
#include "runner.h"
#include <iostream>
#include "debug.h"

using namespace std;

void ps()
{
    printf("asfsaf\n");
    throw lw_base_exception("PX");
}

int little_wrapper_app::main(int argc, char** argv)
{
    try {

        ps();

        app_args args = parse_app_args(argc, argv);
        string work_dir = get_current_work_dir();
        string module_path = get_exe_path();
        return little_wrapper_entrance(args, work_dir, module_path);
    }
    catch (lw_base_exception& e) {
        printf("%s\n", e.what());
        exception_thrown(e);
        //show_dialog(PROJECT_NAME " " VERSION_TEXT " Exception occured", e.what());
    }
    catch (exception& e) {
        printf("Unknown Exception: %s.\n", e.what());
        error_check(false, e.what());
        //show_dialog(PROJECT_NAME " " VERSION_TEXT " Exception occured", e.what());
    }
    catch (...) {
        printf("Unknown Error occurred");
        error_check(false, "Unknown Error occurred");
        //show_dialog(PROJECT_NAME " " VERSION_TEXT " Exception occured", "Unknown Error occurred");
    }

    return 1;
}

int little_wrapper_app::little_wrapper_entrance(app_args args, string workdir, string executable)
{
    printf("suppress-output: %d\n", args.suppress_output);
    printf("show-console: %d\n", args.show_console);

    if (args.help) {
        output_help_text();
    } else if (args.optarg_required) {
        printf("require option for arg: %s\n", args.invaild_opt_name.c_str());
        output_help_text();
    } else if (args.unknown_opt) {
        printf("invaild option: %s\n", args.invaild_opt_name.c_str());
        output_help_text();
    } else if (args.pack) {
        subcommand_pack(args, workdir);
    } else if (args.extract) {
        subcommand_extract(args, workdir);
    } else if (args.detail) {
        subcommand_detail();
    } else {
        return run_program(get_exe_path(), args.start_parameters, args.show_console != args.hide_console, args.show_console, args.suppress_output);
    }

    return 0;
}

void little_wrapper_app::output_help_text()
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
    help_messge += "  --parameter-pass  or  -a                        - pass strings as startup parameters to the program bundled inside.\n";

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

    printf("<<< %s\n", args.pack_exec.c_str());

    printf("pack with source_dir: %s\n", source_dir.c_str());
    string output_file = args.output != "" ? (is_relative_path(args.output) ? workdir + "\\" : "") + args.output : workdir + "\\" + get_filename(source_dir) + ".exe";
    string temp_dir = "temp-compressed";
    printf("output: %s\n", output_file.c_str());

    archiver::lw_options options;
    options.check_hash = !args.pack_no_hash;
    options.exec = args.pack_exec;
    options.show_console = args.show_console == args.hide_console ? true : args.show_console;
    archiver arch(get_exe_path(), archiver::read_only);
    arch.lw_pack(source_dir, temp_dir, options);

    // 清理临时文件
    if (file_exists(temp_dir))
        remove_dir(temp_dir);

    printf("\nfinish\n");
}

void little_wrapper_app::subcommand_extract(app_args args, string workdir)
{
    string dest = args.output != "" ? args.output : args.extract_dest;
    string output_dir = dest != "" ? (is_relative_path(dest) ? workdir + "\\" : "") + dest : workdir + "\\" + get_filename(get_exe_path());

    // 清理已有文件
    if (file_exists(output_dir))
        remove_dir(output_dir);

    archiver arch(get_exe_path(), archiver::write_only);
    arch.lw_extract(output_dir, false, false);
}

void little_wrapper_app::subcommand_detail()
{
    archiver arch(get_exe_path(), archiver::read_only);
    arch.lw_detail();
}