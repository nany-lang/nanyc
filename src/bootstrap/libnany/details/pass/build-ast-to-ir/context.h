#pragma once
#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/fwd.h"
#include "details/ir/program.h"
#include "details/reporting/report.h"
#include "details/grammar/nany.h"
#include <map>
#include <cassert>




namespace Nany
{
namespace IR
{
namespace Producer
{

	// forward declaration
	class Scope;




	/*!
	** \brief Context for IR generation
	*/
	class Context final : public Yuni::NonCopyable<Context>
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		explicit Context(Program& program, Logs::Report);
		//@}


		//! \name Utilities: IR generation
		//@{
		/*!
		** \brief Generate opcode for using a namespace
		** \param nmspc namespace (ex: std.nany.example)
		*/
		void useNamespace(const AnyString& nmspc);

		/*!
		** \brief Generate a mapping between input offsets and line numbers
		*/
		void generateLineIndexes(const AnyString& content);
		//@}



	public:
		//! Linked IR program
		Program& program;
		//! Reporting
		Logs::Report report;
		//! Has debug info ?
		bool debuginfo = true;

		//! Debug source filename
		Yuni::String dbgSourceFilename;
		//! Map contet offset (0-based - bytes) -> lines (1-based, from source input)
		std::map<uint, uint> offsetToLine;

		struct {
			struct {
				Node::Ptr node;
				Node::Ptr classname;
				Node::Ptr lvidnode;
			}
			literal;
		}
		reuse;

	private:
		friend class Scope;

	private:
		uint32_t pPreviousDbgOffset = 0;
		uint32_t pPreviousDbgLine = 0;

	}; // class Producer






} // namespace Producer
} // namespace IR
} // namespace Nany

#include "scope.h"
#include "context.hxx"
