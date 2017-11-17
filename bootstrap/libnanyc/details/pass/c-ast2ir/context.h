#pragma once
#include "libnanyc.h"
#include "details/ir/sequence.h"
#include "details/reporting/report.h"
#include "details/grammar/nany.h"
#include "details/errors/errors.h"
#include "details/pass/c-ast2ir/reuse.h"
#include <map>
#include <cassert>

namespace ny::ir::Producer {

// forward declaration
struct Scope;

//! Context for ir generation
struct Context final {
	Context(AnyString filename, Sequence&, Logs::Report, bool ignoreAtoms);
	Context(const Context&) = delete;
	Context& operator = (const Context&) = delete;

	/*!
	** \brief Generate opcode for using a namespace
	** \param nmspc namespace (ex: std.nany.example)
	*/
	void useNamespace(const AnyString& nmspc);

	//! Generate a mapping between input offsets and line numbers
	void generateLineIndexes(const AnyString& content);

	void invalidateLastDebugLine();

public:
	//! Discard ir code generation for atoms
	bool ignoreAtoms = false;
	//! Linked ir code
	Sequence& ircode;
	//! Reporting
	Logs::Report report;
	//! Has debug info ?
	bool debuginfo = true;

	//! Map contet offset (0-based - bytes) -> lines (1-based, from source input)
	std::map<uint32_t, uint32_t> offsetToLine;

	Reuse reuse;

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
	uint32_t m_previousDbgOffset = 0;
	uint32_t m_previousDbgLine = 0;
	//! Error reporting
	Logs::Handler localErrorHandler;
	Logs::MetadataHandler localMetadataHandler;
	AnyString pFilename;
}; // struct Context

} // namespace ny::ir::Producer

#include "scope.h"
#include "context.hxx"
