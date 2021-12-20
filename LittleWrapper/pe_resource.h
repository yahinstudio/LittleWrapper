#pragma once
#include <string>
#include <windows.h>
#include "exceptions/exceptions.h"

class pe_resource_writer
{
public:
	std::string file;
	std::shared_ptr<void> handle;

	pe_resource_writer(std::string file);

	void remove_resource(std::string type, std::string name);

	void update_resouce(std::string type, std::string name, LPVOID binary, DWORD binary_size);

	void close();

public:
	class pe_resource_writer_exception : public lw_base_exception
	{
	public:
		pe_resource_writer_exception(std::string reason)
			: lw_base_exception(reason)
		{ }
	};
};

class pe_resource_reader
{
public:
	struct res_handle
	{
		LPVOID data;
		DWORD size;
	};

public:
	std::string file;
	std::shared_ptr<HINSTANCE__> module;

	pe_resource_reader(std::string file = "");

	res_handle open_resource(int type, int name);

	res_handle open_resource(std::string type, int name);

	res_handle open_resource(int type, std::string name);

	res_handle open_resource(std::string type, std::string name);

	void close();

private:
	pe_resource_reader::res_handle _open_resource(HRSRC resource, std::string resource_type_name);

public:
	class pe_resource_reader_exception : public lw_base_exception
	{
	public:
		pe_resource_reader_exception(std::string reason)
			: lw_base_exception(reason)
		{ }
	};

	class pe_resource_not_found_exception : public lw_base_exception
	{
	public:
		pe_resource_not_found_exception(std::string reason)
			: lw_base_exception(reason)
		{ }
	};
};