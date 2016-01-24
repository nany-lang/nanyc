#pragma once


namespace Nany
{

	class Context;
	class IntrinsicTable;



	/*!
	** \brief Nany Standard Library 'std.core'
	*/
	void importNSLCore(Context&);

	/*!
	** \brief Import intrinsics related to string manipulation
	*/
	void importNSLCoreString(IntrinsicTable&);

	/*!
	** \brief Import intrinsics related to process manipulation
	*/
	void importNSLOSProcess(IntrinsicTable&);

} // namespace Nany
