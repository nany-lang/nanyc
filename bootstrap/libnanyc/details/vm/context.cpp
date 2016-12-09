#include "details/vm/context.h"
#include "details/vm/context-runner.h"
#include <yuni/core/system/environment.h>
#include <yuni/io/directory/system.h>
#include <yuni/core/string.h>
#include <yuni/core/system/windows.hdr.h>
#include "details/vm/console.h"

using namespace Yuni;


namespace ny {
namespace vm {


Context::Context(Program& program, const AnyString& name)
	: program(program)
	, cf(program.cf)
	, name(name) {
}


Context::Context(Context& rhs)
	: program(rhs.program)
	, cf(rhs.cf) {
	memcpy(&io.fallback.adapter, &rhs.io.fallback.adapter, sizeof(nyio_adapter_t));
	io.cwd = rhs.io.cwd;
	io.mountpointSize = rhs.io.mountpointSize;
	for (uint32_t i = 0; i != io.mountpointSize; ++i) {
		auto& original = rhs.io.mountpoints[i];
		auto& mountpoint = io.mountpoints[i];
		mountpoint.path = original.path;
		if (original.adapter.clone)
			original.adapter.clone(&original.adapter, &mountpoint.adapter);
		else
			memcpy(&mountpoint.adapter, &original.adapter, sizeof(nyio_adapter_t));
	}
}


Context::~Context() {
	for (uint32_t i = 0; i != io.mountpointSize; ++i) {
		auto& mountpoint = io.mountpoints[i];
		if (mountpoint.adapter.release)
			mountpoint.adapter.release(&mountpoint.adapter);
	}
}


bool Context::IO::addMountpoint(const AnyString& path, nyio_adapter_t& adapter) {
	if (mountpointSize < mountpoints.max_size() and path.size() < ShortString256::chunkSize) {
		// inserting the new mountpoint at the begining
		if (mountpointSize++ != 0) {
			uint32_t i = mountpointSize;
			while (i --)
				mountpoints[i + 1] = mountpoints[i];
		}
		auto& mp = mountpoints[0];
		mp.path = path;
		mp.path.trimRight('/');
		if (unlikely(mp.path.empty()))
			mp.path << '/';
		memcpy(&mp.adapter, &adapter, sizeof(nyio_adapter_t));
		// reset the input adapter to prevent it from being used
		memset(&adapter, 0x0, sizeof(nyio_adapter_t));
		return true;
	}
	return false;
}


bool Context::initializeFirstTContext() {
	// default current working directory
	io.cwd = "/home";
	// fallback filesystem
	io.fallback.path.clear(); // just in case
	initFallbackAdapter(io.fallback.adapter);
	// reset mountpoints
	io.mountpointSize = 0;
	String path;
	// mount home folder
	{
		bool r = Yuni::IO::Directory::System::UserHome(path) and path.size() < ShortString256::chunkSize;
		if (unlikely(not r))
			return false;
		auto& mp = io.mountpoints[io.mountpointSize++];
		mp.path = "/home";
		nyio_adapter_create_from_local_folder(&mp.adapter, &cf.allocator, path.c_str(), path.size());
	}
	// mount tmp folder
	{
		bool r = Yuni::IO::Directory::System::Temporary(path) and path.size() < ShortString256::chunkSize;
		if (unlikely(not r))
			return false;
		auto& mp = io.mountpoints[io.mountpointSize++];
		mp.path = "/tmp";
		nyio_adapter_create_from_local_folder(&mp.adapter, &cf.allocator, path.c_str(), path.size());
	}
	// mount '/' -> '/root'
	{
		auto& mp = io.mountpoints[io.mountpointSize++];
		mp.path = "/root";
		nyio_adapter_create_from_local_folder(&mp.adapter, &cf.allocator, nullptr, 0u);
	}
	return true;
}


nyio_adapter_t& Context::IO::resolve(AnyString& adapterpath, const AnyString& path) {
	// /some/root/folder[/some/adapter/folder]
	//                 ^                     ^
	//  mppath/msize --|        path/psize --|
	uint32_t psize = path.size();
	uint32_t count = mountpointSize;
	for (uint32_t i = 0; i != count; ++i) {
		auto& mountpoint = mountpoints[i];
		const auto& mppath = mountpoint.path;
		const uint32_t msize = mppath.size();
		// if the request path size is strictly greater than the mountpoint path,
		// then this mountpoint is certainly not the solution
		if (psize < msize)
			continue;
		// not the root folder if the size do not match (so '/' is required)
		if (msize != psize) {
			// good to go if startsWith...
			if (path[msize] == '/' and 0 == memcmp(mppath.c_str(), path.c_str(), msize)) {
				adapterpath.adapt(path.c_str() + msize, psize - msize);
				return mountpoint.adapter;
			}
		}
		else {
			// root folder, must be strictly identical
			if (0 == memcmp(mppath.c_str(), path.c_str(), msize)) {
				adapterpath.adapt("/", 1u);
				return mountpoint.adapter;
			}
		}
	}
	adapterpath = path;
	return fallback.adapter;
}


bool Context::invoke(uint64_t& exitstatus, const ir::Sequence& callee, uint32_t atomid,
		uint32_t instanceid) {
	exitstatus = static_cast<uint64_t>(-1);
	if (!cf.on_thread_create
		or nyfalse != cf.on_thread_create(program.self(), self(), nullptr, name.c_str(), name.size())) {
		ContextRunner runner{*this, callee};
		try {
			runner.initialize();
			runner.stacktrace.push(atomid, instanceid);
			if (setjmp(runner.jump_buffer) != 666) {
				runner.invoke(callee);
				exitstatus = runner.retRegister;
				if (cf.on_thread_destroy)
					cf.on_thread_destroy(program.self(), self());
				return true;
			}
			else {
				// execution of the program failed
			}
		}
		catch (const std::bad_alloc&) {
			ny::vm::console::badAlloc(*this);
		}
		catch (const std::exception& e) {
			ny::vm::console::exception(*this, e.what());
		}
		if (cf.on_thread_destroy)
			cf.on_thread_destroy(program.self(), self());
	}
	return false;
}


} // namespace vm
} // namespace ny
