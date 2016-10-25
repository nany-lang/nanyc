// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// \brief   String implementation
/// \ingroup std.core

namespace std;


public class Array<:T:> {
	operator new;

	operator dispose {
		if m_size != 0__u32 then
			doClear();
		var sizeof = !!sizeof(#[__nanyc_synthetic] T);
		std.memory.dispose(m_items, sizeof * m_capacity);
	}

	func append(cref element) {
		var size = new u32(m_size);
		var newsize = size + 1u;
		if m_capacity < newsize then
			doGrow(newsize);
		m_size = newsize.pod;
		// create the new element and inc its ref count to keep it alive
		ref newelem = new T(element);
		!!ref(newelem);
		var ptr = m_items + size.pod * !!sizeof(#[__nanyc_synthetic] T);
		!!store.ptr(ptr, !!pointer(newelem));
	}

	/*
	func append(cref array: std.Array<:T:>)
	{
		var arrsize = array.size;
		if arrsize != 0u then
		{
			if m_capacity < arrsize then
				doGrow(size);
			var i = 0u;
			do
			{
				append(array.at(i));
			}
			while (i += 1u) != arrsize;
		}
	}
	*/

	//! Increase the capacity of the container if necessary
	func reserve(size: u32) {
		if m_capacity < size then
			doGrow(size);
	}

	func clear {
		if m_size != 0u then
			doClear();
	}

	func shrink {
		if m_size == 0u then {
			var sizeof = !!sizeof(#[__nanyc_synthetic] T);
			std.memory.dispose(m_items, sizeof * m_capacity);
			m_capacity = 0__u32;
			m_items = null;
		}
	}

	//! Remove the last element
	func pop {
		if m_size != 0u then {
			var size = m_size - 1u;
			assert(size != 0u);
			m_size = size.pod;
			var ptr = new pointer<:T:>(m_items, size.pod);
			var addr = !!load.ptr(ptr.m_ptr);
			!!unref(!!__reinterpret(addr, #[__nanyc_synthetic] T));
		}
	}


	func contains(cref element): bool
		-> (i in self | i == element).cursor().findFirst();

	func at(i: u32): ref {
		assert(i < m_size);
		var ptr = m_items + i.pod * !!sizeof(#[__nanyc_synthetic] T);
		return !!__reinterpret(!!load.ptr(ptr), #[__nanyc_synthetic] T);
	}

	operator [] (i: u32): ref {
		if (i < m_size) then {
			var ptr = m_items + i.pod * !!sizeof(#[__nanyc_synthetic] T);
			return !!__reinterpret(!!load.ptr(ptr), #[__nanyc_synthetic] T);
		}
		return new T;
	}

	//! Append an new element
	operator += (cref element): ref {
		append(element);
		return self;
	}

	//! Number of items in the array
	var size
		-> new u32(m_size);

	//! Capacity (in elements)
	var capacity
		-> new u32(m_capacity);

	//! Get if the array is empty
	var empty
		-> new bool(m_size == 0__u32);

	var first
		-> at(0u); // TODO use a safer function

	var last
		-> at(new u32(m_size) - 1u); // TODO use a safer function


	view (cref filter): ref {
		ref m_parentArray = self;
		ref m_parentFilter = filter;
		return new class {
			//! Empty view ? (without the predicate)
			func empty -> m_parentArray.empty();

			func cursor: any {
				ref accept = m_parentFilter;
				ref originalArray = m_parentArray;
				return new class {
					func findFirst: bool
						-> (not originalArray.empty) and (accept(originalArray.at(0u)) or next());

					func next: bool {
						do {
							m_index += 1u;
							if not (m_index < originalArray.size) then
								return false;
						}
						while not accept(originalArray.at(m_index));
						return true;
					}

					func get: ref -> originalArray.at(m_index);

					func reset {
						m_index = 0u;
					}

					var m_index = 0u;
				};
			}

			view (cref filter): ref
				-> i in m_parentArray | m_parentFilter(i) and filter(i);
		};
	}


private:
	//! Increase the inner storage
	func doGrow(newsize: u32) {
		var oldcapa = new u32(m_capacity);
		var newcapa = oldcapa;
		do {
			newcapa = (if newcapa < 64u then 64u
				else (if newcapa < 4096u then newcapa * 2u else newcapa += 4096u));
		}
		while newcapa < newsize;

		var sizeof = !!sizeof(#[__nanyc_synthetic] T);
		if true then {
			// T is ref
			m_capacity = newcapa.pod;
			m_items = std.memory.reallocate(m_items, (0u64 + oldcapa) * sizeof, (0u64 + newcapa) * sizeof);
		}
		else {
			// T is an object stored inside the array - create a new array
			var newItems = std.memory.allocate(0u64 + newcapa * sizeof);

			// release the old ones
			var size = new u32(m_size);
			if size != 0u then
			{
				var oldItems = m_items;
				var i = size;
				var oldP = new pointer<:T:>(oldItems);
				var newP = new pointer<:T:>(newItems);
				do {
					var oldAddr = !!load.ptr(oldP.m_ptr);
					ref oldItem = !!__reinterpret(oldAddr, #[__nanyc_synthetic] T);

					// copying
					ref newelem = new T(oldItem);
					!!ref(newelem);
					!!store.ptr(newP.m_ptr, !!pointer(newelem));

					// deleting the old one
					!!unref(oldItem);

					oldP += 1u;
					newP += 1u;
				}
				while (i -= 1u) != 0u;

				// delete the old array
				std.memory.dispose(oldItems, sizeof * oldcapa);
			}

			m_capacity = newcapa.pod;
			m_items = newItems;
		}
	}

	func doClear {
		var size  = m_size;
		m_size = 0__u32;
		var i = new u32(size);
		var ptr = new pointer<:T:>(m_items);
		do {
			var addr = !!load.ptr(ptr.m_ptr);
			!!unref(!!__reinterpret(addr, #[__nanyc_synthetic] T));
			ptr += 1u;
		}
		while (i -= 1u) != 0u;
	}


internal:
	// note: u32 by default to have a consistent behavior accross all operating systems
	// note: currently `__u32` instead of `u32` due to the lack of optimizations by the compiler
	//! The current size of the container
	var m_size = 0__u32;
	//! The current capacity of the container
	var m_capacity = 0__u32;
	//! Contents, not zero-terminated
	var m_items: __pointer = null;

} // class Array
