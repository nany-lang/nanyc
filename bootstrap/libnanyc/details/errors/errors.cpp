#include "errors.h"
#include <yuni/thread/mutex.h>
#include "../grammar/nany.h"
#include <unordered_map>
#include <memory>

using namespace Yuni;


namespace ny {
namespace Logs {


struct InternalState final {
	void* userdefined = nullptr;
	Callback callback = nullptr;
};


static thread_local InternalState localHandler;


struct InternalMetadataState final {
	void* userdefined = nullptr;
	CallbackMetadata callback = nullptr;
};

static thread_local InternalMetadataState localMetadataHandler;


Handler::Handler(void* userdefined, Callback callback) {
	auto& handler = localHandler;
	// save the current state
	previousState.userdefined = handler.userdefined;
	previousState.callback    = handler.callback;
	// install the new state
	handler.userdefined = userdefined;
	handler.callback = callback;
}


Handler::~Handler() {
	// restore the previous state
	auto& handler = localHandler;
	handler.userdefined = previousState.userdefined;
	handler.callback = previousState.callback;
}


Handler::State Handler::exportThreadState() {
	auto& handler = localHandler;
	return State{handler.userdefined, handler.callback};
}


MetadataHandler::MetadataHandler(void* userdefined, CallbackMetadata callback) {
	auto& handler = localMetadataHandler;
	// save the current state
	previousState.userdefined = handler.userdefined;
	previousState.callback    = handler.callback;
	// install the new state
	handler.userdefined = userdefined;
	handler.callback = callback;
}


MetadataHandler::~MetadataHandler() {
	// restore the previous state
	auto& handler = localMetadataHandler;
	handler.userdefined = previousState.userdefined;
	handler.callback = previousState.callback;
}


void* userHandlerPointer() {
	return localMetadataHandler.userdefined;
}


} // namespace Logs
} // namespace ny


namespace ny {
namespace {


void retrieveMetadata(Logs::Report& entry, const AST::Node* node = nullptr) {
	auto& mdstate = Logs::localMetadataHandler;
	if (mdstate.callback) {
		auto& filename = entry.message.origins.location.filename;
		auto& line     = entry.message.origins.location.pos.line;
		auto& offset   = entry.message.origins.location.pos.offset;
		auto level     = entry.message.level;
		mdstate.callback(mdstate.userdefined, level, node, filename, line, offset);
	}
}


} // anonymous namespace


Logs::Report error() {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::error);
	retrieveMetadata(entry);
	return entry;
}

bool error(const AnyString& msg) {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::error);
	if (not msg.empty())
		entry << msg;
	else
		entry << "(empty)";
	retrieveMetadata(entry);
	return false;
}


Logs::Report error(const AST::Node& node) {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::error);
	retrieveMetadata(entry, &node);
	return entry;
}


void error(const AST::Node& node, const AnyString& msg) {
	error(node) << msg;
}


Logs::Report warning() {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::warning);
	retrieveMetadata(entry);
	return entry;
}

Logs::Report warning(const AST::Node& node) {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::warning);
	retrieveMetadata(entry, &node);
	return entry;
}


Logs::Report ice() {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::ICE);
	retrieveMetadata(entry);
	return entry;
}


Logs::Report ice(const AST::Node& node) {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::ICE);
	retrieveMetadata(entry, &node);
	return entry;
}


bool unexpectedNode(const AST::Node& node, const AnyString& extra) {
	if (not AST::ruleIsError(node.rule)) {
		auto report = ice(node);
		auto rulename = AST::ruleToString(node.rule);
		report << "unexpected node '" << rulename << '\'';
		if (not extra.empty())
			report << ' ' << extra;
	}
	return false;
}


Logs::Report info() {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::info);
	retrieveMetadata(entry);
	return entry;
}


Logs::Report hint() {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::hint);
	retrieveMetadata(entry);
	return entry;
}


Logs::Report trace() {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::trace);
	retrieveMetadata(entry);
	return entry;
}


Logs::Report verbose() {
	auto& state = Logs::localHandler;
	auto entry = state.callback(state.userdefined, Logs::Level::verbose);
	retrieveMetadata(entry);
	return entry;
}


} // namespace ny
