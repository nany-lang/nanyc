#pragma once
#include <yuni/yuni.h>
#include "details/compiler/compdb.h"
#include <cassert>
#include <memory>


namespace ny {

//! Nany Program (result of the compilation)
class Program final {
public:
	static auto* pointer(nyprogram_t* program) { return reinterpret_cast<Program*>(program); }
	static auto* pointer(Program& program) { return reinterpret_cast<nyprogram_t*>(&program); }
	static auto* pointer(const Program& program) { return reinterpret_cast<const nyprogram_t*>(&program); }
	static auto* pointer(Program* program) { return reinterpret_cast<nyprogram_t*>(program); }
	static auto* pointer(const Program* program) { return reinterpret_cast<const nyprogram_t*>(program); }
	static auto& ref(nyprogram_t* program) { assert(program); return *pointer(program); }

	std::unique_ptr<ny::compiler::Compdb> compdb;
};

} // namespace ny
