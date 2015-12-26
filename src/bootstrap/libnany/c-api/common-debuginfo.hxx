#pragma once
#include <yuni/yuni.h>
#include <yuni/core/system/memory.h>
#include <yuni/core/system/cpu.h>

using namespace Yuni;



namespace // anonymous
{


	template<class T>
	static inline void nany_details_export_memory_usage(T& stream)
	{
		System::Memory::Usage usage;
		auto total = usage.total / (1024u * 1024u * 1024u);
		auto avail = Math::Round((double) usage.available / (1024u * 1024u * 1024u), 2);
		ShortString64 availstr = avail;
		availstr.trimRight('0');

		stream << "mem: " << availstr << " GiB free / " << total << " GiB";
	}


	template<class T>
	static inline void nany_details_export_system(T& stream)
	{
		stream << YUNI_OS_NAME;
		#ifdef YUNI_OS_64
		stream << " (64bits)";
		#else
		stream << " (32bits)";
		#endif
	}


	template<class T>
	static inline void nany_details_export_compiler_version(T& stream)
	{
		stream << YUNI_COMPILER_NAME;

		#ifdef __VERSION__
		{
			stream << ' ' << __VERSION__;
		}
		#else
		{
			#ifdef _MSC_FULL_VER
			stream << ' ' << _MSC_FULL_VER;
			#endif
			#ifdef __clang__
			stream << ' ' << __clang_version__;
			#else
			# ifdef __GNUC__
			stream << ' ' << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__;
			# endif
			#endif

			#ifdef __xlc__
			stream << ' ' << __xlc__;
			#endif
		}
		#endif
	}



} // anonymous namespace
