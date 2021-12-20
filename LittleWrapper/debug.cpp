#include "debug.h"
#include <string>
#include "utils/general_utils.h"
#include <windows.h>
#include "project.h"
#include "exceptions/exceptions.h"
#include "utils/env_utils.h"

using namespace std;

static bool has_pdb_file()
{
	return file_exists(get_dir_name(get_exe_path()) + "\\" + get_exe_filename(true) + ".pdb");
}

int _error_check(int expression, std::string err_msg, const char* file, int line, const char* function, const char* data, const char* time)
{
	if (expression == 0)
	{
		char* buf = new char[DEBUG_MAX_ERROR_MESSAGE_LEN];
		string last_err_msg = get_last_error_message();

		string err_msg_template =
			"\nexpression-value: %d\n"
			"file: %s on %d\n"
			"function: %s\n"
			"compile-time: %s %s\n"
			"cause: %s\n"
			"probable reason: %s(%d)\n";

		if (has_pdb_file())
		{
			err_msg_template +=
				"___________________________________traceback______________________________"
				"\n%s";

			string st = StackTraceback().to_string();

			sprintf_s(buf, DEBUG_MAX_ERROR_MESSAGE_LEN, err_msg_template.c_str(),
				expression,
				file,
				line,
				function,
				data, time,
				err_msg.c_str(),
				last_err_msg.c_str(), GetLastError(),

				st.c_str()
			);
		} else {
			sprintf_s(buf, DEBUG_MAX_ERROR_MESSAGE_LEN, err_msg_template.c_str(),
				expression,
				file,
				line,
				function,
				data, time,
				err_msg.c_str(),
				last_err_msg.c_str(), GetLastError()
			);
		}

		printf("%s\n", buf);

#ifdef ENABLED_ERROR_DIALOG
		string title = string(PROJECT_NAME) + " " + get_application_version() + " Error occurred";
		MessageBoxA(nullptr, buf, title.c_str(), MB_ICONERROR | MB_OK);
#endif
		delete[] buf;
		exit(1);
	}

	return expression;
}

void exception_thrown(exception& ex)
{
	char* buf = new char[DEBUG_MAX_ERROR_MESSAGE_LEN];
	string last_err_msg = get_last_error_message();

	string err_msg_template =
		"exception thrown: %s\n"
		"compile time: %s %s\n"
		"message: %s\n"
		"probable reason: %s(%d)\n";

	if (has_pdb_file())
	{
		err_msg_template += 
			"___________________________________traceback______________________________"
			"\n%s";

		string st = StackTraceback(ex).to_string();

		sprintf_s(buf, DEBUG_MAX_ERROR_MESSAGE_LEN, err_msg_template.c_str(),
			typeid(ex).name(),
			__DATE__, __TIME__,
			ex.what(),
			last_err_msg.c_str(), GetLastError(),

			st.c_str()
		);
	} else {
		sprintf_s(buf, DEBUG_MAX_ERROR_MESSAGE_LEN, err_msg_template.c_str(),
			typeid(ex).name(),
			__DATE__, __TIME__,
			ex.what(),
			last_err_msg.c_str(), GetLastError()
		);
	}

	printf("%s\n", buf);

#ifdef ENABLED_ERROR_DIALOG
	string title = string(PROJECT_NAME) + " " + get_application_version() + " Error occurred";
	MessageBoxA(nullptr, buf, title.c_str(), MB_ICONERROR | MB_OK);
#endif
	
	delete[] buf;
	exit(1);
}

void unknown_exception_thrown()
{
	char* buf = new char[DEBUG_MAX_ERROR_MESSAGE_LEN];
	string last_err_msg = get_last_error_message();

	string err_msg_template =
		"compile time: %s %s\n"
		"probable reason: %s(%d)\n";

	if (has_pdb_file())
	{
		err_msg_template += 
			"___________________________________traceback______________________________"
			"\n%s";

		string st = StackTraceback(StackWalker::AfterCatch).to_string();

		sprintf_s(buf, DEBUG_MAX_ERROR_MESSAGE_LEN, err_msg_template.c_str(),
			__DATE__, __TIME__,
			last_err_msg.c_str(), GetLastError(),

			st.c_str()
		);
	} else {
		sprintf_s(buf, DEBUG_MAX_ERROR_MESSAGE_LEN, err_msg_template.c_str(),
			__DATE__, __TIME__,
			last_err_msg.c_str(), GetLastError()
		);
	}

	printf("%s\n", buf);

#ifdef ENABLED_ERROR_DIALOG
	string title = string(PROJECT_NAME) + " " + get_application_version() + " Error occurred";
	MessageBoxA(nullptr, buf, title.c_str(), MB_ICONERROR | MB_OK);
#endif

	delete[] buf;
	exit(1);
}

string get_last_error_message()
{
	char* msg_buf;
	DWORD err_code = GetLastError();

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		err_code,
		0,
		(LPSTR)&msg_buf,
		0,
		0
	);

	string result = string(msg_buf);
	LocalFree(msg_buf);
	return string_replace(result, "\r\n", "");
}
