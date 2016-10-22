#include "details/context/source.h"
#include "details/context/build-info.h"
#include "details/reporting/report.h"
#include "details/pass/build-ast-to-ir/context.h"
#include "details/pass/build-ast-to-ir/scope.h"

using namespace Yuni;





namespace Nany
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
		auto producer = std::make_unique<IR::Producer::Context>(buildinfo.cf, m_filename, out, report);
		// generate namespace-related opcodes
		producer->useNamespace(buildinfo.parsing.nmspc.first);
		// map code offset (in bytes) with line numbers (from source input)
		producer->generateLineIndexes(m_content);

		// generate IR code for all AST nodes
		IR::Producer::Scope scope{*producer};
		scope.addDebugCurrentFilename();
		uint32_t bpoffset = out.emitBlueprintUnit(m_filename);
		uint32_t bpoffsiz = out.emitBlueprintSize();
		uint32_t bpoffsck = out.emitStackSizeIncrease();

		bool success = true;
		for (uint32_t i = 0; i != astnodes.size(); ++i)
			success &= scope.visitAST(astnodes[i]);

		out.emitEnd();
		uint32_t blpsize = out.opcodeCount() - bpoffset;
		out.at<IR::ISA::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
		scope.sequence().at<IR::ISA::Op::stacksize>(bpoffsck).add = scope.nextvar();

		// do not keep back information
		buildinfo.parsing.parser.clear();
		m_content.clear();
		m_content.shrink();
		return success;
	}




} // namespace Nany
