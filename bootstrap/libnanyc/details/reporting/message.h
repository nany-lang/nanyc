#pragma once
#include "libnanyc.h"
#include <yuni/core/smartptr/intrusive.h>
#include <yuni/core/string.h>
#include <yuni/core/noncopyable.h>
#include "levels.h"
#include "nany/nany.h"
#include <iosfwd>


namespace ny { struct Atom; }


namespace ny {
namespace Logs {


class Message final
	: public Yuni::IIntrusiveSmartPtr<Message, false, Yuni::Policy::ObjectLevelLockable>
	, Yuni::NonCopyable<Message> {
public:
	//! The class ancestor
	using Ancestor = Yuni::IIntrusiveSmartPtr<Message, false, Yuni::Policy::ObjectLevelLockable>;
	//! The most suitable smart ptr for the class
	using Ptr = Ancestor::SmartPtrType<Message>::Ptr;
	//! Threading policy
	using ThreadingPolicy = Ancestor::ThreadingPolicy;

public:
	Message(Level level)
		: level(level) {
	}

	//! Create a new sub-entry
	Message& createEntry(Level level);

	void appendEntry(const Message::Ptr& message);

	void print(nyconsole_t&, bool unify = false);

	bool isClassifiedAsError() const {
		return static_cast<uint>(level) > static_cast<uint>(Level::warning);
	}


public:
	//! Error level
	Level level = Level::error;
	//! Section
	Yuni::ShortString16 section;
	//! prefix to highlight
	YString prefix;
	//! The message itself
	YString message;

	//! Sub-entries
	std::vector<Message::Ptr> entries;

	/*!
	** \internal Default value means 'like previously'
	*/
	struct Origin final {
		//! Current filename (if any)
		struct Location final {
			//! Reset the location from a given atom
			void resetFromAtom(const ny::Atom&);

			//! Current target
			YString target;
			//! Current filename
			YString filename;

			//! Current position
			struct {
				//! Current line (1-based, otherwise disabled)
				uint line = 0;
				//! Current offset (1-based, otherwise disabled)
				uint offset = 0;
				//! Current offset (1-based, otherwise disabled)
				uint offsetEnd = 0;
			}
			pos;
		}
		location;
	}
	origins;

	//! Flag to remember if some errors or warning have occured or not
	bool hasErrors = false;

}; // class Message


} // namespace Logs
} // namespace ny
