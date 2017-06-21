#pragma once
#include "libnanyc.h"
#include <yuni/core/string.h>
#include <yuni/core/noncopyable.h>
#include <yuni/thread/mutex.h>
#include "levels.h"
#include <iosfwd>
#include <memory>


namespace ny { struct Atom; }


namespace ny {
namespace Logs {


struct Message final: Yuni::NonCopyable<Message> {
	Message(Level level)
		: level(level) {
	}

	//! Create a new sub-entry
	Message& createEntry(Level level);

	void appendEntry(std::unique_ptr<Message>& message);

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
	std::vector<std::shared_ptr<Message>> entries;

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
	mutable yuni::Mutex m_mutex;

}; // struct Message


} // namespace Logs
} // namespace ny
