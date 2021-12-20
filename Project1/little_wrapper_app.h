#pragma once
#include "string"
#include "app_argument.h"

class little_wrapper_app
{
public:
	int main(int argc, char** argv);
	int little_wrapper_entrance(app_args args, std::string workdir, std::string executable);
	void output_help_text();
	int run_program(std::string executable, std::string parameter_additional, bool show_console_set, bool show_console, bool no_output);

	void subcommand_pack(app_args args, std::string workdir);
	void subcommand_extract(app_args args, std::string workdir);
	void subcommand_detail();
};

