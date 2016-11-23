#pragma once
#include "atom.h"
#include "details/utils/stringrefs.h"
#include "details/ir/fwd.h"



namespace ny
{

	// forward declaration
	class ClassdefTable;


	/*!
	** \brief Atoms
	** \note This class is not thread-safe
	*/
	struct AtomMap final
	{
		//! Create a new atom related to a part of a namespace
		Atom& createNamespace(Atom& parent, const AnyString& name);

		//! Create a new atom related to a function
		Atom& createFuncdef(Atom& parent, const AnyString& name);

		//! Create a new atom related to a class
		Atom& createClassdef(Atom& parent, const AnyString& name);

		//! Create a new atom related to a variable
		Atom& createVardef(Atom& parent, const AnyString& name);

		//! Create a new atom related to a type alias
		Atom& createTypealias(Atom& parent, const AnyString& name);

		//! Create a new atom related to an unit (source file)
		Atom& createUnit(Atom& parent, const AnyString& name);


		//! Find the IR sequence for a given {atomid/instanceid} (null if not found)
		const ir::Sequence* sequenceIfExists(uint32_t atomid, uint32_t index) const;

		//! Find the IR sequence for a given {atomid/instanceid}
		const ir::Sequence& sequence(uint32_t atomid, uint32_t index) const;

		//! Retrieve the human readable name of an atom (empty if not found)
		AnyString symbolname(uint32_t atomid, uint32_t index) const;

		//! Retrive an Atom object from its unique id (const)
		Atom::Ptr findAtom(uint32_t atomid) const;
		//! Retrive an Atom object from its unique id
		Atom::Ptr findAtom(uint32_t atomid);

		//! Try to retrieve the corresponding classes for core objects (bool, i32...)
		bool fetchAndIndexCoreObjects();


	public:
		//! The root atom (global namespace)
		Atom root;
		//! String catalog
		StringRefs& stringrefs;

		struct {
			Atom::Ptr object[nyt_count];
		}
		core;

	private:
		//! Default constructor
		explicit AtomMap(StringRefs& stringrefs);
		//! Create a new atom
		Atom& createNewAtom(Atom::Type type, Atom& root, const AnyString& name);

	private:
		std::vector<Atom::Ptr> m_byIndex;
		uint32_t m_atomGrpID = 0;
		friend class ClassdefTable;

	}; // struct AtomMap






} // namespace ny

#include "atom-map.hxx"
