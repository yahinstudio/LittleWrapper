#include "pe_resource.h"
#include "utils/env_utils.h"
#include <string>

using namespace std;

pe_resource_writer::pe_resource_writer(std::string file)
{
	this->file = file;

	if (file == get_exe_path())
		throw pe_resource_writer_exception("forbidden to open self executeable file in pe_resource_writer");

	HANDLE _handle = BeginUpdateResourceA(file.c_str(), false);

	if (_handle == nullptr)
		throw pe_resource_writer_exception("failed BeginUpdateResourceA() for file: " + file);
	
	handle.reset(_handle, [this](void* handle) {
		if (!EndUpdateResourceA(handle, false))
			throw pe_resource_writer_exception("failed to close the handle opended by EndUpdateResourceA() for module: " + this->file);
	});
}

void pe_resource_writer::remove_resource(std::string type, std::string name)
{
	if (!UpdateResourceA(handle.get(), type.c_str(), name.c_str(), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), nullptr, 0))
		throw pe_resource_writer_exception("failed to remove resource: type(" + type + "), name(" + name + ")");
}

void pe_resource_writer::update_resouce(std::string type, std::string name, LPVOID binary, DWORD binary_size)
{
	if (!UpdateResourceA(handle.get(), type.c_str(), name.c_str(), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), binary, binary_size))
		throw pe_resource_writer_exception("failed to update resource: type(" + type + "), name(" + name + "), length(" + std::to_string(binary_size) + ")");
}

void pe_resource_writer::close()
{
	handle.reset();
}

///////////////////////////////////////////////////

pe_resource_reader::pe_resource_reader(std::string file)
{
	this->file = file;

	HMODULE _module = file == get_exe_path() ? GetModuleHandleA(nullptr) : LoadLibraryA(file.c_str());

	if (_module == nullptr)
		throw pe_resource_reader_exception("failed LoadLibrary() for module: " + file);

	module.reset(_module, [this](HINSTANCE__* module) {
		if (module != GetModuleHandleA(nullptr))
			if (!FreeLibrary(module))
				throw pe_resource_reader_exception("failed FreeLibrary() for module: " + this->file);
	});
}

pe_resource_reader::res_handle pe_resource_reader::open_resource(int type, int name)
{
	HRSRC resource = FindResourceA(module.get(), MAKEINTRESOURCEA(name), MAKEINTRESOURCEA(type));
	string resource_type_name = "type(" + to_string(type) + "), name(" + to_string(name) + ")";

	return _open_resource(resource, resource_type_name);
}

pe_resource_reader::res_handle pe_resource_reader::open_resource(std::string type, int name)
{
	HRSRC resource = FindResourceA(module.get(), MAKEINTRESOURCEA(name), type.c_str());
	string resource_type_name = "type(" + type + "), name(" + to_string(name) + ")";

	return _open_resource(resource, resource_type_name);
}

pe_resource_reader::res_handle pe_resource_reader::open_resource(int type, std::string name)
{
	HRSRC resource = FindResourceA(module.get(), name.c_str(), MAKEINTRESOURCEA(type));
	string resource_type_name = "type(" + to_string(type )+ "), name(" + name + ")";

	return _open_resource(resource, resource_type_name);
}

pe_resource_reader::res_handle pe_resource_reader::open_resource(std::string type, std::string name)
{
	HRSRC resource = FindResourceA(module.get(), name.c_str(), type.c_str());
	string resource_type_name = "type(" + type + "), name(" + name + ")";

	return _open_resource(resource, resource_type_name);
}

pe_resource_reader::res_handle pe_resource_reader::_open_resource(HRSRC resource, string resource_type_name)
{
	if (resource == nullptr)
		throw pe_resource_not_found_exception("no resource [" + resource_type_name + "] found in module: " + file);

	HGLOBAL res_loaded = LoadResource(module.get(), resource);
	if (res_loaded == nullptr)
		throw pe_resource_reader_exception("failed LoadResource(" + resource_type_name + ") for module: " + file);

	LPVOID res_lock = LockResource(res_loaded);
	if (res_lock == nullptr)
		throw pe_resource_reader_exception("failed LockResource(" + resource_type_name + ") for a resource in module: " + file);

	res_handle reshandle;
	reshandle.data = res_lock;
	reshandle.size = SizeofResource(module.get(), resource);
	return reshandle;
}

void pe_resource_reader::close()
{
	module.reset();
}