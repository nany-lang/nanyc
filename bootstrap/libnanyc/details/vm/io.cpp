#include "details/vm/io.h"
#include "details/vm/thread.h"
#include <yuni/io/directory/system.h>
#include "libnanyc.h"
#include <cstring>


namespace ny {
namespace vm {

namespace {

void release(nyio_adapter_t& adapter) {
	if (adapter.release)
		adapter.release(&adapter);
}

} // namespace

Mountpoints::Mountpoints() {
	nyio_adapter_init_dummy(&m_fallback.adapter);
	yuni::String path;
	constexpr uint32_t maxPathSize = yuni::ShortString256::chunkSize;
	// mount home folder
	{
		bool r = yuni::IO::Directory::System::UserHome(path) and path.size() < maxPathSize;
		if (unlikely(not r))
			throw "failed to get home folder";
		auto& mp = m_mounts[m_count++];
		mp.path = "/home";
		nyio_adapter_init_localfolder(&mp.adapter, path.c_str(), path.size());
	}
	// mount tmp folder
	{
		bool r = yuni::IO::Directory::System::Temporary(path) and path.size() < maxPathSize;
		if (unlikely(not r))
			throw "failed to get temporary folder";
		auto& mp = m_mounts[m_count++];
		mp.path = "/tmp";
		nyio_adapter_init_localfolder(&mp.adapter, path.c_str(), path.size());
	}
	// mount '/' -> '/root'
	{
		auto& mp = m_mounts[m_count++];
		mp.path = "/root";
		nyio_adapter_init_localfolder(&mp.adapter, nullptr, 0u);
	}
}

Mountpoints::~Mountpoints() {
	for (uint32_t i = 0; i != m_count; ++i)
		release(m_mounts[i].adapter);
}

nyio_adapter_t& Mountpoints::resolve(AnyString& adapterpath, const AnyString& path) {
	// /some/root/folder[/some/adapter/folder]
	//                 ^                     ^
	//  mppath/msize --|        path/psize --|
	uint32_t psize = path.size();
	for (uint32_t i = 0; i != m_count; ++i) {
		auto& mountpoint = m_mounts[i];
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
	return m_fallback.adapter;
}

bool Mountpoints::add(const AnyString& path, nyio_adapter_t& adapter) {
	if (m_count < m_mounts.max_size() and path.size() < yuni::ShortString256::chunkSize) {
		// inserting the new mountpoint at the begining
		if (m_count++ != 0) {
			uint32_t i = m_count;
			while (i --)
				m_mounts[i + 1] = m_mounts[i];
		}
		auto& mp = m_mounts[0];
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

nyio_adapter_t* io_resolve(nyvmthread_t* vmtx, nyanystr_t* relpath, const nyanystr_t* path) {
	try {
		if (path and path->len != 0 and path->c_str and path->len < 1024 * 1024) {
			auto& thread = *reinterpret_cast<ny::vm::Thread*>(vmtx->internal);
			AnyString result;
			AnyString pth{path->c_str, static_cast<uint32_t>(path->len)};
			auto& adapter = thread.io.mountpoints.resolve(result, pth);
			if (relpath) {
				relpath->c_str = result.c_str();
				relpath->len = result.size();
			}
			return &adapter;
		}
	}
	catch (...) {
	}
	return nullptr;
}

const char* io_get_cwd(nyvmthread_t* vmtx, uint32_t* length) {
	auto& thread = *reinterpret_cast<ny::vm::Thread*>(vmtx->internal);
	auto& cwd = thread.io.cwd;
	if (length)
		*length = cwd.size();
	return cwd.c_str();
}

} // namespace vm
} // namespace ny
