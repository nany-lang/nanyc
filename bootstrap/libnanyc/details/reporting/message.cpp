#include "message.h"
#include "details/atom/atom.h"
#include <yuni/core/system/console/console.h>

using namespace Yuni;

namespace ny::Logs {

namespace {

void propagateError(Message* message) {
	do {
		message->hasErrors = true;
		message = message->parent;
	}
	while (message != nullptr);
}

} // namespace

Message& Message::createEntry(Level level) {
	auto entry = std::make_shared<Message>(level);
	MutexLocker locker{m_mutex};
	entry->origins = origins;
	entry->parent = this;
	entries.push_back(entry);
	if (static_cast<uint32_t>(level) <= static_cast<uint32_t>(Level::error))
		propagateError(this);
	return *entry;
}


void Message::appendEntry(std::unique_ptr<Message>& entry) {
	if (!!entry) {
		if (not (entry->entries.empty() and entry->level == Level::none)) {
			MutexLocker locker{m_mutex};
			entry->parent = this;
			if (entry->hasErrors)
				propagateError(this);
			entries.emplace_back(entry.release());
		}
	}
}


void Message::Origin::Location::resetFromAtom(const Atom& atom) {
	filename = atom.origin.filename;
	pos.line = atom.origin.line;
	pos.offset = atom.origin.offset;
}


} // ny::Logs
