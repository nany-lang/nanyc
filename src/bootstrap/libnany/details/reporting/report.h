#pragma once
#include <yuni/yuni.h>
#include "fwd.h"
#include "message.h"
#include "levels.h"



namespace Nany
{
namespace Logs
{


	class EmptyReport;



	class Report final
	{
	public:
		//! \name Constructors
		//@{
		explicit Report(Message& message);
		Report(Report& report);
		Report(Report&&);
		//@}

		//! \name Messages
		//@{
		//! Create ICE
		Report ICE();
		//! Create a new error message
		Report error();
		//! Create a new warning message
		Report warning();
		//! Create a new hint message
		Report hint();
		//! Create a new suggest message
		Report suggest();
		//! Create a new success message
		Report success();
		//! Create a new info message
		Report info();
		//! Create a new info message
		Report info(AnyString prefix);
		//! Create a new trace message
		Report verbose();
		//! Create a new trace message
		Report trace();

		//! Create a dummy message, for sub-grouping
		Report subgroup();

		void appendEntry(const Message::Ptr&);
		//@}


		//! \name Utilities
		//@{
		//! Get if there are some error messages
		bool hasErrors() const;

		YString& text();
		const YString& text() const;

		//! Get access to the message itself (used by the C-API)
		Message& data();

		Message::Origin& origins();
		//@}


		//! \name Operators
		//@{
		template<class T> Report& operator << (const T& value);
		//! Copy operator
		Report& operator = (const Report&) = delete;
		//! bool
		operator bool () const;
		//@}


	public:
		Message& message;

	}; // class Report





	class EmptyReport final
	{
	public:
		bool hasErrors() const;

		template<class T> EmptyReport& operator << (const T& value);
	};




} // namespace Logs
} // namespace Nany

#include "report.hxx"
