#pragma once
#include "details/ir/sequence.h"
#include "nany/nany.h"
#include "details/atom/atom-map.h"
#include "details/context/build.h"
#include "libnany-config.h"



namespace Nany
{
namespace VM
{


	class Program final
	{
	public:
		/*!
		** \brief Create a brand new program
		*/
		Program(Build&, const AtomMap&);

		/*!
		** \brief Create a program from another program
		**
		** This constructor can be used to create a new execution stack
		** (for example in a new thread)
		*/
		explicit Program(Program& inherit);

		/*!
		** \brief Execute the main entry point
		*/
		bool execute(uint32_t atomid, uint32_t instanceid);

		/*!
		** \brief Print a message on the console
		*/
		void printStderr(const AnyString& msg);

		nyprogram_t* self();
		const nyprogram_t* self() const;


	public:
		//! Settings
		nyprogram_cf_t cf;

		//! User build
		Build& build;
		//! Atom Map
		const AtomMap& map;

		//! Return value, a pod or an object
		uint64_t retvalue = 0;
		//! Does the program own the sequence ?
		bool ownsSequence = false;

	}; // class Program






} // namespace VM
} // namespace Nany

#include "vm.hxx"
