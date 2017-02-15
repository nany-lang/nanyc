// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std;


public func optional<:T:>()
	-> new std.Optional<:T:>;


public func optional(ref object)
	-> new std.Optional<:typeof(object):>(object);


public class Optional<:T:> {

	operator new;

	operator new(ref object: T) {
		m_object = !!pointer(object);
		!!ref(object);
	}

	operator new(cref other: std.Optional<:T:>) {
		if other.valid then {
			m_object = !!pointer(other.object);
			!!ref(m_object);
		}
	}

	operator clone(cref other: std.Optional<:T:>) {
		if other.valid then {
			m_object = !!pointer(other.object);
			!!ref(m_object);
		}
	}

	operator dispose {
		if m_object != null then
			!!unref(!!__reinterpret(m_object, #[__nanyc_synthetic] T));
	}

	var deref -> {get: get(), set: acquire(value)};

	var valid
		-> m_object != null;

	func acquire(ref object: T) {
		if m_object != null then
			!!unref(!!__reinterpret(m_object, #[__nanyc_synthetic] T));
		!!ref(object);
		m_object = !!pointer(object);
	}

	func acquire(cref other: std.Optional<:T:>) {
		if m_object != null then
			!!unref(!!__reinterpret(m_object, #[__nanyc_synthetic] T));
		if other.valid then {
			!!ref(other.m_object);
			m_object = !!pointer(other.object);
		}
	}

	func release {
		if m_object != null then {
			!!unref(!!__reinterpret(m_object, #[__nanyc_synthetic] T));
			m_object = null;
		}
	}

	func get: ref T {
		assert(m_object != null);
		return !!__reinterpret(m_object, #[__nanyc_synthetic] T);
	}

private:
	var m_object = null;
}
