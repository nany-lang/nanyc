#pragma once
#include "libnanyc.h"
#include <yuni/core/noncopyable.h>
#include "details/ir/sequence.h"
#include "details/reporting/report.h"
#include "details/grammar/nany.h"
#include "details/errors/errors.h"
#include "nany/nany.h"
#include <map>
#include <array>
#include <cassert>




namespace ny {
namespace ir {
namespace Producer {

// forward declaration
class Scope;




/*!
** \brief Context for ir generation
*/
class Context final : public Yuni::NonCopyable<Context> {
public:
	//! \name Constructor & Destructor
	//@{
	//! Default constructor
	explicit Context(nybuild_cf_t&, AnyString filename, Sequence&, Logs::Report, bool ignoreAtoms);
	//@}


	//! \name Utilities: ir generation
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
	//! re=use objects for executing code when exiting the scope
	void prepareReuseForScopeExit();
	//@}


	void invalidateLastDebugLine();


public:
	//! Discard ir code generation for atoms
	bool ignoreAtoms = false;

	//! Information about the current build
	nybuild_cf_t& cf;
	//! Linked ir code
	Sequence& ircode;
	//! Reporting
	Logs::Report report;
	//! Has debug info ?
	bool debuginfo = true;

	//! Map contet offset (0-based - bytes) -> lines (1-based, from source input)
	std::map<uint, uint> offsetToLine;

	struct {
		struct {
			yuni::Ref<AST::Node> node;
			yuni::Ref<AST::Node> classname = nullptr;
			yuni::Ref<AST::Node> lvidnode = nullptr;
		}
		literal;

		struct {
			yuni::Ref<AST::Node> createObject;
			YString text;
		}
		string;

		struct {
			yuni::Ref<AST::Node> node;
			yuni::Ref<AST::Node> lvidnode = nullptr;
		}
		ascii;

		struct {
			yuni::Ref<AST::Node> node;
			yuni::Ref<AST::Node> funcname;
			yuni::Ref<AST::Node> varname = nullptr;
			yuni::Ref<AST::Node> callparam = nullptr;
		}
		func;

		struct {
			yuni::Ref<AST::Node> node;
			yuni::Ref<AST::Node> funcname;
		}
		operatorDefault;

		struct {
			yuni::Ref<AST::Node> node;
			yuni::Ref<AST::Node> funcname;
		}
		operatorClone;

		struct {
			yuni::Ref<AST::Node> node; // expr
			yuni::Ref<AST::Node> func = nullptr;
			yuni::Ref<AST::Node> classdef = nullptr;
			yuni::Ref<AST::Node> funcbody = nullptr;
			yuni::Ref<AST::Node> params = nullptr;
			yuni::Ref<AST::Node> rettype = nullptr;
		}
		closure;

		struct {
			yuni::Ref<AST::Node> node;
			yuni::Ref<AST::Node> container = nullptr;
			yuni::Ref<AST::Node> viewname = nullptr;
			yuni::Ref<AST::Node> elementname = nullptr;
			yuni::Ref<AST::Node> predicate = nullptr;
			yuni::Ref<AST::Node> call = nullptr;
			yuni::Ref<AST::Node> premadeAlwaysTrue;
		}
		inset;

		struct {
			yuni::Ref<AST::Node> node;
			yuni::Ref<AST::Node> viewlvid = nullptr;
			std::array<yuni::Ref<AST::Node>, 4> cursorname;
			yuni::Ref<AST::Node> elementname = nullptr;
			yuni::Ref<AST::Node> scope;

			yuni::Ref<AST::Node> ifnode = nullptr;
			yuni::Ref<AST::Node> elseClause;
			yuni::Ref<AST::Node> elseScope = nullptr;
		}
		loops;

		struct {
			struct {
				yuni::Ref<AST::Node> node;
				yuni::Ref<AST::Node> propname = nullptr;
				yuni::Ref<AST::Node> returnValue = nullptr;

				//! node type
				yuni::Ref<AST::Node> type = nullptr;
				//! node to use as a child when any
				yuni::Ref<AST::Node> typeIsAny;
				//! node to use as a child when ref
				yuni::Ref<AST::Node> typeIsRefAny;
			}
			get;

			struct {
				yuni::Ref<AST::Node> node;
				yuni::Ref<AST::Node> propname = nullptr;
				yuni::Ref<AST::Node> body = nullptr;
			}
			set;
		}
		properties;

		struct {
			yuni::Ref<AST::Node> node;
			yuni::Ref<AST::Node> funcname = nullptr;
			yuni::Ref<AST::Node> funcbody = nullptr;
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
			yuni::Ref<AST::Node> node;
			yuni::Ref<AST::Node> classbody = nullptr;
		}
		object;

		struct {
			yuni::Ref<AST::Node> node;
			yuni::Ref<AST::Node> typeofcall = nullptr;
		}
		shorthandArray;

		struct {
			struct {
				yuni::Ref<AST::Node> node;
				yuni::Ref<AST::Node> body;
			}
			exit;
		}
		scope;
	}
	reuse;

	//! Debug current source filename
	AnyString dbgSourceFilename;

	struct final {
		void* userdata = nullptr;
		void (*on_unittest)(void* userdata, const char* mod, uint32_t mlen, const char* name, uint32_t nlen) = nullptr;
	}
	event;

private:
	friend class Scope;
	static Logs::Report emitReportEntry(void*, Logs::Level level);
	static void retriveReportMetadata(void*, Logs::Level, const AST::Node*, Yuni::String&, uint32_t&, uint32_t&);

private:
	uint32_t pPreviousDbgOffset = 0;
	uint32_t pPreviousDbgLine = 0;
	//! Error reporting
	Logs::Handler localErrorHandler;
	Logs::MetadataHandler localMetadataHandler;
	AnyString pFilename;

}; // class Producer






} // namespace Producer
} // namespace ir
} // namespace ny

#include "scope.h"
#include "context.hxx"
