#pragma once
#include <yuni/yuni.h>
#include "classdef.h"
#include "details/utils/clid.h"
#include <vector>
#include <memory>
#include "details/atom/ctype.h"

namespace ny {

struct Atom;

class Signature final {
public:
	struct Paramtype final {
		//! Kind of the parameter
		CType kind = CType::t_any;
		//! Atom attached to it, if any (kind == nyt_any)
		Atom* atom = nullptr;
		//! Qualifiers (ref, const...)
		Qualifiers qualifiers;

		bool operator == (const Paramtype& rhs) const;
		bool operator != (const Paramtype& rhs) const;
	};

	struct Parameters final {
		//! Reserve the number of parameters
		void resize(uint count);
		//! The total number of parameters
		uint size() const;
		//! Get if empty
		bool empty() const;

		const Paramtype& operator [] (uint32_t index) const;
		Paramtype& operator [] (uint32_t index);


		bool operator == (const Parameters& rhs) const;

	private:
		friend class Signature;
		void hash(size_t&) const;
		//! Each type for each parameter
		std::vector<Paramtype> m_types;
	};


public:
	//! Get the hash of this singature
	size_t hash() const;

	bool operator == (const Signature& rhs) const;
	bool operator != (const Signature& rhs) const;

public:
	//! Function parameters (func (a, b , c))
	Parameters parameters;
	//! Template parameters
	Parameters tmplparams;

}; // class Signature

} // namespace ny

#include "signature.hxx"
