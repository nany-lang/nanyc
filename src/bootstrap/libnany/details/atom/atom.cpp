#include "atom.h"
#include "details/ir/sequence.h"
#include "details/atom/classdef-table.h"
#include "details/atom/classdef-table-view.h"
#include "details/reporting/report.h"
#include "details/errors/errors.h"

using namespace Yuni;





namespace Nany
{

	namespace // anonymous
	{

		template<class OutT, class ListT, class TableT>
		static void atomParametersPrinter(OutT& out, ListT& list, const TableT* table, bool avoidSelf, AnyString sepBefore, AnyString sepAfter)
		{
			bool first = true;
			out << sepBefore;
			list.each([&](uint32_t i, const AnyString& paramname, const Vardef& vardef)
			{
				// avoid the first virtual parameter
				if (avoidSelf and i == 0 and paramname == "self")
					return;

				if (not first)
					out << ", ";
				first = false;
				out << paramname;

				if (table)
				{
					if (table) // and table->hasClassdef(vardef.clid))
					{
						auto& retcdef = table->classdef(vardef.clid);
						if (not retcdef.isVoid())
						{
							out << ": ";
							retcdef.print(out, *table, false);
						}
					}
				}
				else
				{
					if (not vardef.clid.isVoid())
						out << ": any";
				}
			});
			out << sepAfter;
		}

	} // anonymous namespace






	Atom::Atom(const AnyString& name, Atom::Type type)
		: type(type)
		, name(name)
	{}


	Atom::Atom(Atom& rootparent, const AnyString& name, Atom::Type type)
		: type(type)
		, parent(&rootparent)
		, name(name)
	{
		rootparent.pChildren.insert(std::pair<AnyString, Atom::Ptr>(name, this));
	}


	Atom::~Atom()
	{
		assert(instances.size() == pInstancesMD.size());
		if (opcodes.owned)
			delete opcodes.sequence;
	}


	bool Atom::canAccessTo(const Atom& atom) const
	{
		if (atom.isNamespace()) // all namespaces are accessble
			return true;

		if (this == &atom) // same...
			return true;

		// belonging to a class ?
		if (parent and parent == atom.parent and isClassMember())
			return true;

		// get if the targets are identical
		if (origin.target == atom.origin.target)
		{
			// determining whether the current atom is an ancestor of the provided one
			// TODO fix accessibility (with visibility)
			return true;
		}
		else
		{
			// not on the same target ? Only public elements are accessible
			return atom.isPublicOrPublished();
		}
		return false; // fallback
	}


	void Atom::retrieveFullname(Yuni::String& out, const ClassdefTableView* table) const
	{
		if (parent)
		{
			if (parent->parent)
			{
				parent->retrieveFullname(out, table);
				out += '.';
			}

			switch (type)
			{
				case Type::funcdef:
				{
					if (name.first() != '^')
					{
						out += name;
					}
					else
					{
						if (name.startsWith("^default-var-%"))
						{
							AnyString sub{name.c_str() + 14, name.size() - 14};
							auto ix = sub.find('-');

							if (ix < sub.size())
							{
								++ix;
								AnyString varname{sub.c_str() + ix, sub.size() - ix};
								out << varname;
							}
							else
								out << "<invalid-field>";
						}
						else if (name.startsWith("^view^"))
						{
							out += ':';
							out.append(name.c_str() + 6, name.size() - 6);
						}
						else
							out.append(name.c_str() + 1, name.size() - 1);
					}

					if (not tmplparamsForPrinting.empty())
						atomParametersPrinter(out, tmplparamsForPrinting, table, false, "<:", ":>");
					break;
				}

				case Type::classdef:
				{
					if (name.first() != '^')
						out += name;
					else
						out.append(name.c_str() + 1, name.size() - 1);

					if (not tmplparamsForPrinting.empty())
						atomParametersPrinter(out, tmplparamsForPrinting, table, false, "<:", ":>");
					break;
				}

				case Type::namespacedef:
				case Type::typealias:
				case Type::vardef:
				case Type::unit:
				{
					out += name;
					break;
				}
			}
		}
	}


	YString Atom::fullname() const
	{
		YString out;
		retrieveFullname(out);
		return out;
	}


	void Atom::doAppendCaption(YString& out, const ClassdefTableView* table) const
	{
		// append the name of its ancestor, with the table to resolve their specialization
		// (for template clsses for example)
		retrieveFullname(out, table); // parents

		switch (type)
		{
			case Type::funcdef:
			{
				if (name.startsWith("^default-var-%")) // keep it simple
					break;

				if (name.first() == '^')
					out << ' '; // for beauty

				// break fallthru
			}
			case Type::classdef:
			case Type::typealias:
			{
				if (not tmplparams.empty())
					atomParametersPrinter(out, tmplparams, table, false, "<:", ":>");
				if (not parameters.empty())
					atomParametersPrinter(out, parameters, table, true, "(", ")");

				if (table)
				{
					if (not returnType.clid.isVoid() and table->hasClassdef(returnType.clid))
					{
						auto& retcdef = table->classdef(returnType.clid);
						if (not retcdef.isVoid())
						{
							out.write(": ", 2);
							retcdef.print(out, *table, false);
						}
						break;
					}
				}
				else
				{
					if (not returnType.clid.isVoid())
						out << ": ref";
					break;
				}
				break;
			}

			case Type::namespacedef:
			case Type::vardef:
			case Type::unit:
				break;
		}
	}


	void Atom::retrieveCaption(YString& out, const ClassdefTableView& table) const
	{
		doAppendCaption(out, &table);
	}

	YString Atom::caption(const ClassdefTableView& view) const
	{
		String out;
		retrieveCaption(out, view);
		return out;
	}

	YString Atom::caption() const
	{
		String out;
		doAppendCaption(out, nullptr);
		return out;
	}




	inline void Atom::doPrint(const ClassdefTableView& table, uint depth) const
	{
		auto entry = trace();
		for (uint i = depth; i--; )
			entry.message.prefix << "    ";

		if (parent != nullptr)
		{
			entry.message.prefix << table.keyword(*this) << ' ';
			retrieveCaption(entry.data().message, table);
			entry << " [id:" << atomid;
			if (isMemberVariable())
				entry << ", field: " << varinfo.effectiveFieldIndex;
			entry << ']';
		}
		else
			entry << "{global namespace}";


		++depth;
		for (auto& child: pChildren)
			child.second->doPrint(table, depth);

		if (Type::classdef == type)
			trace(); // for beauty
	}


	void Atom::print(const ClassdefTableView& table) const
	{
		doPrint(table, 0);
	}


	bool Atom::performNameLookupOnChildren(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name, bool* singleHop)
	{
		assert(not name.empty());

		// the current scope
		Atom& scope = (nullptr == scopeForNameResolution) ? *this : *scopeForNameResolution;

		if (scope.isFunction() and name == "^()") // shorthand for function calls
		{
			list.push_back(std::ref(scope));
			return true;
		}

		bool success = false;
		scope.eachChild(name, [&](Atom& child) -> bool
		{
			list.push_back(std::ref(child));
			success = true;
			return true; // let's continue
		});

		if (success and name == "^()") // operator () resolved by a real atom
		{
			if (singleHop)
				*singleHop = true;
		}
		return success;
	}


	bool Atom::performNameLookupFromParent(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name)
	{
		assert(not name.empty());

		// the current scope
		Atom& scope = (nullptr == scopeForNameResolution) ? *this : *scopeForNameResolution;

		// rule: do not continue if there are some matches and if the scope is a class
		// this is to avois spurious name resolution:
		//
		//     func foo() -> 42;
		//
		//     class SomeClass
		//     {
		//         func foo() -> 12;
		//         func bar -> foo(); // must resolve to `SomeClass.foo`, and not `foo`
		//     }
		//

		if (scope.parent)
		{
			// should start from the very bottom
			bool askToParentFirst = not scope.isClass();;
			if (askToParentFirst)
			{
				if (scope.parent->performNameLookupFromParent(list, name))
					return true;
			}

			// try to resolve locally
			bool found = scope.performNameLookupOnChildren(list, name);

			if (not found and (not askToParentFirst)) // try again
				found = scope.parent->performNameLookupFromParent(list, name);

			return found;
		}
		else
		{
			// try to resolve locally
			return scope.performNameLookupOnChildren(list, name);
		}

		return false;
	}


	uint32_t Atom::invalidateInstance(const Signature& signature, uint32_t id)
	{
		instances[id].reset(nullptr);

		pInstancesIDs[signature] = (uint32_t) -1;

		auto& md = pInstancesMD[id];
		md.sequence = nullptr;
		md.remapAtom = nullptr;

		return (uint32_t) -1;
	}


	uint32_t Atom::createInstanceID(const Signature& signature, IR::Sequence* sequence, Atom* remapAtom)
	{
		assert(sequence != nullptr);

		// the new instanceID
		uint32_t iid = (uint32_t) instances.size();

		instances.emplace_back(sequence);
		pInstancesMD.emplace_back();

		auto& md = pInstancesMD[iid];
		md.remapAtom = remapAtom;
		md.sequence  = sequence;

		pInstancesIDs.insert(std::make_pair(signature, iid));
		assert(instances.size() == pInstancesMD.size());
		return iid;
	}


	Tribool::Value Atom::findInstance(const Signature& signature, uint32_t& iid, Classdef& rettype, Atom*& remapAtom) const
	{
		auto it = pInstancesIDs.find(signature);
		if (it != pInstancesIDs.end())
		{
			uint32_t id = it->second;
			if (id != (uint32_t) -1)
			{
				assert(id < pInstancesMD.size());
				auto& md  = pInstancesMD[id];

				iid       = id;
				remapAtom = md.remapAtom;

				rettype.import(md.rettype);
				rettype.qualifiers = md.rettype.qualifiers;
				return Tribool::Value::yes;
			}
			return Tribool::Value::no;
		}
		return Tribool::Value::indeterminate;
	}


	void Atom::printInstances(Clob& out, const AtomMap& atommap) const
	{
		assert(instances.size() == pInstancesMD.size());
		String prgm;

		for (size_t i = 0; i != instances.size(); ++i)
		{
			out << pInstancesMD[i].symbol << " // " << atomid << " #" << i << "\n{\n";

			instances[i].get()->print(prgm, &atommap);

			prgm.replace("\n", "\n    ");
			prgm.trimRight();
			out << prgm << "\n}\n";
		}
	}


	uint32_t Atom::findInstanceID(const IR::Sequence& sequence) const
	{
		for (size_t i = 0; i != instances.size(); ++i)
		{
			if (&sequence == instances[i].get())
				return static_cast<uint32_t>(i);
		}
		return (uint32_t) -1;
	}


	AnyString Atom::findInstanceCaption(const IR::Sequence& sequence) const
	{
		for (size_t i = 0; i != instances.size(); ++i)
		{
			if (&sequence == instances[i].get())
				return pInstancesMD[i].symbol;
		}
		return AnyString{};
	}


	const Atom* Atom::findChildByMemberFieldID(uint32_t field) const
	{
		const Atom* ret = nullptr;
		eachChild([&](const Atom& child) -> bool
		{
			if (child.varinfo.fieldindex == field)
			{
				ret = &child;
				return false;
			}
			return true; // let's continue
		});
		return ret;
	}


	void Atom::extractNames(AnyString& keyword, AnyString& varname, const AnyString& name)
	{
		if (not name.empty())
		{
			if (name[0] != '^')
			{
				keyword.clear();
				varname = name;
				return;
			}
			if (name.startsWith("^default-var-%"))
			{
				keyword = "<default-init>";
				auto endOffset = name.find_last_of('-');
				varname = (endOffset < name.size())
					? AnyString{name, endOffset + 1} : AnyString{"<invalid-field>"};
				return;
			}
			if (name.startsWith("^view^"))
			{
				keyword = "view";
				varname = AnyString{name.c_str() + 6, name.size() - 6};
				return;
			}

			keyword = "operator";
			varname = AnyString{name.c_str() + 1, name.size() - 1};
		}

		keyword.clear();
		varname.clear();
	}

	AnyString Atom::keyword() const
	{
		switch (type)
		{
			case Type::funcdef:
			{
				return (name[0] != '^')
					? "func"
					: ((not name.startsWith("^default-var-%"))
						? (not name.startsWith("^view^") ? "operator" : "view")
						: "<default-init>");
			}
			case Type::classdef:     return "class";
			case Type::namespacedef: return "namespace";
			case Type::vardef:       return "var";
			case Type::typealias:    return "typedef";
			case Type::unit:         return "unit";
		}
		return "auto";
	}


	uint32_t Atom::findClassAtom(Atom*& out, const AnyString& name)
	{
		// first, try to find the dtor function
		Atom* atomA = nullptr;
		uint32_t count = 0;

		eachChild([&](Atom& child) -> bool
		{
			if (child.isClass() and child.name == name)
			{
				if (likely(atomA == nullptr))
				{
					atomA = &child;
				}
				else
				{
					count = 2;
					return false;
				}
			}
			return true;
		});

		if (count != 0)
			return count;

		out = atomA;
		return atomA ? 1 : 0;
	}



	uint32_t Atom::findFuncAtom(Atom*& out, const AnyString& name)
	{
		// first, try to find the dtor function
		Atom* atomA = nullptr;
		uint32_t count = 0;

		eachChild([&](Atom& child) -> bool
		{
			if (child.isFunction() and child.name == name)
			{
				if (likely(atomA == nullptr))
				{
					atomA = &child;
				}
				else
				{
					count = 2;
					return false;
				}
			}
			return true;
		});

		if (count != 0)
			return count;

		out = atomA;
		return atomA ? 1 : 0;
	}


	uint32_t Atom::findVarAtom(Atom*& out, const AnyString& name)
	{
		// first, try to find the dtor function
		Atom* atomA = nullptr;
		uint32_t count = 0;

		eachChild([&](Atom& child) -> bool
		{
			if (child.isMemberVariable() and child.name == name)
			{
				if (likely(atomA == nullptr))
				{
					atomA = &child;
				}
				else
				{
					count = 2;
					return false;
				}
			}
			return true;
		});

		if (count != 0)
			return count;

		out = atomA;
		return atomA ? 1 : 0;
	}


	Atom* Atom::findNamespaceAtom(const AnyString& name)
	{
		// first, try to find the dtor function
		Atom* atomA = nullptr;

		eachChild([&](Atom& child) -> bool
		{
			if (child.isNamespace() and child.name == name)
			{
				atomA = &child;
				return false;
			}
			return true;
		});
		return atomA;
	}


	void Atom::renameChild(const AnyString& from, const AnyString& to)
	{
		Ptr child;
		auto range = pChildren.equal_range(from);
		for (auto it = range.first; it != range.second; )
		{
			if (unlikely(!!child)) // error
				return;
			child = it->second;
			it = pChildren.erase(it);
		}

		child->name = to;
		pChildren.insert(std::pair<AnyString, Atom::Ptr>(to, child));
	}


	bool Atom::findParent(const Atom& atom) const
	{
		const Atom* p = this;
		do
		{
			p = p->parent;
			if (p == &atom)
				return true;
		}
		while (p);
		return false;
	}




} // namespace Nany
