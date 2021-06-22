#include "debug.h"
#include "string"
#include "utils.h"
#include "assert.h" // assert()
#include "windows.h"

using namespace std;

void error_check(bool returnvalue)
{
	assert(returnvalue);
}

string get_last_error_message()
{
	string result;

	char* msg_buf;
	uint64_t err_code = GetLastError();

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		err_code,
		0,
		(LPSTR)&msg_buf,
		0,
		0
	);

	result = msg_buf;
	free(msg_buf);

	return string_replace(result, "\r\n", "");
}