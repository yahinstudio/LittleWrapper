#pragma once
#include <string>
#include "exceptions/exceptions.h"

#define DEBUG_MAX_ERROR_MESSAGE_LEN (8 * 1024)

#define error_check(expression, err_msg) _error_check(expression, err_msg, __FILE__, __LINE__,  __FUNCTION__, __DATE__, __TIME__)

int _error_check(int expression, std::string err_msg, const char* file, int line, const char* function, const char* data, const char* time);

void exception_thrown(std::exception& ex);

void unknown_exception_thrown();

std::string get_last_error_message();