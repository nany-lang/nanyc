#include "context.h"

using namespace Yuni;

namespace ny::ir::Producer {

Context::Context(AnyString filename, Sequence& ircode, Logs::Report report, bool ignoreAtoms)
	: ignoreAtoms(ignoreAtoms)
	, ircode(ircode)
	, report(report)
	, dbgSourceFilename(filename)
	, localErrorHandler(this, &emitReportEntry)
	, localMetadataHandler(this, &retriveReportMetadata) {
}

Logs::Report Context::emitReportEntry(void* self, Logs::Level level) {
	auto& ctx = *(reinterpret_cast<Context*>(self));
	return ctx.report.fromErrLevel(level);
}

void Context::retriveReportMetadata(void* self, Logs::Level, const AST::Node* node, String& filename,
		uint32_t& line, uint32_t& offset) {
	auto& ctx = *(reinterpret_cast<Context*>(self));
	filename = ctx.dbgSourceFilename;
	if (node and node->offset > 0) {
		auto it = ctx.offsetToLine.lower_bound(node->offset);
		if (it != ctx.offsetToLine.end()) {
			if (it->first == node->offset or (--it != ctx.offsetToLine.end())) {
				line = it->second;
				offset = node->offset - it->first;
				return;
			}
		}
		line = 1;
		offset = 1;
	}
	else {
		line = 0;
		offset = 0;
	}
}

void Context::useNamespace(const AnyString& nmspc) {
	if (not nmspc.empty()) {
		nmspc.words(".", [&](const AnyString & part) -> bool {
			ir::emit::blueprint::namespacedef(ircode, part);
			return true;
		});
	}
}

void Context::generateLineIndexes(const AnyString& content) {
	uint32_t line = 1;
	const char* base = content.c_str();
	const char* end  = base + content.size();
	for (const char* c = base; c != end; ++c) {
		if (*c == '\n')
			offsetToLine.emplace(static_cast<uint32_t>(c - base), ++line);
	}
}

} // namespace ny::ir::Producer
