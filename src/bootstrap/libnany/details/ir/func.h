#pragma once
#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include "details/ir/segment.h"




namespace Nany
{
namespace IR
{


	class Func final
	{
	public:
		//! Attached segment
		Segment segment;

	}; // class Func







} // namespace IR
} // namespace Nany

#include "segment.hxx"
