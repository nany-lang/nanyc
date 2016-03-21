#pragma once
#include <yuni/yuni.h>
#include <yuni/core/smartptr/intrusive.h>
#include <yuni/core/string.h>
#include <yuni/core/noncopyable.h>
#include "details/fwd.h"
#include "details/utils/clid.h"
#include "vardef.h"
#include "nany/nany.h"
#include "details/ir/fwd.h"
#include "signature.h"
#include <unordered_map>
#include <map>
#include "libnany-config.h"




namespace Nany
{

	// forward class
	class AtomMap;
	class ClassdefTable;
	class ClassdefTableView;




	/*!
	** \brief Definition of a single class or function
	*/
	class Atom final
		: public Yuni::IIntrusiveSmartPtr<Atom, false, Yuni::Policy::SingleThreaded>
		, Yuni::NonCopyable<Atom>
	{
	public:
		//! The class ancestor
		typedef Yuni::IIntrusiveSmartPtr<Atom, false, Yuni::Policy::SingleThreaded>  Ancestor;
		//! The most suitable smart ptr for the class
		typedef Ancestor::SmartPtrType<Atom>::Ptr  Ptr;

		enum class Type: yuint8
		{
			//! define a part of a namespace
			namespacedef,
			//! function definition
			funcdef,
			//! class definition
			classdef,
			//! Variable definition
			vardef,
			//! Typedef
			typealias,
			//! Unit (source file)
			unit,
		};

		struct Parameters final
		{
			//! Get the total number of parameters
			uint size() const;
			//! Get if empty
			bool empty() const;
			//! Add a new parameter
			bool append(const CLID&, const AnyString& name);
			//! Try to find a parameter index from its name
			yuint32 findByName(const AnyString& name, yuint32 offset = 0) const;

			const std::pair<AnyString, Vardef>& operator [] (uint index) const;

			void swap(Parameters&);

			//! Parameter name
			const AnyString& name(uint index) const;
			//! Var def
			const Vardef& vardef(uint index) const;

			//! Iterate through all parameters
			// \param callbakc void()(uint32_t index, const AnyString& name, const Vardef& vardef)
			template<class T> void each(const T& callback) const;

			//! Shortcircuit value (if applicable)
			// \TODO ishortcircuitvalue: this property is only used twice...
			bool shortcircuitValue = false;

		private:
			struct Data final {
				yuint32 count = 0u;
				std::pair<AnyString, Vardef> params[Config::maxFuncDeclParameterCount];
			};
			std::unique_ptr<Data> pData;
		};


	public:
		//! Create a dummy atom
		static Atom* createDummy();


	public:
		//! Public destructor
		~Atom();

		//! \name Queries
		//@{
		//! Get the clid for this atom
		CLID clid() const;

		//! Get if the atom is a namespace
		bool isNamespace() const;
		//! Get if the atom is a class
		bool isClass() const;
		//! Get if the atom is a function
		bool isFunction() const;
		//! Get if the atom is an operator
		bool isOperator() const;
		//! Get if the atom is a member variable
		bool isMemberVariable() const;
		//! Get if the atom is a class member (function or variable)
		bool isClassMember() const;
		//! Is a constructor
		bool isMemberVarDefaultInit() const;
		//! Is a type alias
		bool isTypeAlias() const;
		//! Get if the atom is an unit
		bool isUnit() const;

		//! Get if the atom is publicly accessible
		bool isPublicOrPublished() const;

		//! Get if the atom has a return type
		bool hasReturnType() const;

		//! Get if the atom has generic type parameters
		bool hasGenericParameters() const;

		/*!
		** \brief Get if this atom can access (use) another one
		*/
		bool canAccessTo(const Atom&) const;

		/*!
		** \brief Perform a name lookup from the local scope to the top root atom
		**
		** This method is meant to be called several times with the same input (The input
		** parameters are guarantee to not be modified if nothing is found The input
		** vector is never cleared)
		**
		** \param[out] list A non-empty list if several overloads have been found
		** \return True if at least one overload has been found
		*/
		bool performNameLookupFromParent(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name);

		/*!
		** \brief Perform a name lookup from the local scope only
		**
		** This method is meant to be called several times with the same input (The input
		** parameters are guarantee to not be modified if nothing is found The input
		** vector is never cleared)
		**
		** \param[out] list A non-empty list if several overloads have been found
		** \return True if at least one overload has been found
		*/
		bool performNameLookupOnChildren(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name,
			bool* singleHop = nullptr);

		/*!
		** \brief Get if this atom has a child member
		*/
		bool hasMember(const AnyString& name) const;

		/*!
		** \brief Iterate through all children
		**
		** \code
		** atom.eachChild([&](Atom& child) -> bool {
		**	std::cout << child.name << std::endl;
		**	return true;
		** });
		** \endcode
		*/
		template<class C> void eachChild(const C& callback);
		template<class C> void eachChild(const C& callback) const;

		/*!
		** \brief Iterate through all children with a given name
		**
		** \code
		** atom.eachChild("childname", [&](Atom& child) -> bool {
		**	std::cout << child.name << std::endl;
		**	return true;
		** });
		** \endcode
		*/
		template<class C> void eachChild(const AnyString& needle, const C& callback);
		template<class C> void eachChild(const AnyString& needle, const C& callback) const;

		uint32_t findClassAtom(Atom*& out, const AnyString& name);
		uint32_t findFuncAtom(Atom*& out, const AnyString& name);
		uint32_t findVarAtom(Atom*& out, const AnyString& name);
		Atom* findNamespaceAtom(const AnyString& name);

		const Atom* findChildByMemberFieldID(uint32_t field) const;

		/*!
		** \brief Rename an atom
		**
		** \warning This method MUST be used with care since the atom does not own
		** the string representing its name
		*/
		void renameChild(const AnyString& from, const AnyString& to);

		//! Get the number of children
		uint32_t size() const;

		//! Get if the atom is empty (no child)
		bool empty() const;
		//@}


		//! \name sizeof
		//@{
		//! Size at runtime (sizeof)
		uint64_t runtimeSizeof() const;
		//@}


		//! \name Instances
		//@{
		/*!
		** \brief Fetch the sequence for a given signature (if any) and update the signature
		**
		** \param[out] Pointer to the sequence attached to the given signature
		** \param[in,out] The signature. If the instance is found, its return type
		**  will be updated accordingly
		*/
		uint32_t findInstance(IR::Sequence*& sequence, Atom*& remapAtom, Signature& signature);
		//! Find Instance ID
		uint32_t findInstanceID(const IR::Sequence&) const;

		//! Get the caption for a sequence instance
		AnyString findInstanceCaption(const IR::Sequence&) const;

		//! Fetch the sequence according its instance id
		const IR::Sequence* fetchInstance(uint32_t instanceid) const;

		AnyString fetchInstanceCaption(uint32_t instanceid) const;

		/*!
		** \brief Fetch the sequence according its instance id
		*/
		const IR::Sequence& instance(uint32_t instanceid) const;

		/*!
		** \brief Keep an instance of the atom for a given signature
		**
		** \param signature The signature of the atom (parameters)
		** \param sequence The sequence itself (must not be null)
		** \param symbolname The complete symbol name (ex: "func A.foo(b: ref __i32): ref __i32")
		*/
		uint32_t assignInstance(const Signature& signature, IR::Sequence* sequence,
			const AnyString& symbolname, Atom* remapAtom);

		//! Mark as invalid a given signature
		uint32_t assignInvalidInstance(const Signature& signature);
		//@}


		//! \name Info / Debugging
		//@{
		//! Get the full name of the atom (without any parameters)
		YString fullname() const;
		//! Append the full name of the atom (without any parameters)
		void retrieveFullname(YString& out) const;


		//! Get the full name of the atom along with its parameters
		// \internal mainly used for convenience and debugging purposes
		YString caption() const;
		//! Get the full name of the atom along with its parameter
		// \internal mainly used for convenience and debugging purposes
		YString caption(const ClassdefTableView&) const;
		//! Append the full name of the atom along with its parameters
		void retrieveCaption(YString& out, const ClassdefTableView& table) const;


		//! Print the subtree
		void print(Logs::Report& report, const ClassdefTableView&) const;

		//! Print all instanciated sequences for the atom
		void printInstances(Yuni::Clob& out, const AtomMap&) const;

		/*!
		** \brief Keyword (class, func, var, namespace...)
		**
		** \note The result may lack in precision, since it can depend on the current
		**  classdef attached to it
		** \see Nany::ClassdefTableView::keyword()
		*/
		AnyString keyword() const;
		//@}


	private:
		//! All instances, indexed by their internal id
		std::vector<std::unique_ptr<IR::Sequence>> instances;

	public:
		//! Atom unique ID (32-bits only, used for classification)
		yuint32 atomid = 0u;

		//! Various information for vardef only (class member)
		struct
		{
			//! Field index within the class (before optimization)
			// \note This value is used internally for mapping
			uint16_t fieldindex = 0;
			//! Effective field index for runtime
			uint16_t effectiveFieldIndex = 0;
		}
		varinfo;


		//! Various information for classdef only
		struct
		{
			//! Flag to determine whether the class has already been instanciated or not
			bool isInstanciated = false;
			//! Next field index for new variable members
			uint16_t nextFieldIndex = 0;

			//! Direct access to the destructor (used for reduce compilation time)
			struct {
				uint32_t atomid = 0;
				uint32_t instanceid = (uint32_t) -1;
			} dtor;

			//! Direct access to the clone func (used to reduce compilation time)
			struct {
				uint32_t atomid = 0;
				uint32_t instanceid = (uint32_t) -1;
			} clone;
		}
		classinfo;


		//! Flag to determine whether this entry is a blueprint or a namespace
		const Type type;
		//! Parent node
		Atom* parent = nullptr;
		//! Visibility
		nyvisibility_t visibility = nyv_public;

		//! Origins
		struct
		{
			//! Target source
			CTarget* target = nullptr;
			//! File source
			YString filename;

			//! Start offset
			uint32_t line = 0;
			//! end offset
			uint32_t offset = 0;
		}
		origin;


		//! The return type
		Vardef returnType;

		//! Parameters (for functions)
		Parameters parameters;
		//! Template parameters
		Parameters tmplparams;
		//! Template parameters just for printing informations (such as errors)
		Parameters tmplparamsForPrinting;

		//! The maximum number of variables / classdefs registered for the atom
		uint localVariablesCount = 0u;

		//! The original IR sequence
		struct
		{
			//! The original IR sequence
			IR::Sequence* sequence = nullptr;
			//! Offset to start within this sequence
			// \warning offset of the operands of the blueprint, not the opcode value
			uint32_t offset = 0;

			//! Flag to determine whether the sequence is owned by the atom or not
			bool owned = false;
		}
		opcodes;

		//! Builtin alias (empty if none)
		// \TODO builtinalias: this property is only used twice
		AnyString builtinalias;
		//! Builtin type (!= nyt_void if this atom represents a builtin)
		nytype_t builtinMapping = nyt_void;

		//! A different scope for name resolution, if not null
		Atom* scopeForNameResolution = nullptr;

		//! Name of the current atom
		AnyString name;

		//! Flag to suppress spurious error messages and code generation
		bool hasErrors = false;

		//! True if the atom has been defined by the user
		bool usedDefined = false;

		//! Can be used for error reporting
		bool canBeSuggestedInErrReporting = true;


	private:
		//! Default constructor
		explicit Atom(const AnyString& name, Type type);
		//! Default constructor, with a parent
		explicit Atom(Atom& rootparent, const AnyString& name, Type type);
		void doPrint(Logs::Report&, const ClassdefTableView& table, uint depth) const;
		template<class T> void doAppendCaption(YString& out, const T* table) const;

	private:
		//! All children
		std::multimap<AnyString, Ptr> pChildren;
		//! All code instances
		//std::unordered_map<Signature, std::pair<uint32_t, IR::Sequence*>> pInstancesBySign;
		struct SignatureMetadata {
			uint32_t instanceid = (uint32_t) -1;
			Atom* remapAtom = nullptr;
			IR::Sequence* sequence = nullptr;
		};
		std::unordered_map<Signature, SignatureMetadata> pInstancesBySign;
		//! Symbol names for instances in `pInstances`
		std::vector<Yuni::String> pSymbolInstances;

		// nakama !
		friend class AtomMap;

	}; // class Atom





} // namespace Nany

#include "atom.hxx"
