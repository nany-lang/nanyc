#pragma once
#include "libnanyc.h"
#include <yuni/core/string.h>
#include "classdef.h"
#include "details/utils/stringrefs.h"
#include "atom-map.h"
#include <unordered_map>
#include <vector>


namespace ny {


struct ClassdefTableView;
struct Atom;


struct ClassdefTable final {
	//! \name Constructor & Destructor
	//@{
	//! Default constructor
	ClassdefTable();
	//! Copy constructor
	ClassdefTable(const ClassdefTable&) = delete;
	//! Move constructor
	ClassdefTable(ClassdefTable&&) = default;
	//! Destructor
	~ClassdefTable() = default;
	//@}


	//! \name Classdef
	//@{
	//! Get if a clid is available
	bool hasClassdef(const CLID&) const;

	/*!
	** \brief Retrieve or create a classdef from its ID
	*/
	Classdef& classdef(const CLID&);
	/*!
	** \brief Retrieve a classdef from its ID (or from the current layer)
	*/
	const Classdef& classdef(const CLID&) const;
	/*!
	** \brief Retrieve a classdef from its ID without using the current layer
	*/
	const Classdef& rawclassdef(const CLID&) const;
	Classdef& rawclassdef(const CLID&);

	//! Retrieve the real classdef from its ID (or from the current layer) even if a class member
	const Classdef& classdefFollowClassMember(const CLID&) const;

	/*!
	** \brief Get or create an interface from its name for a given CLID
	** \see classdef()
	*/
	Funcdef& addClassdefInterface(const CLID&, const AnyString& name);
	/*!
	** \brief Get or create an interface from its name for a given classdef
	*/
	Funcdef& addClassdefInterface(const Classdef&, const AnyString& name);

	/*!
	** \brief Get or create an interface from its name for a given CLID
	** \see classdef()
	*/
	Funcdef& addClassdefInterfaceSelf(const CLID&, const AnyString& name);
	/*!
	** \brief Get or create an interface from its name for a given classdef
	*/
	Funcdef& addClassdefInterfaceSelf(const Classdef&, const AnyString& name);

	/*!
	** \brief Get or create an interface from its name for a given CLID
	** \see classdef()
	*/
	Funcdef& addClassdefInterfaceSelf(const CLID&);
	/*!
	** \brief Get or create an interface from its name for a given classdef
	*/
	Funcdef& addClassdefInterfaceSelf(const Classdef&);

	/*!
	** \brief Make a target class ID share the same definition than a source class ID
	** \return True if the operation succeeded, false if source was not found
	*/
	bool makeHardlink(const CLID& source, const CLID& target);

	/*!
	** \brief Bulk create several class id
	*/
	void bulkCreate(std::vector<CLID>& out, uint32_t atomid, uint32_t count);

	/*!
	** \brief Bulk append class id
	*/
	void bulkAppend(uint32_t atomid, uint32_t offset, uint32_t count);

	/*!
	** \brief Register an Atom (created from blueprints)
	*/
	void registerAtom(Atom& atom);
	//@}


	//! \name Utilities
	//@{
	/*!
	** \brief Get the keyword associated to an atom (class, func, var, cref, namespace...)
	** \see ny::Atom::keyword()
	*/
	AnyString keyword(const Atom&) const;

	//! Find the atom of a classdef, via its extends if necessary
	Atom* findClassdefAtom(const Classdef&) const;
	//! Find the atom of a classdef, via its extends if necessary
	// (do not take layer into consideration)
	Atom* findRawClassdefAtom(const Classdef&) const;
	//@}


	//! \name Substitutes
	//@{
	//! Get if the current layer has a substitute for a given CLID
	bool hasSubstitute(CLID) const;
	//! Get the substiture for the current atomid in the current layer
	Classdef& substitute(uint32_t) const;

	//! Append a new substitute
	Classdef& addSubstitute(CType kind, Atom* atom, const Qualifiers&) const;

	//! Get the atom id of the current layer
	uint32_t substituteAtomID() const;

	//! Resize the substitutes for the current layer
	void substituteResize(uint32_t count);

	//! Make substitutes permanent
	void mergeSubstitutes();
	//@}


	//! \name Operators
	//@{
	//! No assignment operator
	ClassdefTable& operator = (const ClassdefTable&) = delete;
	//@}

public:
	//! String dictionary
	StringRefs stringrefs;
	//! Atom map
	AtomMap atoms;


private:
	inline bool nameLookupForClassdef(Classdef& classdef);
	inline bool nameLookupForSelfInterface(Classdef& classdef);

private:
	//! Current overlay layer
	//! \see class ClassdefTableView
	struct LayerItem final {
		void swap(LayerItem& rhs);

		//! The atom id associated to the current layer
		uint32_t atomid = (uint32_t) - 1;
		//! Max substitutes
		// \note On some STL implementation, std::vector.size is rather ineficient (distance(end - begin))
		uint32_t count = 0;
		//! local clid locally replaced
		std::vector<bool> flags;
		//! the substitutes
		std::vector<Classdef> storage;
	};

	//! All class definitions
	std::unordered_map<CLID, yuni::Ref<Classdef>> m_classdefs;
	//! The current layer
	mutable LayerItem m_layer;

	friend struct ClassdefTableView;

}; // struct ClassdefTable


} // namespace ny

#include "classdef-table.hxx"
