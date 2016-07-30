#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include "nany/nany.h"
#include <memory>


namespace Nany
{

	class Project;
	class IntrinsicTable;


	/*!
	** \brief Nany Standard Library 'std.core'
	*/
	void importNSLCore(Project&);

	/*!
	** \brief Import intrinsics related to string manipulation
	*/
	void importNSLCoreString(IntrinsicTable&);

	/*!
	** \brief Import intrinsics related to process manipulation
	*/
	void importNSLOSProcess(IntrinsicTable&);

	/*!
	** \brief Import intrinsics related to environment variables manipulation
	*/
	void importNSLEnv(IntrinsicTable&);



} // namespace Nany
