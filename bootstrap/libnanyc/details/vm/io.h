#pragma once
#include <nanyc/io.h>
#include <nanyc/vm.h>
#include <yuni/core/string.h>
#include <array>


namespace ny {
namespace vm {

struct Mountpoints final {
	struct Entry final {
		yuni::ShortString256 path;
		nyio_adapter_t adapter;
	};

	Mountpoints();
	~Mountpoints();

	/*!
	** \brief Find the adapter and the relative adapter path from a virtual path
	**
	** \param[out] relativepath Get a non-empty absolute path (but may contain segments like '.' amd '..')
	** \param path Am aboslute virtual path
	** \return An adapter, fallbackAdapter if not found
	*/
	nyio_adapter_t& resolve(AnyString& relativepath, const AnyString& path);

	/*!
	** \brief Add a new virtual mountpoint
	**
	** \param path A Virtual path (may not exist)
	** \param adapter The adapter to handle this mountpoint
	** \return True if the operation succeeded
	*/
	bool add(const AnyString& path, nyio_adapter_t& adapter);

private:
	//! The total number of mountpoints
	uint32_t m_count = 0;
	//! All mountpoints, from the last added to the first one (stored in the reverse order)
	std::array<Entry, 32> m_mounts;
	//! Current working directory
	Entry m_fallback;
};

struct IO final {
	//! Virtual current working directory
	yuni::String cwd = "/home";
	//! All mountpoints
	Mountpoints mountpoints;
};

const char* io_get_cwd(nyvmthread_t* vmtx, uint32_t* length);

} // namespace vm
} // namespace ny
