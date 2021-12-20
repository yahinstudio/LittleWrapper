#pragma once
#include <string>
#include "vcruntime_exception.h"
#include "traceback.h"
#include "utils/general_utils.h"
#include "../libs/stack_walker/StackWalker.h"

class lw_base_exception : public std::exception
{
public:
	std::string message;

	inline lw_base_exception(std::string message) : exception()
	{
		this->message = message;
	}

	virtual const char* what() const override
	{
		return this->message.c_str();
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

class failed_to_pack_exception : public lw_base_exception
{
public:
	failed_to_pack_exception()
		: lw_base_exception("no any file packable")
	{ }
};

class failed_to_alloc_exception : public lw_base_exception
{
public:
	failed_to_alloc_exception(unsigned long size)
		: lw_base_exception("request heap memory failed with size: " + size)
	{ }
};