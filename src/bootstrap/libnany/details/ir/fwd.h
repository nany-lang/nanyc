#pragma once
#include <yuni/yuni.h>
#include <cstdint>



namespace Nany
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



	class Program;
	class Atom;
	class ClassdefRef;




} // namespace IR
} // namespace Nany


namespace Nany
{
namespace IR
{
namespace Producer
{

	class Scope;
	class Context;


} // namespace Producer
} // namespace IR
} // namespace Nany
