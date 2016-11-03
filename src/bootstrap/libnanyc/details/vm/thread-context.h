#pragma once
#include <yuni/yuni.h>
#include "program.h"
#include "details/ir/sequence.h"
#include <array>




namespace ny
{
namespace vm
{

	struct Mountpoint final
	{
		Yuni::ShortString256 path;
		nyio_adapter_t adapter;
	};


	class ThreadContext final
	{
	public:
		//! Default constructor
		explicit ThreadContext(Program& program, const AnyString& name);
		//! Clone a thread context
		explicit ThreadContext(ThreadContext&);
		//! Destructor
		~ThreadContext();

		//! Get the equivalent C type
		nytctx_t* self();
		//! Get the equivalent C type (const)
		const nytctx_t* self() const;

		/*!
		** \brief Print a message on the console
		*/
		void cerr(const AnyString& msg);
		//! Set the text color on the error output
		void cerrColor(nycolor_t);

		void cerrException(const AnyString& msg);
		void cerrUnknownPointer(void*, uint32_t offset);


		bool invoke(uint64_t& exitstatus, const IR::Sequence& callee, uint32_t atomid, uint32_t instanceid);

		bool initializeFirstTContext();


	public:
		//! Attached program
		Program& program;

		//! Temporary structure for complex return values by intrinsics
		struct {
			uint64_t size;
			uint64_t capacity;
			void* data;
		}
		returnValue;

		struct IO {
			/*!
			** \briefFind the adapter and the relative adapter path from a virtual path
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
			bool addMountpoint(const AnyString& path, nyio_adapter_t& adapter);

		public:
			// Current Working directory
			Yuni::String cwd;
			//! The total number of mountpoints
			uint32_t mountpointSize = 0;
			//! All mountpoints, from the last added to the first one (stored in the reverse order)
			std::array<Mountpoint, 32> mountpoints;
			//! Current working directory
			Mountpoint fallback;
		}
		io;

		nyprogram_cf_t& cf;
		//! Thread name
		Yuni::ShortString64 name;

	private:
		void initFallbackAdapter(nyio_adapter_t& adapter);

	}; // class ThreadContext


} // namespace vm
} // namespace ny

#include "thread-context.hxx"
