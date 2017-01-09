#include "details/context/source.h"
#include "details/context/build-info.h"
#include "details/reporting/report.h"
#include "scope.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {

bool Source::passTransformASTToIR(Logs::Report& report) {
	auto& buildinfo = *m_details;
	if (nullptr == buildinfo.parsing.rootnode)
		return false;
	auto& astnodes = buildinfo.parsing.rootnode->children;
	if (unlikely(astnodes.empty()))
		return true;
	auto& irout = buildinfo.parsing.ircode;
	// helper for generating IR code
	auto producer = std::make_unique<ir::Producer::Context>(buildinfo.cf, m_filename, irout, report);
	// generate namespace-related opcodes
	producer->useNamespace(buildinfo.parsing.nmspc.first);
	// map code offset (in bytes) with line numbers (from source input)
	producer->generateLineIndexes(m_content);
	// generate IR code for all AST nodes
	ir::Producer::Scope scope{*producer};
	ir::emit::dbginfo::filename(irout, scope.context.dbgSourceFilename);
	uint32_t bpoffset = ir::emit::blueprint::unit(irout, m_filename);
	uint32_t bpoffsiz = ir::emit::pragma::blueprintSize(irout);
	uint32_t bpoffsck = ir::emit::increaseStacksize(irout);
	bool success = true;
	for (uint32_t i = 0; i != astnodes.size(); ++i)
		success &= scope.visitAST(astnodes[i]);
	ir::emit::scopeEnd(irout);
	uint32_t blpsize = irout.opcodeCount() - bpoffset;
	irout.at<ir::isa::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
	irout.at<ir::isa::Op::stacksize>(bpoffsck).add = scope.nextvar();
	// do not keep back information
	buildinfo.parsing.parser.clear();
	m_content.clear();
	m_content.shrink();
	return success;
}


} // namespace ny
