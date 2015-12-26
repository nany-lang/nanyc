#pragma once
#include <yuni/yuni.h>
#include <yuni/core/smartptr/intrusive.h>
#include <yuni/core/string.h>
#include <yuni/core/noncopyable.h>
#include "fwd.h"
#include "levels.h"
#include <iosfwd>



namespace Nany { class Node; }

namespace Nany
{
namespace Logs
{

	class Message final
		: public Yuni::IIntrusiveSmartPtr<Message, false, Yuni::Policy::ObjectLevelLockable>
		, Yuni::NonCopyable<Message>
	{
	public:
		//! The class ancestor
		typedef Yuni::IIntrusiveSmartPtr<Message, false, Yuni::Policy::ObjectLevelLockable>  Ancestor;
		//! The most suitable smart ptr for the class
		typedef Ancestor::SmartPtrType<Message>::Ptr  Ptr;
		//! Threading policy
		typedef Ancestor::ThreadingPolicy ThreadingPolicy;


	public:
		Message(Level level)
			: level(level)
		{}

		//! Create a new sub-entry
		Message& createEntry(Level level);

		void appendEntry(const Message::Ptr& message);

		void print(std::ostream&) const;
		void print(Yuni::Clob&) const;

		bool isClassifiedAsError() const
		{
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
		struct Origin final
		{
			//! Current filename (if any)
			struct
			{
				//! Current target
				YString target;
				//! Current filename
				YString filename;

				//! Current position
				struct
				{
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
} // namespace Nany
