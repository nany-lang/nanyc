#pragma once
#include "context.h"
#include "details/atom/classdef-table.h"
#include "details/intrinsic/intrinsic-table.h"
#include "details/ir/program.h"




namespace Nany
{

	// TODO remove this class
	class Isolate final
	{
	public:
		struct AttachedProgramRef final
		{
			AttachedProgramRef(IR::Program* program, bool owned)
				: program(program), owned(owned) {}
			AttachedProgramRef(AttachedProgramRef&&) = default;
			AttachedProgramRef(const AttachedProgramRef&) = delete;
			~AttachedProgramRef();

			IR::Program* program = nullptr;
			bool owned = false;
		};


	public:
		//! \name Constructor
		//@{
		Isolate(const IntrinsicTable& intrinsics) : intrinsics(intrinsics) {}
		Isolate(const Isolate&) = delete;
		~Isolate() = default;
		//@}


		//! \name Program
		//@{
		//! Attach an IR program
		bool attach(IR::Program& program, Logs::Report& report, bool owned = false);
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
		//! Intrinsics (must be protected by itself)
		const IntrinsicTable& intrinsics;

		Yuni::Mutex mutex;

	private:
		void doPrintInstanciatedCodeForAtomWL(Logs::Report& report, Yuni::Clob& out, YString& tmp, const Atom& atom) const;

	private:
		std::vector<AttachedProgramRef> pAttachedPrograms;

	}; // class Isolate






} // namespace Nany
