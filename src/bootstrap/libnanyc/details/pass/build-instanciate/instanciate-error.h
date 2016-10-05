#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include "details/atom/classdef-table-view.h"
#include "details/ir/isa/data.h"
#include "details/ir/sequence.h"
#include "details/errors/errors.h"
#include "stack-frame.h"




namespace Nany
{
namespace Pass
{
namespace Instanciate
{
namespace complain
{


	//! Invalid classdef (ICE)
	bool classdef(const Classdef&, const char* usertxt);




} // namespace complain
} // namespace Instanciate
} // namespace Pass
} // namespace Nany
