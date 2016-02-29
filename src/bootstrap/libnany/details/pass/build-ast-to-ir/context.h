#pragma once
#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/fwd.h"
#include "details/ir/sequence.h"
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
		explicit Context(Sequence& sequence, Logs::Report);
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

		void emitTmplParameters(const std::vector<std::pair<uint32_t, AnyString>>&);
		//@}


	public:
		//! Linked IR sequence
		Sequence& sequence;
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

			struct {
				Node::Ptr createObject;
				YString text;
			}
			string;

			struct {
				Node::Ptr node;
				Node* funcname = nullptr;
				Node* varname = nullptr;
			}
			func;

			struct {
				Node::Ptr node;
				Node* funcname = nullptr;
			}
			operatorDefault;

			struct {
				Node::Ptr node;
				Node* funcname;
			}
			operatorClone;

			//! For template parameters
			std::vector<std::pair<uint32_t, AnyString>> lastPushedTmplParams;
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
