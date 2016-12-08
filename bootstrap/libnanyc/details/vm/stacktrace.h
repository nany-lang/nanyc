#pragma once
#include "libnanyc.h"
#include "details/atom/atom-map.h"
#include "details/context/build.h"


namespace ny {
namespace vm {


template<bool Enabled>
struct Stacktrace final {
	static void push(uint32_t, uint32_t) {}
	static void pop() {}
	static void dump(Build&) {}
};


template<>
struct Stacktrace<true> final {
	union Frame {
		uint32_t atomidInstance[2]; // atom id / instance id
		uint64_t u64;
	};

	Stacktrace();
	Stacktrace(const Stacktrace&) = delete;
	~Stacktrace();

	void push(uint32_t atomid, uint32_t instanceid);
	void pop() noexcept;
	void dump(Build&, const AtomMap&) const;

	Stacktrace& operator = (const Stacktrace&) = delete;


private:
	void grow();

	Frame* topframe = nullptr;
	Frame* upperLimit = nullptr;
	Frame* baseframe = nullptr;

}; // class Stacktrace


} // namespace vm
} // namespace ny

#include "stacktrace.hxx"
