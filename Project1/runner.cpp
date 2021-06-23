#include "runner.h"
#include "iostream"
#include "fstream"
#include "wrapper.h"
#include "utils.h"
#include "debug.h"
#include "env.h"
#include "windows.h"

using namespace std;

static void execute_shell(string temp_dir, string start_script)
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
		show_dialog(PROJECT_NAME, "主脚本_start.txt内没有任何内容");
		return;
	}
	printf("%s\n", command.c_str());
	//command = "cd /D \"" + temp_dir + "\" && " + command;
	command = temp_dir + "\\" + command;

	printf("exec: %s\nwork dir: %s\n", command.c_str(), workdir.c_str());

	system(command.c_str());

	printf("child process exited.");
}

static void start_child_process(string temp_dir, string start_script)
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
		show_dialog(PROJECT_NAME, "主脚本_start.txt内没有任何内容");
		set_window_visible(true);
		return;
	}
	//command = "cd /D \"" + temp_dir + "\" && " + command;

	printf("exec: %s\nwork dir: %s\n", command.c_str(), workdir.c_str());

	changed_current_work_dir(temp_dir);

	// Start the child process. 
	bool success = CreateProcessA(nullptr,   // No module name (use command line)
		(LPSTR)command.c_str(),               // Command line
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
		show_dialog("主程序执行失败", "exec: " + command + "\nwork dir: " + workdir + "\nerr: " + last_error);
		return;
	}

	printf("----------\n");

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	printf("----------\n");

	printf("child process exited.\n");
}

void run_program(string file, string temp_dir)
{
	int r = extract_binaries(file, temp_dir);
	switch (r)
	{
	case 1:
		show_dialog(PROJECT_NAME, "程序损坏，无法读取标识数据(MagicHeader)");
		return;
	case 2:
		show_dialog(PROJECT_NAME, "应用程序内没有任何打包数据");
		return;
	case 3:
		show_dialog(PROJECT_NAME, "程序损坏，无法读取对应的数据");
		return;
	case 4:
		show_dialog(PROJECT_NAME, "程序损坏，无法读取对应的数据(Jumpdata)");
		return;
	case 5:
		show_dialog(PROJECT_NAME, "程序损坏，无法读取对应的数据(Metadata)");
		return;
	}

	string main_script = temp_dir + "\\_start.txt";
	if (!file_exists(main_script))
	{
		show_dialog(PROJECT_NAME, "找不到主脚本: _start.txt");
		return;
	}
	if (is_file_a_dir(main_script))
	{
		show_dialog(PROJECT_NAME, "_start.txt不是一个文件");
		return;
	}
	
	set_window_visible(false);
	printf("temp dir: %s\n", temp_dir.c_str());

	// execute_shell(temp_dir, main_script);
	start_child_process(temp_dir, main_script);
	set_window_visible(true);
}