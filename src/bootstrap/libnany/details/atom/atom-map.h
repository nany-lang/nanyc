#pragma once
#include "atom.h"
#include "details/utils/stringrefs.h"
#include "details/ir/fwd.h"



namespace Nany
{

	// forward declaration
	class ClassdefTable;





	/*!
	** \brief Atoms
	** \note This class is not thread-safe
	*/
	class AtomMap final
	{
	public:
		/*!
		** \brief Create a new atom related to a part of a namespace
		*/
		Atom* createNamespace(Atom& root, const AnyString& name);

		/*!
		** \brief Create a new atom related to a function
		*/
		Atom* createFuncdef(Atom& root, const AnyString& name);

		/*!
		** \brief Create a new atom related to a class
		*/
		Atom* createClassdef(Atom& root, const AnyString& name);

		/*!
		** \brief Create a new atom related to a variable
		*/
		Atom* createVardef(Atom& root, const AnyString& name);


		/*
		** \brief Fetch a program
		*/
		const IR::Program* fetchProgram(uint32_t atomid, uint32_t instanceid) const;

		/*
		** \brief Fetch a program
		*/
		const IR::Program& program(uint32_t atomid, uint32_t instanceid) const;

		/*!
		** \brief Retrieve the human readable name of an atom
		*/
		AnyString fetchProgramCaption(uint32_t atomid, uint32_t instanceid) const;

		//! Retrieve atom {const}
		const Atom* findAtom(uint32_t atomid) const;
		//! Retrieve atom
		Atom* findAtom(uint32_t atomid);




	public:
		//! The root atom (global namespace)
		Atom root;
		//! String catalog
		StringRefs& stringrefs;


	private:
		//! Default constructor
		explicit AtomMap(StringRefs& stringrefs);
		//! Create a new atom
		Atom* createNewAtom(Atom::Type type, Atom& root, const AnyString& name);

	private:
		std::vector<Atom*> pByIndex;
		yuint32 pAtomGrpID = 0;
		friend class ClassdefTable;

	}; // class AtomMap






} // namespace Nany

#include "atom-map.hxx"
