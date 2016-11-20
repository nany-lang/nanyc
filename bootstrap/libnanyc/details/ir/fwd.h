#pragma once
#include <yuni/yuni.h>
#include <cstdint>



namespace ny
{
namespace IR
{

	/*!
	** \brief Constant for declaring classdef with several overloads
	*/
	static constexpr yuint64 kClassdefHasOverloads = (yuint64) -1;

	/*!
	** \brief Magic dust for blueprints
	*/
	static constexpr yuint64 blueprintMagicDust = 0x123456789ABCDEF;



	class Sequence;
	class ClassdefRef;




} // namespace IR
} // namespace ny


namespace ny
{
namespace IR
{
namespace Producer
{

	class Scope;
	class Context;


} // namespace Producer
} // namespace IR
} // namespace ny
