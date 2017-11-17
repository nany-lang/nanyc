#pragma once
#include <yuni/yuni.h>
#include <cstdint>

namespace ny::ir {

//! Constant for declaring classdef with several overloads
static constexpr yuint64 kClassdefHasOverloads = (yuint64) - 1;

//! Magic dust for blueprints
static constexpr yuint64 blueprintMagicDust = 0x123456789ABCDEF;

} // ny::ir

namespace ny::ir::Producer {

struct Scope;
struct Context;

} // ny::ir::Producer
