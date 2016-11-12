#include "atom.h"
#include "details/ir/sequence.h"
#include "details/atom/classdef-table.h"
#include "details/atom/classdef-table-view.h"
#include "details/reporting/report.h"
#include "details/errors/errors.h"

using namespace Yuni;





namespace ny
{

	namespace {


	template<class OutT, class ListT, class TableT>
	void atomParametersPrinter(OutT& out, ListT& list, const TableT* table, bool avoidSelf, AnyString sepBefore, AnyString sepAfter)
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


	Flags<Atom::Category> findCategory(Atom* parent, Atom::Type type, const AnyString& name)
	{
		Flags<Atom::Category> category;

		if (parent and parent->type == Atom::Type::classdef)
			category += Atom::Category::classParent;

		if (unlikely(name.empty()))
			return category;

		switch (type)
		{
			case Atom::Type::funcdef:
			{
				if (name[0] == '^')
				{
					category += Atom::Category::special;

					if (category(Atom::Category::classParent))
					{
						if (name == "^new" or name == "^default-new")
						{
							category += Atom::Category::ctor;
						}
						else if (name == "^clone")
						{
							category += Atom::Category::clone;
						}
						else if (name == "^#user-dispose")
						{
							category += Atom::Category::dtor;
						}
						else if (name.startsWith("^view^"))
						{
							category += Atom::Category::view;
						}
						else if (name.startsWith("^default-var-%"))
						{
							category += Atom::Category::defvarInit;
						}
						else
						{
							category += Atom::Category::funcoperator;
							if (name == "^()")
								category += Atom::Category::functor;
						}
					}
					else
					{
						if (name.startsWith("^unittest^"))
							category += Atom::Category::unittest;
						else
							category += Atom::Category::funcoperator;
					}

					if (category(Atom::Category::funcoperator))
					{
						if (name.startsWith("^propget^"))
							category += Atom::Category::propget;
						else if (name.startsWith("^propset^"))
							category += Atom::Category::propset;
					}
				}
				break;
			}
			case Atom::Type::vardef:
			{
				if (name.startsWith("^trap^"))
					category += Atom::Category::capturedVar;
				break;
			}
			default:
				break;
		}
		return category;
	}


	void makeCaption(String& out, const Atom& atom, const ClassdefTableView* table, bool fullname = true)
	{
		// append the name of its ancestor, with the table to resolve their specialization
		// (for template clsses for example)
		atom.retrieveFullname(out, table, fullname); // parents

		switch (atom.type)
		{
			case Atom::Type::funcdef:
			{
				if (atom.isMemberVarDefaultInit())
					break;
				if (atom.isSpecial()) // for beauty
					out << ' ';
				if (atom.isProperty()) // just the name for properties
					break;
			}
			// [[fallthu]]
			case Atom::Type::classdef:
			case Atom::Type::typealias:
			{
				if (not atom.tmplparams.empty())
					atomParametersPrinter(out, atom.tmplparams, table, false, "<:", ":>");
				if (not atom.parameters.empty())
					atomParametersPrinter(out, atom.parameters, table, true, "(", ")");

				if (table)
				{
					if (not atom.returnType.clid.isVoid() and table->hasClassdef(atom.returnType.clid))
					{
						auto& retcdef = table->classdef(atom.returnType.clid);
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
					if (not atom.returnType.clid.isVoid())
						out << ": ref";
					break;
				}
				break;
			}

			case Atom::Type::namespacedef:
			case Atom::Type::vardef:
			case Atom::Type::unit:
				break;
		}
	}


	void printTreeRecursive(const Atom& atom, const ClassdefTableView& table, uint depth = 0)
	{
		auto entry = trace();
		for (uint i = depth; i--; )
			entry.message.prefix << "    ";

		if (atom.parent != nullptr)
		{
			entry.message.prefix << table.keyword(atom) << ' ';
			bool parentNames = atom.isNamespace() or atom.isUnit();
			makeCaption(entry.data().message, atom, &table, parentNames);
			entry << " [id:" << atom.atomid;
			if (atom.isMemberVariable())
				entry << ", field: " << atom.varinfo.effectiveFieldIndex;
			entry << ']';
		}
		else
			entry << "{global namespace}";

		++depth;
		atom.eachChild([&table, depth] (const Atom& child) -> bool {
			printTreeRecursive(child, table, depth);
			return true;
		});

		if (Atom::Type::classdef == atom.type)
			trace(); // for beauty
	}


	} // anonymous namespace




	Atom::Instances::Ref Atom::Instances::create(const Signature& signature, IR::Sequence* sequence, Atom* remapAtom)
	{
		assert(sequence != nullptr);
		uint32_t index = size();
		m_instances.emplace_back();
		auto& details = m_instances[index];
		details.remapAtom = remapAtom;
		details.sequence  = std::unique_ptr<IR::Sequence>(sequence);
		m_instancesIDs.emplace(signature, index);
		return Ref{*this, index};
	}


	void Atom::Instances::update(uint32_t index, String&& symbol, const Classdef& rettype)
	{
		assert(index < size());
		auto& details = m_instances[index];
		details.rettype.import(rettype);
		details.rettype.qualifiers = rettype.qualifiers;
		details.symbol = std::move(symbol);
	}


	uint32_t Atom::Instances::invalidate(uint32_t index, const Signature& signature)
	{
		assert(index < m_instances.size());
		m_instancesIDs[signature] = (uint32_t) -1;
		auto& details = m_instances[index];
		details.sequence = nullptr;
		details.remapAtom = nullptr;
		return (uint32_t) -1;
	}


	Tribool::Value Atom::Instances::isValid(const Signature& signature, uint32_t& iid, Classdef& rettype, Atom*& remapAtom) const
	{
		auto it = m_instancesIDs.find(signature);
		if (it != m_instancesIDs.end())
		{
			uint32_t id = it->second;
			if (id != (uint32_t) -1)
			{
				assert(id < m_instances.size());
				auto& details  = m_instances[id];
				iid = id;
				remapAtom = details.remapAtom;
				rettype.import(details.rettype);
				rettype.qualifiers = details.rettype.qualifiers;
				return Tribool::Value::yes;
			}
			return Tribool::Value::no;
		}
		return Tribool::Value::indeterminate;
	}


	String Atom::Instances::print(const AtomMap& atommap) const
	{
		String prgm;
		String out;
		for (size_t i = 0; i != m_instances.size(); ++i)
		{
			out << m_instances[i].symbol << " // #" << i << "\n{\n";
			if (m_instances[i].sequence)
				m_instances[i].sequence->print(prgm, &atommap);
			prgm.replace("\n", "\n    ");
			prgm.trimRight();
			out << prgm << "\n}\n";
		}
		return out;
	}


	Atom::Atom(const AnyString& name, Atom::Type type)
		: category{findCategory(nullptr, type, name)}
		, type(type)
		, m_name{name}
	{}


	Atom::Atom(Atom& rootparent, const AnyString& name, Atom::Type type)
		: category{findCategory(&rootparent, type, name)}
		, type(type)
		, parent(&rootparent)
		, m_name{name}
	{
		rootparent.m_children.emplace(AnyString{m_name}, this);
	}


	Atom::~Atom()
	{
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
						out += m_name;
					}
					else
					{
						if (isMemberVarDefaultInit())
						{
							AnyString sub{m_name, 14};
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
								out << AnyString{m_name, 9};
							else
								out << "<unmanaged prop>";
						}
						else if (isView())
						{
							out += ':'; // ^view^
							out.append(AnyString{m_name, 6});
						}
						else if (isUnittest())
						{
							out.append(AnyString{m_name, 17}); // "^unittest^module:<name>"
						}
						else
							out.append(AnyString{m_name, 1}); // operator like, removing ^
					}

					if (not tmplparamsForPrinting.empty())
						atomParametersPrinter(out, tmplparamsForPrinting, table, false, "<:", ":>");
					break;
				}

				case Type::classdef:
				{
					out += m_name;

					if (not tmplparamsForPrinting.empty())
						atomParametersPrinter(out, tmplparamsForPrinting, table, false, "<:", ":>");
					break;
				}

				case Type::namespacedef:
				case Type::typealias:
				case Type::vardef:
				case Type::unit:
				{
					out += m_name;
					break;
				}
			}
		}
	}


	String Atom::fullname() const
	{
		String out;
		retrieveFullname(out);
		return out;
	}


	void Atom::retrieveCaption(String& out, const ClassdefTableView& table) const
	{
		makeCaption(out, *this, &table);
	}


	String Atom::caption(const ClassdefTableView& view) const
	{
		String out;
		retrieveCaption(out, view);
		return out;
	}


	String Atom::caption() const
	{
		String out;
		makeCaption(out, *this, nullptr);
		return out;
	}


	void Atom::printTree(const ClassdefTableView& table) const
	{
		printTreeRecursive(*this, table);
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
			else if (name.startsWith("^unittest^"))
			{
				keyword = "unittest";
				varname = AnyString{name, 17}; // "^unittest^module:<testname>"
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
				if (isUnittest())
					return "unittest";
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
			if (child.isClass() and child.m_name == name)
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
			if (child.isFunction() and child.m_name == name)
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
			if (child.isMemberVariable() and child.m_name == name)
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
			if (child.isNamespace() and child.m_name == name)
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
		auto range = m_children.equal_range(from);
		for (auto it = range.first; it != range.second; )
		{
			if (unlikely(!!child)) // error
				return;
			child = it->second;
			it = m_children.erase(it);
		}
		child->m_name = to;
		child->category = findCategory(child->parent, child->type, to);
		m_children.emplace(AnyString{child->m_name}, child);
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


	uint32_t Atom::Parameters::findByName(const AnyString& name, uint32_t offset) const
	{
		if (!!pData)
		{
			auto& internal = *pData.get();
			uint32_t count = internal.count;
			for (uint32_t i = offset; i < count; ++i)
			{
				if (name == internal.params[i].first)
					return i;
			}
		}
		return static_cast<uint32_t>(-1);
	}




} // namespace ny
