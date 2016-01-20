#pragma once
#include "context.h"
#include "details/atom/classdef-table.h"
#include "details/ir/sequence.h"
#include "nany/nany.h"




namespace Nany
{

	// TODO remove this class
	class Isolate final
	{
	public:
		struct AttachedSequenceRef final
		{
			AttachedSequenceRef(IR::Sequence* sequence, bool owned)
				: sequence(sequence), owned(owned) {}
			AttachedSequenceRef(AttachedSequenceRef&&) = default;
			AttachedSequenceRef(const AttachedSequenceRef&) = delete;
			~AttachedSequenceRef();

			IR::Sequence* sequence = nullptr;
			bool owned = false;
		};


	public:
		//! \name Constructor
		//@{
		Isolate(nycontext_t& ctx) : context(ctx) {}
		Isolate(const Isolate&) = delete;
		~Isolate() = default;
		//@}


		//! \name Sequence
		//@{
		//! Attach an IR sequence
		bool attach(IR::Sequence& sequence, Logs::Report& report, bool owned = false);
		//@}

		//! \name Compilation
		//@{
		/*!
		** \brief Try to instanciate an entry point
		*/
		bool instanciate(Logs::Report report, const AnyString& entrypoint);
		//@}

		//! \name Run
		//@{
		/*!
		** \brief Try to instanciate an entry point
		*/
		int run(bool& success, nycontext_t&, const AnyString& entrypoint);
		//@}


		//! \name Debug
		//@{
		//! Print all opcode to logs
		void print(Logs::Report&) const;
		//@}


		//! \name Operators
		//@{
		Isolate& operator = (const Isolate&) = delete;
		//@}


	public:
		//! The datatype matrix (must be protected by self)
		ClassdefTable classdefTable;
		//! Context
		nycontext_t& context;

		Yuni::Mutex mutex;

	private:
		void doPrintInstanciatedCodeForAtomWL(Logs::Report& report, Yuni::Clob& out, YString& tmp, const Atom& atom) const;

	private:
		std::vector<AttachedSequenceRef> pAttachedSequences;

	}; // class Isolate






} // namespace Nany
