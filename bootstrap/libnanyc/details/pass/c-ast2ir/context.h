#pragma once
#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/fwd.h"
#include "details/ir/sequence.h"
#include "details/reporting/report.h"
#include "details/grammar/nany.h"
#include "details/errors/errors.h"
#include "nany/nany.h"
#include <map>
#include <array>
#include <cassert>




namespace ny
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
		explicit Context(nybuild_cf_t&, AnyString filename, Sequence&, Logs::Report);
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


		//! \name Reuse nodes
		//@{
		//! re-use objects for string declaration
		void prepareReuseForStrings();
		//! re-use objects for ascii
		void prepareReuseForAsciis();
		//! re-use objects for classes (ctor)
		void prepareReuseForClasses();
		//! re-use objects for literals
		void prepareReuseForLiterals();
		//! re-use objects for clusores
		void prepareReuseForClosures();
		//! re-use objects for variable members
		void prepareReuseForVariableMembers();
		//! re-use objects for in
		void prepareReuseForIn();
		//! re-use objects for for..do
		void prepareReuseForLoops();
		//! re-use objects for properties
		void prepareReuseForPropertiesGET();
		//! re-use objects for properties
		void prepareReuseForPropertiesSET();
		//! re-use objects for unittest
		void prepareReuseForUnittest();
		//! re-use objects for anonymous objects
		void prepareReuseForAnonymObjects();
		//! re-use objects for shorthand arrays
		void prepareReuseForShorthandArray();
		//@}


		void invalidateLastDebugLine();


	public:
		//! Discard IR code generation for atoms
		bool ignoreAtoms = false;

		//! Information about the current build
		nybuild_cf_t& cf;
		//! Linked IR sequence
		Sequence& sequence;
		//! Reporting
		Logs::Report report;
		//! Has debug info ?
		bool debuginfo = true;

		//! Map contet offset (0-based - bytes) -> lines (1-based, from source input)
		std::map<uint, uint> offsetToLine;

		struct {
			struct {
				AST::Node::Ptr node;
				AST::Node* classname = nullptr;
				AST::Node* lvidnode = nullptr;
			}
			literal;

			struct {
				AST::Node::Ptr createObject;
				YString text;
			}
			string;

			struct {
				AST::Node::Ptr node;
				AST::Node* lvidnode = nullptr;
			}
			ascii;

			struct {
				AST::Node::Ptr node;
				AST::Node* funcname = nullptr;
				AST::Node* varname = nullptr;
				AST::Node* callparam = nullptr;
			}
			func;

			struct {
				AST::Node::Ptr node;
				AST::Node* funcname = nullptr;
			}
			operatorDefault;

			struct {
				AST::Node::Ptr node;
				AST::Node* funcname = nullptr;
			}
			operatorClone;

			struct {
				AST::Node::Ptr node; // expr
				AST::Node* func = nullptr;
				AST::Node* classdef = nullptr;
				AST::Node* funcbody = nullptr;
				AST::Node* params = nullptr;
				AST::Node* rettype = nullptr;
			}
			closure;

			struct {
				AST::Node::Ptr node;
				AST::Node* container = nullptr;
				AST::Node* viewname = nullptr;
				AST::Node* elementname = nullptr;
				AST::Node* predicate = nullptr;
				AST::Node* call = nullptr;
				AST::Node::Ptr premadeAlwaysTrue;
			}
			inset;

			struct {
				AST::Node::Ptr node;
				AST::Node* viewlvid = nullptr;
				std::array<AST::Node*,4> cursorname;
				AST::Node* elementname = nullptr;
				AST::Node* scope;

				AST::Node* ifnode = nullptr;
				AST::Node::Ptr elseClause;
				AST::Node* elseScope = nullptr;
			}
			loops;

			struct {
				struct {
					AST::Node::Ptr node;
					AST::Node* propname = nullptr;
					AST::Node* returnValue = nullptr;

					//! node type
					AST::Node* type = nullptr;
					//! node to use as a child when any
					AST::Node::Ptr typeIsAny;
					//! node to use as a child when ref
					AST::Node::Ptr typeIsRefAny;
				}
				get;

				struct {
					AST::Node::Ptr node;
					AST::Node* propname = nullptr;
					AST::Node* body = nullptr;
				}
				set;
			}
			properties;

			struct {
				AST::Node::Ptr node;
				AST::Node* funcname = nullptr;
				AST::Node* funcbody = nullptr;
			}
			unittest;

			struct {
				//! Temporary buffer for the attribute name
				Yuni::ShortString32 attrname;
				//! Temporary buffer for the attribute value (if any)
				Yuni::ShortString32 value;
			}
			attributes;

			struct {
				AST::Node::Ptr node;
				AST::Node* classbody = nullptr;
			}
			object;

			struct {
				AST::Node::Ptr node;
				AST::Node* typeofcall = nullptr;
			}
			shorthandArray;
		}
		reuse;

	private:
		friend class Scope;
		static Logs::Report emitReportEntry(void*, Logs::Level level);
		static void retriveReportMetadata(void*, Logs::Level, const AST::Node*, Yuni::String&, uint32_t&, uint32_t&);

	private:
		uint32_t pPreviousDbgOffset = 0;
		uint32_t pPreviousDbgLine = 0;
		//! Debug source filename
		AnyString dbgSourceFilename;

		//! Error reporting
		Logs::Handler localErrorHandler;
		Logs::MetadataHandler localMetadataHandler;
		AnyString pFilename;

	}; // class Producer






} // namespace Producer
} // namespace IR
} // namespace ny

#include "scope.h"
#include "context.hxx"
