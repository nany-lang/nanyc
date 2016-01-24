#pragma once
#include <yuni/yuni.h>
#include <yuni/string.h>
#include <vector>
#include "details/atom/atom-map.h"



namespace Nany
{
namespace VM
{

	template<bool Enabled>
	struct Stacktrace final
	{
		static void push(uint32_t, uint32_t) {}
		static void pop() {}
		static void dump(nycontext_t&) {}
	};



	template<>
	struct Stacktrace<true> final
	{
		union Frame
		{
			//! atom id / instance id
			uint32_t atomidInstance[2];
			//! raw value
			uint64_t u64;
		};

	public:
		//! Default constructor
		Stacktrace()
		{
			baseframe  = (Frame*)::malloc(sizeof(Frame) * 64);
			if (YUNI_UNLIKELY(nullptr == baseframe))
				throw std::bad_alloc();
			upperLimit = baseframe + 64;
			topframe   = baseframe;
		}

		//! deleted Copy constructor
		Stacktrace(const Stacktrace&) = delete;

		//! Destructor
		~Stacktrace()
		{
			::free(baseframe);
		}


		//! Register a new stack frame
		void push(uint32_t atomid, uint32_t instanceid)
		{
			if (YUNI_UNLIKELY(not (++topframe < upperLimit)))
				grow();
			*topframe = Frame{{atomid, instanceid}};
		}


		//! Remove the last stack frame
		void pop()
		{
			assert(topframe > baseframe);
			--topframe;
		}


		//! Export the whole stack to a string
		void dump(nycontext_t&, const AtomMap&) const;

		//! deleted Operator assignment
		Stacktrace& operator = (const Stacktrace&) = delete;


	private:
		//! Increase the capacity of the stack
		void grow();

	private:
		//! The current frame
		Frame* topframe = nullptr;
		//! The upper frame limit
		Frame* upperLimit = nullptr;
		//! The base frame
		Frame* baseframe = nullptr;

	}; // class Stacktrace





} // namespace VM
} // namespace Nany
