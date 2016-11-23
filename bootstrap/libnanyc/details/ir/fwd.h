#pragma once
#include <yuni/yuni.h>
#include <cstdint>



namespace ny {
namespace ir {

/*!
** \brief Constant for declaring classdef with several overloads
*/
static constexpr yuint64 kClassdefHasOverloads = (yuint64) - 1;

/*!
** \brief Magic dust for blueprints
*/
static constexpr yuint64 blueprintMagicDust = 0x123456789ABCDEF;



class ClassdefRef;




} // namespace ir
} // namespace ny


namespace ny {
namespace ir {
namespace Producer {

class Scope;
class Context;


} // namespace Producer
} // namespace ir
} // namespace ny
