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
						if (table->hasClassdef(vardef.clid))
						{
							auto& retcdef = table->rawclassdef(vardef.clid);
							if (not retcdef.isVoid())
							{
								out << ": ";
								retcdef.print(out, *table, false);
								if (debugmode)
									out << ' ' << retcdef.clid;
							}
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




	inline void Atom::name(const AnyString& newname)
	{
		pName = newname;
		category.clear();

		if (parent and parent->type == Type::classdef)
			category += Category::classParent;

		if (not newname.empty())
		{
			switch (type)
			{
				case Type::funcdef:
				{
					if (newname[0] == '^')
					{
						category += Category::special;

						if (category(Category::classParent))
						{
							if (newname == "^new" or newname == "^default-new")
							{
								category += Category::ctor;
							}
							else if (newname == "^clone")
							{
								category += Category::clone;
							}
							else if (newname == "^#user-dispose")
							{
								category += Category::dtor;
							}
							else if (newname.startsWith("^view^"))
							{
								category += Category::view;
							}
							else if (newname.startsWith("^default-var-%"))
							{
								category += Category::defvarInit;
							}
							else
							{
								category += Category::funcoperator;
								if (newname == "^()")
									category += Category::functor;
							}
						}
						else
							category += Category::funcoperator;

						if (category(Category::funcoperator))
						{
							if (newname.startsWith("^propget^"))
								category += Category::propget;
							else if (newname.startsWith("^propset^"))
								category += Category::propset;
						}
					}
					break;
				}

				case Type::vardef:
				{
					if (newname.startsWith("^trap^"))
						category += Category::capturedVar;
					break;
				}

				default:
					break;
			}
		}
	}


	Atom::Atom(const AnyString& atomName, Atom::Type type)
		: type(type)
	{
		this->name(atomName);
	}


	Atom::Atom(Atom& rootparent, const AnyString& atomName, Atom::Type type)
		: type(type)
		, parent(&rootparent)
	{
		this->name(atomName);
		rootparent.pChildren.insert(std::pair<AnyString, Atom::Ptr>(atomName, this));
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


	void Atom::retrieveFullname(Yuni::String& out, const ClassdefTableView* table, bool parentName) const
	{
		if (parent)
		{
			if (parentName and parent->parent)
			{
				parent->retrieveFullname(out, table);
				out += '.';
			}

			switch (type)
			{
				case Type::funcdef:
				{
					if (not isSpecial())
					{
						out += pName;
					}
					else
					{
						if (isMemberVarDefaultInit())
						{
							AnyString sub{pName, 14};
							auto ix = sub.find('-');

							if (ix < sub.size())
							{
								++ix;
								out << AnyString{sub, ix};
							}
							else
								out << "<invalid-field>";
						}
						else if (isProperty())
						{
							if (isPropertyGet() or isPropertySet()) // ^propget^ / ^propset^
								out << AnyString{pName, 9};
							else
								out << "<unmanaged prop>";
						}
						else if (isView())
						{
							out += ':'; // ^view^
							out.append(AnyString{pName, 6});
						}
						else
							out.append(AnyString{pName, 1}); // operator like, removing ^
					}

					if (not tmplparamsForPrinting.empty())
						atomParametersPrinter(out, tmplparamsForPrinting, table, false, "<:", ":>");
					break;
				}

				case Type::classdef:
				{
					out += pName;

					if (not tmplparamsForPrinting.empty())
						atomParametersPrinter(out, tmplparamsForPrinting, table, false, "<:", ":>");
					break;
				}

				case Type::namespacedef:
				case Type::typealias:
				case Type::vardef:
				case Type::unit:
				{
					out += pName;
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


	void Atom::doAppendCaption(YString& out, const ClassdefTableView* table, bool fullname) const
	{
		// append the name of its ancestor, with the table to resolve their specialization
		// (for template clsses for example)
		retrieveFullname(out, table, fullname); // parents

		switch (type)
		{
			case Type::funcdef:
			{
				if (isMemberVarDefaultInit())
					break;

				if (isSpecial()) // for beauty
					out << ' ';

				if (isProperty()) // just the name for properties
					break;
			}
			// [[fallthu]]
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




	inline void Atom::doPrintTree(const ClassdefTableView& table, uint depth) const
	{
		auto entry = trace();
		for (uint i = depth; i--; )
			entry.message.prefix << "    ";

		if (parent != nullptr)
		{
			entry.message.prefix << table.keyword(*this) << ' ';
			bool parentNames = isNamespace() or isUnit();
			doAppendCaption(entry.data().message, &table, parentNames);
			entry << " [id:" << atomid;
			if (isMemberVariable())
				entry << ", field: " << varinfo.effectiveFieldIndex;
			entry << ']';
		}
		else
			entry << "{global namespace}";

		++depth;
		for (auto& child: pChildren)
			child.second->doPrintTree(table, depth);

		if (Type::classdef == type)
			trace(); // for beauty
	}


	void Atom::printTree(const ClassdefTableView& table) const
	{
		doPrintTree(table, 0);
	}


	bool Atom::nameLookupOnChildren(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name, bool* singleHop)
	{
		assert(not name.empty());

		// the current scope
		Atom& scope = *this;
		// shorthand for func call ?
		bool isFuncCall = (name == "^()");

		if (isFuncCall and scope.isFunction()) // shorthand for function calls
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

		if (success and isFuncCall) // operator () resolved by a real atom
		{
			if (singleHop)
				*singleHop = true;
		}
		return success;
	}


	bool Atom::nameLookupFromParentScope(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name)
	{
		assert(not name.empty());

		// the current scope
		Atom* scope = parentScope();

		// rule: do not continue if there are some matches and if the scope is a class
		// this is to avoid spurious name resolution:
		//
		//     func foo() -> 42;
		//
		//     class SomeClass
		//     {
		//         func foo() -> 12;
		//         func bar -> foo(); // must resolve to `SomeClass.foo`, and not `foo`
		//     }
		//

		if (scope)
		{
			bool askToParentFirst = not this->isClass();
			if (askToParentFirst)
			{
				return // from the parent
					(scope->nameLookupFromParentScope(list, name))
					// or locally
					or this->nameLookupOnChildren(list, name);
			}
			else
			{
				// try to resolve locally
				return this->nameLookupOnChildren(list, name)
					or scope->nameLookupFromParentScope(list, name);
			}
		}
		else
		{
			// try to resolve locally
			return this->nameLookupOnChildren(list, name);
		}
	}


	bool Atom::propertyLookupOnChildren(std::vector<std::reference_wrapper<Atom>>& list,
		const AnyString& prefix, const AnyString& name)
	{
		assert(not name.empty());
		ShortString128 propname{prefix};
		propname << name;

		bool success = false;
		eachChild(propname, [&](Atom& child) -> bool
		{
			list.push_back(std::ref(child));
			success = true;
			return true; // let's continue
		});
		return success;
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


	/*static*/
	void Atom::extractNames(AnyString& keyword, AnyString& varname, const AnyString& name)
	{
		if (not name.empty())
		{
			if (name[0] != '^')
			{
				keyword.clear();
				varname = name;
			}
			else if (name.startsWith("^default-var-%"))
			{
				keyword = "<default-init>";
				auto endOffset = name.find_last_of('-');
				varname = (endOffset < name.size())
					? AnyString{name, endOffset + 1} : AnyString{"<invalid-field>"};
			}
			else if (name.startsWith("^view^"))
			{
				keyword = "view";
				varname = AnyString{name, 6};
			}
			else if (name.startsWith("^propget^"))
			{
				keyword = "property:get";
				varname = AnyString{name, 9};
			}
			else if (name.startsWith("^propset^"))
			{
				keyword = "property:set";
				varname = AnyString{name, 9};
			}
			else
			{
				keyword = "operator";
				varname = AnyString{name.c_str() + 1, name.size() - 1};
			}
		}
		else
		{
			keyword.clear();
			varname.clear();
		}
	}

	AnyString Atom::keyword() const
	{
		switch (type)
		{
			case Type::funcdef:
			{
				if (not isSpecial())
					return "func";
				if (isView())
					return "view";
				if (isPropertyGet())
					return "property:get";
				if (isPropertySet())
					return "property:set";
				if (isMemberVarDefaultInit())
					return "<default-init>";
				return "operator";
			}
			case Type::classdef:     return "class";
			case Type::vardef:       return "var";
			case Type::namespacedef: return "namespace";
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
			if (child.isClass() and child.pName == name)
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
			if (child.isFunction() and child.pName == name)
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
			if (child.isMemberVariable() and child.pName == name)
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
			if (child.isNamespace() and child.pName == name)
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

		child->name(to);
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
