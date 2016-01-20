#pragma once
#include "details/ir/sequence.h"
#include "nany/nany.h"
#include "details/atom/atom-map.h"
#include "details/context/context.h"
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
		Program(nycontext_t&, const IR::Sequence*, const AtomMap&);

		/*!
		** \brief Create a program from another program
		**
		** This constructor can be used to create a new execution stack
		** (for example in a new thread)
		*/
		explicit Program(Program& inherit);

		//! Destructor
		~Program();


		/*!
		** \brief Execute the main entry point
		*/
		bool execute();


	public:
		// TODO make it independent from the compilation process

		//! User context
		nycontext_t& context;
		//! Byte code
		const IR::Sequence* sequence;
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
