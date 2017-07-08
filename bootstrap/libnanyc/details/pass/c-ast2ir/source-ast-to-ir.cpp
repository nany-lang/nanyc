#include "source-ast-to-ir.h"
#include "scope.h"
#include "details/ir/emit.h"

using namespace Yuni;


namespace ny {
namespace compiler {

bool passTransformASTToIR(ny::compiler::Source& source, Logs::Report& report, const nycompile_opts_t& opts) {
	if (unlikely(!source.parsing.rootnode))
		return false;
	auto& astnodes = source.parsing.rootnode->children;
	if (unlikely(astnodes.empty()))
		return true;
	auto& irout = source.parsing.ircode;
	bool ignoreAtoms = opts.on_unittest != nullptr;
	// helper for generating IR code
	ir::Producer::Context producer(source.filename, irout, report, ignoreAtoms);
	producer.event.userdata = opts.userdata;
	producer.event.on_unittest = opts.on_unittest;
	// generate namespace-related opcodes
	producer.useNamespace(source.parsing.nmspc.first);
	// map code offset (in bytes) with line numbers (from source input)
	producer.generateLineIndexes(source.parsing.parser.firstSourceContent());
	// generate IR code for all AST nodes
	ir::Producer::Scope scope{producer};
	ir::emit::dbginfo::filename(irout, scope.context.dbgSourceFilename);
	uint32_t bpoffset = ir::emit::blueprint::unit(irout, source.filename);
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
	source.parsing.parser.clear();
	return success;
}

} // namespace compiler
} // namespace ny
