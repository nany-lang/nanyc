#pragma once
#include <yuni/yuni.h>
#include <memory>
#include <limits>
#include "nany/memalloc.h"

namespace ny::Memory {

//! C++ memory allocator based on the user-defined allocator
template<class T>
class Allocator final {
public:
	// STL compliance

	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef std::true_type propagate_on_container_move_assignment;
	template<class U> struct rebind {
		typedef Allocator<U> other;
	};
	typedef std::true_type is_always_equal;


public:
	Allocator(nyallocator_t& allocator) noexcept
		: allocator(allocator) {
	}

	Allocator(const Allocator&) noexcept = default;

	static pointer address(reference x) noexcept {
		return &x;
	}
	static const_pointer address(const_reference x) noexcept {
		return &x;
	}

	pointer allocate(size_type n, const_pointer /*hint*/ = nullptr) {
		pointer p = (pointer) allocator.allocate(&allocator, n * sizeof(T));
		if (YUNI_UNLIKELY(!p))
			throw std::bad_alloc();
		return p;
	}

	void deallocate(pointer p, size_type n) noexcept {
		allocator.deallocate(&allocator, p, n * sizeof(T));
	}

	static size_type max_size() noexcept {
		return std::template numeric_limits<size_type>::max();
	}

	template<class U, class... Args>
	void construct(U* p, Args&& ... args) {
		::new((void*) p) U(std::forward<Args>(args)...);
	}

	template<class U> void destroy(U* p) {
		p->~U();
	}

	template<class U> bool operator == (const Allocator<U>& rhs) const noexcept {
		return &allocator == &rhs.allocator;
	}

	template<class U> bool operator != (const Allocator<U>& rhs) const noexcept {
		return &allocator != &rhs.allocator;
	}

private:
	//! User context
	nyallocator_t& allocator;

}; // class Allocator

} // ny::Memory
