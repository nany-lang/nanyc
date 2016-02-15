#include "atom.h"
#include "details/ir/sequence.h"
#include "details/atom/classdef-table.h"
#include "details/atom/classdef-table-view.h"
#include "details/reporting/report.h"

using namespace Yuni;





namespace Nany
{

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
		assert(instances.size() == pSymbolInstances.size());
		if (opcodes.owned)
			delete opcodes.sequence;
	}


	uint32_t Atom::findInstance(IR::Sequence*& sequence, Signature& signature)
	{
		auto it = pInstancesBySign.find(signature);
		if (it != pInstancesBySign.end())
		{
			sequence = it->second.second;

			auto& storedRetType = it->first.returnType;
			signature.returnType.import(storedRetType);
			signature.returnType.qualifiers = storedRetType.qualifiers;
			return it->second.first;
		}
		return (uint32_t) -1;
	}


	bool Atom::canAccessTo(const Atom& atom) const
	{
		// same ancestor ? requesting access to a namespace ?
		if ((parent == atom.parent) or (atom.type == Type::namespacedef))
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


	void Atom::retrieveFullname(Yuni::String& out) const
	{
		if (parent)
		{
			if (parent->parent)
			{
				parent->retrieveFullname(out);
				out += '.';
			}

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
				else
					out.append(name.c_str() + 1, name.size() - 1);
			}
		}
	}


	YString Atom::fullname() const
	{
		YString out;
		retrieveFullname(out);
		return out;
	}


	template<class T>
	void Atom::doAppendCaption(YString& out, const T* table) const
	{
		retrieveFullname(out);

		switch (type)
		{
			case Type::funcdef:
			{
				if (name.startsWith("^default-var-%")) // keep it simple
					break;

				if (name.first() == '^')
					out << ' ';

				bool first = true;

				if (not tmplparams.empty())
				{
					out << "<:";

					tmplparams.each([&](uint32_t, const AnyString& paramname, const Vardef& vardef)
					{
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

					out << ":>";
				}

				out << '(';

				if (not parameters.empty())
				{
					first = true;

					parameters.each([&](uint32_t i, const AnyString& paramname, const Vardef& vardef)
					{
						// avoid the first virtual parameter
						if (i == 0 and paramname == "self")
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
				}
				out << ')';

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

			case Type::classdef:
			case Type::namespacedef:
			case Type::vardef:
			case Type::typealias:
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
		doAppendCaption<ClassdefTableView>(out, nullptr);
		return out;
	}




	inline void Atom::doPrint(Logs::Report& report, const ClassdefTableView& table, uint depth) const
	{
		auto entry = report.trace();
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
			child.second->doPrint(report, table, depth);

		if (Type::classdef == type)
			report.trace(); // for beauty
	}


	void Atom::print(Logs::Report& report, const ClassdefTableView& table) const
	{
		doPrint(report, table, 0);
	}


	bool Atom::performNameLookupOnChildren(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name)
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


	uint32_t Atom::assignInvalidInstance(const Signature& signature)
	{
		pInstancesBySign.insert(std::make_pair(signature, std::make_pair((uint32_t) -1, nullptr)));
		return (uint32_t) -1;
	}


	uint32_t Atom::assignInstance(const Signature& signature, IR::Sequence* sequence, const AnyString& symbolname)
	{
		assert(sequence != nullptr);
		uint32_t iid = (uint32_t) instances.size();
		instances.emplace_back(sequence);
		pSymbolInstances.emplace_back(symbolname);
		pInstancesBySign.insert(std::make_pair(signature, std::make_pair(iid, sequence)));
		assert(instances.size() == pSymbolInstances.size());
		return iid;
	}


	void Atom::printInstances(Clob& out, const AtomMap& atommap) const
	{
		assert(instances.size() == pSymbolInstances.size());
		Clob prgm;

		for (size_t i = 0; i != instances.size(); ++i)
		{
			out << pSymbolInstances[i] << " // " << atomid << " #" << i << "\n{\n";

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
				return pSymbolInstances[i];
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


	AnyString Atom::keyword() const
	{
		switch (type)
		{
			case Type::funcdef:
			{
				return (name[0] != '^')
					? "func"
					: ((not name.startsWith("^default-var-%"))
						? "operator"
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




} // namespace Nany
