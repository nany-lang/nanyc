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

		bool success = true;
		auto& astnodes = buildinfo.parsing.rootnode->children;
		if (not astnodes.empty())
		{
			// output IR sequence
			auto& out = buildinfo.parsing.sequence;

			// helper for generating IR code
			IR::Producer::Context producer{out, report};
			// generate namespace-related opcodes
			producer.useNamespace(buildinfo.parsing.nmspc.first);
			producer.dbgSourceFilename = pFilename;
			// map code offset (in bytes) with line numbers (from source input)
			producer.generateLineIndexes(pContent);

			// generate IR code for all AST nodes
			IR::Producer::Scope scope{producer};
			scope.addDebugCurrentFilename();
			uint32_t bpoffset = out.emitBlueprintUnit(pFilename);
			uint32_t bpoffsiz = out.emitBlueprintSize();
			uint32_t bpoffsck = out.emitStackSizeIncrease();

			for (auto& element: astnodes)
				success &= scope.visitAST(*element);

			out.emitEnd();
			uint32_t blpsize = out.opcodeCount() - bpoffset;
			out.at<IR::ISA::Op::pragma>(bpoffsiz).value.blueprintsize = blpsize;
			scope.sequence().at<IR::ISA::Op::stacksize>(bpoffsck).add = scope.nextvar();

			// do not keep back information
			buildinfo.parsing.parser.clear();
			pContent.clear();
			pContent.shrink();
		}
		return success;
	}




} // namespace Nany
