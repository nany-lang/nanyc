#include "details/context/source.h"
#include "details/context/build-info.h"
#include "details/reporting/report.h"
#include "scope.h"
#include "details/ir/emit.h"

using namespace Yuni;





namespace ny
{

	bool Source::passTransformASTToIRWL(Logs::Report& report)
	{
		auto& buildinfo = *pBuildInfo;
		if (nullptr == buildinfo.parsing.rootnode)
			return false;

		auto& astnodes = buildinfo.parsing.rootnode->children;
		if (unlikely(astnodes.empty()))
			return true;

		// output IR sequence
		auto& out = buildinfo.parsing.sequence;

		// helper for generating IR code
		auto producer = std::make_unique<ir::Producer::Context>(buildinfo.cf, m_filename, out, report);
		// generate namespace-related opcodes
		producer->useNamespace(buildinfo.parsing.nmspc.first);
		// map code offset (in bytes) with line numbers (from source input)
		producer->generateLineIndexes(m_content);

		// generate IR code for all AST nodes
		ir::Producer::Scope scope{*producer};
		scope.addDebugCurrentFilename();
		uint32_t bpoffset = out.emitBlueprintUnit(m_filename);
		uint32_t bpoffsiz = out.emitBlueprintSize();
		uint32_t bpoffsck = out.emitStackSizeIncrease();

		bool success = true;
		for (uint32_t i = 0; i != astnodes.size(); ++i)
			success &= scope.visitAST(astnodes[i]);

		ir::emit::scopeEnd(out);
		uint32_t blpsize = out.opcodeCount() - bpoffset;
		out.at<ir::ISA::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
		scope.sequence().at<ir::ISA::Op::stacksize>(bpoffsck).add = scope.nextvar();

		// do not keep back information
		buildinfo.parsing.parser.clear();
		m_content.clear();
		m_content.shrink();
		return success;
	}




} // namespace ny
