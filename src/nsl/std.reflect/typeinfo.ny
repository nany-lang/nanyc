// Nany - https://nany.io
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace std.reflection;


public class Typeinfo<:T:> {
	//! Classname (ex: 'File' from `std.io.File`)
	var name
		-> new string(!!__nanyc_reflect_name(#[__nanyc_synthetic] T));

	//! Symbol (ex: 'std.io.File')
	var classname
		-> new string(!!__nanyc_reflect_classname(#[__nanyc_synthetic] T));

	//! Keyword ('class', 'func', 'operator', 'view'...)
	var keyword
		-> new string(!!__nanyc_reflect_keyword(#[__nanyc_synthetic] T));

	//! Get if the type is a class
	var isClass
		-> new bool(!!__nanyc_reflect_is_class(#[__nanyc_synthetic] T));

	//! Get if the type is a function
	var isFunc
		-> new bool(!!__nanyc_reflect_is_func(#[__nanyc_synthetic] T));

	//! Get if the type is a variable
	var isVar
		-> new bool(!!__nanyc_reflect_is_var(#[__nanyc_synthetic] T));

	//! Get if the type is a var
	var isTypedef
		-> new bool(!!__nanyc_reflect_is_typedef(#[__nanyc_synthetic] T));

	//! Get if the type is a view
	var isView
		-> new bool(!!__nanyc_reflect_is_view(#[__nanyc_synthetic] T));

	//! Get if the type is an operator
	var isOperator
		-> new bool(!!__nanyc_reflect_is_operator(#[__nanyc_synthetic] T));

	//! Get if the type is a constructor
	var ctor
		-> new bool(!!__nanyc_reflect_is_ctor(#[__nanyc_synthetic] T));

	//! Get if the type is a destructor
	var dtor
		-> new bool(!!__nanyc_reflect_is_dtor(#[__nanyc_synthetic] T));

	var scope
		-> isFunc or isClass;

	//! Get if the type is callable (functor)
	var callable
		-> new bool(!!__nanyc_reflect_callable(#[__nanyc_synthetic] T));

	//! Get if the function or the class has been declared without an accessible name)
	var anonymous
		-> new bool(!!__nanyc_reflect_anonymous(#[__nanyc_synthetic] T));

	//! Get the amount of memory that an instance of the class would occupy
	var bytes
		-> new u32(!!__nanyc_reflect_sizeof(#[__nanyc_synthetic] T));

	var source -> new class {
		//! Filename where the type is defined
		var filename
			-> new string(!!__nanyc_reflect_filename(#[__nanyc_synthetic] T));

		//! Line (1-based) in the file where the type is defined (0 if undefined)
		var line
			-> new u32(!!__nanyc_reflect_line(#[__nanyc_synthetic] T));

		//! Column (1-based) in the file where the type is defined (0 if undefined)
		var column
			-> new u32(!!__nanyc_reflect_column(#[__nanyc_synthetic] T));

		func toString(): ref string {
			var out = filename;
			if line != 0u then {
				out << ':' << line;
				if column != 0u then
					out << ':' << column;
			}
			return out;
		}
	};

	var properties -> new class {
		//! The total number of properties (independent from the visibility)
		var count
			-> new u32(!!__nanyc_reflect_props_count(#[__nanyc_synthetic] T));
	};

	var funcs -> new class {
		//! The total number of functions (independent from the visibility)
		var count
			-> new u32(!!__nanyc_reflect_funcs_count(#[__nanyc_synthetic] T));

		func find(cref name: string, cref callback) {
			(new Typeinfo<:T:>).foreach(func (cref typeinfo) {
				if typeinfo.name == name then
					callback(typeinfo);
			});
		}
	};

	var vars -> new class {
		//! The total number of variables (independent from the visibility)
		var count
			-> new u32(!!__nanyc_reflect_vars_count(#[__nanyc_synthetic] T));
	};

	func foreach(cref callback)
		-> !!__nanyc_reflect_foreach(#[__nanyc_synthetic] T, callback);

	func toString(): ref
		-> toString(0u);

	func toString(indent: u32): ref string {
		var spaces = new string(indent * 4u, pattern: ' ');
		var out = "";
		out << spaces << "// " << source.toString() << '\n';
		out << spaces << keyword << ' ' << classname;
		if isClass then {
			out << " {";
			out << " // " << classname;
			out << '\n';
			var tab = "    ";
			if isClass then {
				out << spaces << tab << "// sizeof: " << bytes << " bytes\n";
				out << spaces << tab << "// props count: " << properties.count << '\n';
			}
			out << spaces << tab << "// funcs count: " << funcs.count << '\n';
			out << spaces << tab << "// vars count:  " << vars.count << '\n';
			out << '\n';
			foreach(func (cref typeinfo) {
				out << spaces << tab << typeinfo.classname << '\n';
				//out << spaces << tab << typeinfo.toString(indent + 1u) << '\n';
			});
			out << spaces << '}';
		}
		return out;
	}

} // class Typeinfo<:T:>


func call<:T:>(ref callable)
	-> callable(new Typeinfo<:T:>);
