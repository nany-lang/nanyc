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
	** \brief Nany Standard Library 'std.memory'
	*/
	void importNSLMemory(IntrinsicTable&);

	/*!
	** \brief Import intrinsics related to IO accesses
	*/
	void importNSLIO(IntrinsicTable&);

	/*!
	** \brief Import intrinsics related to process manipulation
	*/
	void importNSLOSProcess(IntrinsicTable&);

	/*!
	** \brief Import intrinsics related to environment variables manipulation
	*/
	void importNSLEnv(IntrinsicTable&);

	/*!
	** \brief Import intrinsics related to console management
	*/
	void importNSLConsole(IntrinsicTable&);


} // namespace Nany
