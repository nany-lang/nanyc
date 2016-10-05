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


	//! Unknown intrinsic
	bool unknownIntrinsic(const AnyString& name);


	//! Class not instanciated (not ready for use)
	bool classNotInstanciated(const Atom&);


	//! A class is required
	bool classRequired();


	//! Failed to allocate class object (null atom, due to previous error)
	bool canNotAllocateClassNullAtom(const Classdef&, uint32_t lvid);


	//! Invalid Self (not a class)
	bool invalidClassSelf(const AnyString& identifier);


	//! No property found
	bool noproperty(const AnyString& identifier);


	//! Ambigous property call
	bool ambigousPropertyCall(const AnyString& identifier);




} // namespace complain
} // namespace Instanciate
} // namespace Pass
} // namespace Nany
