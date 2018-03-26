// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std;

public class InvalidPointer {
}

public class Optional<:T:> {
	operator new;

	operator new(ref other: Optional<:T:>) {
		if (other.m_object != null) {
			m_object = !!pointer(other.m_object);
			ref object = !!__reinterpret(m_object, #[__nanyc_synthetic] T);
			!!ref(object);
		}
		else
			m_object = null;
	}

	operator new(ref object: T) {
		!!ref(object);
		m_object = !!pointer(object);
	}

	operator dispose {
		if m_object != null then
			!!unref(!!__reinterpret(m_object, #[__nanyc_synthetic] T));
	}

	func acquire(ref object: T) {
		if m_object != null then
			!!unref(!!__reinterpret(m_object, #[__nanyc_synthetic] T));
		!!ref(object);
		m_object = !!pointer(object);
	}

	func reset {
		if m_object != null then {
			!!unref(!!__reinterpret(m_object, #[__nanyc_synthetic] T));
			m_object = null;
		}
	}

	func deref: ref {
		if (m_object == null)
			raise new std.InvalidPointer;
		return !!__reinterpret(m_object, #[__nanyc_synthetic] T);
	}

	var value -> {get: deref(), set: acquire(value)};

	var empty
		-> m_object == null;

private:
	var m_object = null;
}

public func optional<:T:>: ref {
	return new std.Optional<:T:>;
}

public func optional(ref object): ref {
	return new std.Optional<:typeof(object):>(object);
}
