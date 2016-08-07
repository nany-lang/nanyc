#pragma once
#include <yuni/yuni.h>
#include "program.h"
#include "details/ir/sequence.h"




namespace Nany
{
namespace VM
{

	class ThreadContext final
	{
	public:
		struct Mountpoint final
		{
			Yuni::ShortString256 path;
			nyio_adapter_t adapter;
		};

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


		bool initializeProgramSettings();

	public:
		//! Attached program
		Program& program;

		struct IO {
			nyio_adapter_t& resolve(AnyString& relativepath, const AnyString& path);
			void addMountpoint(const AnyString& path, nyio_adapter_t&);

			//! Current working directory
			std::vector<Mountpoint> mountpoints;
			nyio_adapter_t fallbackAdapter;
			// Current Working directory
			Yuni::String cwd;
		}
		io;

		//! Temporary structure for complex return values by intrinsics
		struct {
			uint64_t size;
			uint64_t capacity;
			union {uint64_t u64; void* ptr; } data;
		}
		returnValue;

		nyprogram_cf_t& cf;
		//! Thread name
		Yuni::ShortString64 name;

	private:
		void initFallbackAdapter(nyio_adapter_t&);

	}; // class ThreadContext







} // namespace VM
} // namespace Nany

#include "thread-context.hxx"
