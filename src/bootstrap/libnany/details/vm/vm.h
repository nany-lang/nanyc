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
		//! Default constructor
		explicit ThreadContext(Program& program, const AnyString& name);
		//! Clone a thread context
		explicit ThreadContext(ThreadContext&);
		//! Destructor
		~ThreadContext() = default;

		//! Get the equivalent C type
		nytctx_t* self();
		//! Get the equivalent C type (const)
		const nytctx_t* self() const;

		/*!
		** \brief Print a message on the console
		*/
		void cerr(const AnyString& msg);
		/*!
		** \brief Print a message on the console
		*/
		void cerrColor(nycolor_t);

		void cerrException(const AnyString& msg);


		void triggerEventsDestroy();

		uint64_t invoke(const IR::Sequence& callee, uint32_t atomid, uint32_t instanceid);


	public:
		//! Attached program
		Program& program;

		nyprogram_cf_t& cf;

		//! Thread name
		Yuni::ShortString32 name;

	}; // class ThreadContext







} // namespace VM
} // namespace Nany

#include "vm.hxx"
