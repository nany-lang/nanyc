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
		if (!buildinfo.parsing.rootnode)
			return false;
		auto& astnodes = buildinfo.parsing.rootnode->children;

		// helper for generating IR code
		IR::Producer::Context producer{buildinfo.parsing.sequence, report};
		// generate namespace-related opcodes
		producer.useNamespace(buildinfo.parsing.nmspc.first);
		producer.dbgSourceFilename = pFilename;
		// map code offset (in bytes) with line numbers (from source input)
		producer.generateLineIndexes(pContent);

		bool success = true;

		// generate IR code for all AST nodes
		if (likely(not astnodes.empty()))
		{
			IR::Producer::Scope scope{producer};
			scope.addDebugCurrentFilename();
			uint32_t bpoffsck = scope.sequence().emitStackSizeIncrease();

			for (auto& element: astnodes)
				success &= scope.visitAST(*element);

			scope.sequence().at<IR::ISA::Op::stacksize>(bpoffsck).add = scope.nextvar();
		}

		// do not keep back information
		buildinfo.parsing.parser.clear();
		pContent.clear();
		pContent.shrink();
		return success;
	}




} // namespace Nany
