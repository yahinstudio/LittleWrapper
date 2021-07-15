#include "runner.h"
#include "iostream"
#include "fstream"
#include "wrapper.h"
#include "utils.h"
#include "debug.h"
#include "env.h"
#include "windows.h"

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

static int start_child_process(string temp_dir, string exec)
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
	bool success = CreateProcessA(nullptr,   // No module name (use command line)
		(LPSTR)exec.c_str(),               // Command line
		NULL,              // Process handle not inheritable
		NULL,              // Thread handle not inheritable
		FALSE,             // Set handle inheritance to FALSE
		0,                 // No creation flags
		NULL,              // Use parent's environment block
		workdir.c_str(),   // Use parent's work directory 
		&si,               // Pointer to STARTUPINFO structure
		&pi                // Pointer to PROCESS_INFORMATION structure
	);

	if (!success)
	{
		string last_error = get_last_error_message();
		printf("CreateProcess failed: %s(%d).\n", last_error.c_str(), GetLastError());
		show_dialog("主程序执行失败", "exec: " + exec + "\nwork dir: " + workdir + "\nerr: " + last_error);
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

int run_program(string file, string temp_dir, bool always_show_console)
{
	if(!always_show_console)
		set_window_visible(false);

	string exec = "";
	switch (extract_binaries(file, temp_dir, &exec))
	{
	case 1:
		if (!always_show_console)
		set_window_visible(true);
		show_dialog(PROJ_VER, "程序损坏，无法读取标识数据(MagicHeader)");
		return 1;
	case 2:
		if (!always_show_console)
		set_window_visible(true);
		show_dialog(PROJ_VER, "应用程序内没有任何打包数据");
		return 1;
	case 3:
		if (!always_show_console)
		set_window_visible(true);
		show_dialog(PROJ_VER, "程序损坏，无法读取对应的数据");
		return 1;
	case 4:
		if (!always_show_console)
		set_window_visible(true);
		show_dialog(PROJ_VER, "程序损坏，无法读取对应的数据(Jumpdata)");
		return 1;
	case 5:
		if (!always_show_console)
		set_window_visible(true);
		show_dialog(PROJ_VER, "程序损坏，无法读取对应的数据(Metadata)");
		return 1;
	}

	printf("temp dir: %s\n", temp_dir.c_str());

	int rt = start_child_process(temp_dir, exec);
	set_window_visible(true);
	return rt;
}