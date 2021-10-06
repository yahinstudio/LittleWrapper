#include "runner.h"
#include "iostream"
#include "fstream"
#include "wrapper.h"
#include "utils.h"
#include "debug.h"
#include "env.h"
#include "windows.h"
#include "vector"

using namespace std;

/*
static void execute_shell(string temp_dir, string exec)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	string workdir = get_current_work_dir();

	std::fstream ms(start_script, fstream::in | fstream::binary);
	error_check(!ms.fail(), "execute_shell: could not open the start-script file: " + start_script);

	ms.seekg(0, ms.end);
	int file_len = ms.tellg();
	ms.seekg(0, ms.beg);

	char* cmd = new char[file_len + 1];
	memset(cmd, 0, file_len + 1);
	ms.read(cmd, file_len);
	error_check(!ms.bad(), "execute_shell: could not read from the start-script file: " + start_script);
	ms.close();

	string command = cmd;
	delete[] cmd;

	if (command.length() == 0)
	{
		printf("the main command could not be empty!\n");
		show_dialog(PROJ_VER, "主脚本_start.txt内没有任何内容");
		return;
	}
	printf("%s\n", command.c_str());
	//command = "cd /D \"" + temp_dir + "\" && " + command;
	command = temp_dir + "\\" + command;

	printf("exec: %s\nwork dir: %s\n", command.c_str(), workdir.c_str());

	system(command.c_str());

	printf("child process exited.");
}
*/

static int start_child_process(string temp_dir, string exec, char* env, bool no_output)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	string workdir = get_current_work_dir();

	printf("exec: %s\nwork dir: %s\n", exec.c_str(), workdir.c_str());

	changed_current_work_dir(temp_dir);

	// Start the child process. 
	bool success = CreateProcessA(nullptr,      // No module name (use command line)
		(LPSTR)exec.c_str(),                    // Command line
		NULL,                 // Process handle not inheritable
		NULL,                 // Thread handle not inheritable
		FALSE,                // Set handle inheritance to FALSE
		0,                    // No creation flagsd
		(LPVOID)env,          // Use parent's environment block
		workdir.c_str(),      // Use parent's work directory 
		&si,                  // Pointer to STARTUPINFO structure
		&pi                   // Pointer to PROCESS_INFORMATION structure
	);

	if (!success)
	{
		string last_error = get_last_error_message();
		printf("CreateProcess failed: %s(%d).\n", last_error.c_str(), GetLastError());
		show_dialog("主程序执行失败", "exec: " + exec + "\nwork dir: " + workdir + "\nenv: " + env + "\nerr: " + last_error);
		return 1;
	}

	printf("----------\n");

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Get the Exitcode of the subprocess
	unsigned long exitcode = 0;
	error_check(GetExitCodeProcess(pi.hProcess, &exitcode), "execute_shell: get the exitcode failed");

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	printf("----------\n");

	printf("child process exited with exitcode %ld.\n", exitcode);
	return (int)exitcode;
}

static char* get_environments_of_subprocess(string temp_dir)
{
	std::vector<string> ps;
	
	for (auto e : get_environments())
		ps.push_back(e);

	ps.push_back(string("_LW_EXEFILE=") + string_replace(get_exe_path(), "\\", "/"));
	ps.push_back(string("_LW_EXEDIR=") + string_replace(get_dir_name(get_exe_path()), "\\", "/"));
	ps.push_back(string("_LW_TEMPDIR=") + string_replace(temp_dir, "\\", "/"));
	ps.push_back(string("_LW_EXEFILE_=") + get_exe_path());
	ps.push_back(string("_LW_EXEDIR_=") + get_dir_name(get_exe_path()));
	ps.push_back(string("_LW_TEMPDIR_=") + temp_dir);

	ps.push_back(string("_LW_VERSION=") + VERSION_TEXT);
	ps.push_back(string("_LW_COMPILE_TIME=") + __DATE__ " " __TIME__);

	int envLen = 1;
	for (auto s : ps)
		envLen += s.length() + 1;
	char* env = new char[envLen];
	memset(env, 0, envLen);

	int offset = 0;
	for (auto s : ps)
	{
		int len = s.length();
		memcpy(env + offset, s.c_str(), s.length());
		offset += len + 1;
	}

	return env;
}

static void replace_variables(string& exec, string temp_dir)
{
	exec = string_replace(exec, "$_lw_tempdir_", temp_dir);
	exec = string_replace(exec, "$_lw_exedir_", get_dir_name(get_exe_path()));
	exec = string_replace(exec, "$_lw_exefile_", get_exe_path());

	exec = string_replace(exec, "$_lw_tempdir", string_replace(temp_dir, "\\", "/"));
	exec = string_replace(exec, "$_lw_exedir", string_replace(get_dir_name(get_exe_path()), "\\", "/"));
	exec = string_replace(exec, "$_lw_exefile", string_replace(get_exe_path(), "\\", "/"));
}

int run_program(string file, string temp_dir, std::string additional_argument, bool show_console_set, bool show_console, bool no_output)
{
	// 获取optiondata
	cJSON* meta;
	lw_read_metadata(file, &meta);
	optiondata optdata = get_optiondata(meta);

	bool console_visible = show_console_set ? show_console : optdata.show_console;

	if (!console_visible)
		set_window_visible(false);

	// 解压数据
	lw_extract(file, temp_dir, true, no_output);

	if(!console_visible)
		set_window_visible(false);

	printf("temp dir: %s\n", temp_dir.c_str());
	string exec = optdata.exec;
	replace_variables(exec, temp_dir);
	char* env = get_environments_of_subprocess(temp_dir);

	int rt = start_child_process(temp_dir, exec + additional_argument, env, no_output);

	delete env;

	if (!console_visible)
		set_window_visible(true);

	return rt;
}