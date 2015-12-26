#include "nany/nany.h"
#include <yuni/io/file.h>
#include <yuni/core/system/cpu.h>
#include <yuni/core/system/memory.h>
#include <yuni/core/math.h>
#include "details/context/context.h"
#include "details/reporting/report.h"
#include "libnany-config.h"
#include "common-debuginfo.hxx"
#include <iostream>

using namespace Yuni;





extern "C" nyreport_t*  nany_report_create()
{
	try
	{
		auto* message = new Nany::Logs::Message{Nany::Logs::Level::none};
		message->addRef();
		return (nyreport_t*) message;
	}
	catch (...) {}
	return nullptr;
}


extern "C" void nany_report_ref(nyreport_t* report)
{
	if (report)
		reinterpret_cast<Nany::Logs::Message*>(report)->addRef();
}


extern "C" void nany_report_unref(nyreport_t** report)
{
	if (report and *report)
	{
		try
		{
			auto* message = reinterpret_cast<Nany::Logs::Message*>(*report);
			if (message->release())
				delete message;
		}
		catch (...) {}
		*report = nullptr;
	}
}


extern "C" nybool_t nany_report_print_stdout(const nyreport_t* rptr)
{
	if (rptr)
	{
		try
		{
			auto& message = *(reinterpret_cast<const Nany::Logs::Message*>(rptr));
			message.print(std::cout);
			return nytrue;
		}
		catch (...) {}
	}
	return nyfalse;
}


extern "C" nybool_t nany_report_print_stderr(const nyreport_t* rptr)
{
	if (rptr)
	{
		try
		{
			auto& message = *(reinterpret_cast<const Nany::Logs::Message*>(rptr));
			message.print(std::cerr);
			return nytrue;
		}
		catch (...) {}
	}
	return nyfalse;
}



extern "C" nybool_t nany_report_add_compiler_headerinfo(nyreport_t* rptr)
{
	if (rptr)
	{
		try
		{
			auto& message = *(reinterpret_cast<Nany::Logs::Message*>(rptr));
			Nany::Logs::Report report{message};

			auto r = report.info();
			r.message.section = "project";
			r.message.prefix = "nanyc {c++/bootstrap} ";
			r << 'v' << LIBNANY_VERSION_STR;

			if (debugmode)
				r << " {debug}";
			return nytrue;
		}
		catch (...) {}
	}
	return nyfalse;
}




extern "C" void nany_info(nyreport_t* rptr, const char* text)
{
	if (rptr and text)
	{
		try
		{
			auto& message = *(reinterpret_cast<Nany::Logs::Message*>(rptr));
			Nany::Logs::Report{message}.info() << text;
		}
		catch (...) {}
	}
}


extern "C" void nany_warning(nyreport_t* rptr, const char* text)
{
	if (rptr and text)
	{
		try
		{
			auto& message = *(reinterpret_cast<Nany::Logs::Message*>(rptr));
			Nany::Logs::Report{message}.warning() << text;
		}
		catch (...) {}
	}
}


extern "C" void nany_error(nyreport_t* rptr, const char* text)
{
	if (rptr and text)
	{
		try
		{
			auto& message = *(reinterpret_cast<Nany::Logs::Message*>(rptr));
			Nany::Logs::Report{message}.error() << text;
		}
		catch (...) {}
	}
}
