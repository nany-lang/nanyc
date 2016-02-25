#pragma once
#include <yuni/yuni.h>
#include <yuni/core/string.h>
#include "details/reporting/report.h"
#include "details/atom/atom.h"
#include <cassert>
#include <vector>



namespace Nany
{
namespace Pass
{
namespace Instanciate
{


	struct ReturnValueMarker
	{
		ReturnValueMarker(const CLID& clid, uint32_t line, uint32_t offset)
			: clid(clid), line(line), offset(offset)
		{}
		CLID clid;
		uint32_t line, offset;
	};


	struct LVIDInfo final
	{
		//! List of referer
		uint32_t referer = 0;
		//! List of all fully-defined variables
		bool errorReported = false;
		//! LVID marked as any
		bool markedAsAny = false;
		//! LVID marked as dynamic
		bool isConstexpr = false;
		//! LVID marked as assignment '=' (for name/call resolution)
		bool isAssignment = false;
		//! flag to remember if the variable has been used
		bool hasBeenUsed = false;
		//! unref at the end of the scope
		bool autorelease = false;
		//! Flag for synthetic values (type, func...) contrary to real user variables
		bool synthetic = true;

		//! The scope depth when the variable has been declared
		int scope = -1; // -1: no scope, : 0 first scope
		//! declaration offset (stackalloc) in the out IR sequence
		uint32_t offsetDeclOut = (uint32_t) -1;
		//! lvid alias
		uint32_t alias = 0;
		//! Any associated text
		uint32_t text_sid = (uint32_t) -1;

		struct {
			//! Does the value come from a memory allocation ?
			bool memalloc = false;
			//! Does the value come from a func call ?
			bool returnedValue = false;

			//! Does the value come from a variable member ? (atomid != 0)
			struct {
				uint32_t self   = 0u; // lvid
				uint32_t atomid = 0u;
				uint32_t field  = 0u;
			}
			varMember;
		}
		origin;

		//! File origin
		struct {
			const char* url = nullptr;
			//! Line in the associated file
			uint32_t line = 1u;
			//! Offset in the associated file
			uint32_t offset = 1u;
		}
		file;

		//! warnings (valid only if userDefinedName is valid)
		struct {
			//! Warn if unused
			bool unused = true;
		}
		warning;

		//! Resolved name (if any)
		AnyString resolvedName;
		//! User-defined name for the local variable
		AnyString userDefinedName;


	public:
		void fillLogEntryWithLocation(Logs::Report& entry) const;

	}; // class LVIDInfo



	//! A single element within the stack for analysing opcodes
	struct AtomStackFrame final
	{
		explicit AtomStackFrame(Atom& atom);

		uint32_t localVariablesCount() const;

		uint32_t findLocalVariable(const AnyString& name) const;

		void resizeRegisterCount(uint32_t count, ClassdefTableView& table);

		//! Check if a lvid is valid
		bool verify(uint32_t lvid) const;

		void invalidate(uint32_t lvid);


	public:
		//! The atom
		Atom& atom;
		//! Current atomid
		yuint32 atomid = 0;
		//! Information on local registers
		std::vector<LVIDInfo> lvids;
		//! Current scope depth
		int scope = 0;

		//! list of return values
		std::vector<ReturnValueMarker> returnValues;

		//! List of possible solutions per clid
		// TODO use a more efficient container
		std::unordered_map<CLID, std::vector<std::reference_wrapper<Atom>>> resolvePerCLID;

		//! Self parameters for ctors
		// {varname/parameter} -> {lvid, used flag}
		std::unique_ptr<std::unordered_map<AnyString, std::pair<LVID, bool>>> selfParameters;

		uint32_t blueprintOpcodeOffset = (uint32_t) -1;
	};






} // namespace Instanciate
} // namespace Pass
} // namespace Nany

#include "stack-frame.hxx"
