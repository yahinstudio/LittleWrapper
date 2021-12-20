#pragma once
#include <string>
#include "vcruntime_exception.h"
#include "traceback.h"
#include "utils/general_utils.h"

class lw_base_exception : public std::exception
{
public:
	std::string reason;
	std::vector<traceback::stack_frame> stack_frames;

	inline lw_base_exception(std::string reason) : exception()
	{
		this->reason = reason;
		stack_frames = traceback::stack_trace();
	}

	virtual const char* what() const
	{
		return this->reason.c_str();
	}

	const std::string traceback()
	{
		std::string result;

		char* buf = new char[1024];
		for (traceback::stack_frame sf : stack_frames)
		{
			memset(buf, 0, 1024);
			sprintf(buf, "0x%lld : %s : %s on %ld (%s)\n\n", sf.address, sf.name.c_str(), sf.file.c_str(), sf.line, sf.module.c_str());
			result += buf;
		}
		delete[] buf;

		return result;
	}
};

class jumpdata_not_found_exception : public lw_base_exception 
{
public:
	jumpdata_not_found_exception() 
		: lw_base_exception("The Magic Header could not be located")
	{ }
};

class jumpdata_invalid_exception : public lw_base_exception 
{ 
public:
	jumpdata_invalid_exception()
		: lw_base_exception("The Jumpdata was corrupted")
	{ }
};

class metadata_not_found_exception : public lw_base_exception 
{
public:
	metadata_not_found_exception()
		: lw_base_exception("The Executable did not contain any Binaries")
	{ }
};

class metadata_invalid_exception : public lw_base_exception 
{
public:
	metadata_invalid_exception()
		: lw_base_exception("The Metadata was corrupted")
	{ }
};

class binaries_damaged_exception : public lw_base_exception
{
public:
	binaries_damaged_exception()
		: lw_base_exception("The Binaries inside this executable were corrupted")
	{ }
};

class source_dir_not_found_exception : public lw_base_exception
{
public:
	source_dir_not_found_exception(std::string dir_path)
		: lw_base_exception("The source_dir not found or not a directory: " + dir_path)
	{ }
};

class app_argument_required_exception : public lw_base_exception
{
public:
	app_argument_required_exception(std::string argument_required)
		: lw_base_exception("the argument " + argument_required + " was required")
	{ }
};

