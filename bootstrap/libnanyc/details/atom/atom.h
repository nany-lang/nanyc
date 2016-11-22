#pragma once
#include <yuni/yuni.h>
#include <yuni/core/smartptr/intrusive.h>
#include <yuni/core/string.h>
#include <yuni/core/noncopyable.h>
#include <yuni/core/flags.h>
#include <yuni/core/tribool.h>
#include "details/fwd.h"
#include "details/utils/clid.h"
#include "vardef.h"
#include "nany/nany.h"
#include "details/ir/fwd.h"
#include "signature.h"
#include <unordered_map>
#include <unordered_set>
#include <map>
#include "libnanyc-config.h"




namespace ny
{

	// forward class
	struct AtomMap;
	class ClassdefTable;
	class ClassdefTableView;
	namespace ir { struct Sequence; }


	/*!
	** \brief Definition of a single class or function
	*/
	struct Atom final
		: public Yuni::IIntrusiveSmartPtr<Atom, false, Yuni::Policy::SingleThreaded>
		, Yuni::NonCopyable<Atom>
	{
		//! The class ancestor
		using Ancestor = Yuni::IIntrusiveSmartPtr<Atom, false, Yuni::Policy::SingleThreaded>;
		//! The most suitable smart ptr for the class
		using Ptr = Ancestor::SmartPtrType<Atom>::Ptr;

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

		enum class Flags: uint32_t
		{
			//! Allow this atom in error reporting
			suggestInReport,
			//! Allow this atom (and the sub-functions) to capture variables
			captureVariables,
			//! Suppress error messages related to this atom (and code generation)
			error,
			//! Append captured variables when calling this function (ctor)
			pushCapturedVariables,
			//! This func will call itself (directly or indirectly)
			recursive,

			//! Atom currently being instanciated
			instanciating,
		};

		template<Flags F> struct FlagAutoSwitch final {
			FlagAutoSwitch(Atom& atom) :atom(atom) { atom.flags += F; }
			~FlagAutoSwitch() { atom.flags -= F; }
			Atom& atom;
		};

		enum class Category: uint32_t
		{
			//! Not a normal func/class
			special,
			//! Operator
			funcoperator,
			//! Is the atom a view
			view,
			//! When the atom has a class as parent
			classParent,
			//! When the atom is a constructor
			ctor,
			//! When the atom is copy constructor
			clone,
			//! When the atom is the destructor
			dtor,
			//! Default variable initialization
			defvarInit,
			//! Functor object
			functor,
			//! Captured variable
			capturedVar,
			//! Property get
			propget,
			//! Property set
			propset,
			//! property set, custom operator, for future uses
			propsetCustom,
			//! unittest
			unittest,
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
			uint32_t findByName(const AnyString& name, uint32_t offset = 0) const;

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
				uint32_t count = 0u;
				std::pair<AnyString, Vardef> params[Config::maxFuncDeclParameterCount];
			};
			std::unique_ptr<Data> pData;
		};



	public:
		//! Create a dummy atom
		static Atom* createDummy();

		//! Get the atom keyword ('func', 'class', 'operator'...) of a given name
		static void extractNames(AnyString& keyword, AnyString& varname, const AnyString& name);


	public:
		//! Public destructor
		~Atom();

		//! \name Queries
		//@{
		//! Atom name
		AnyString name() const;
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
		//! Get if the atom is a special atom (operator, view...)
		bool isSpecial() const;
		//! Get if the atom is a view
		bool isView() const;
		//! Get if the atom is a member variable
		bool isMemberVariable() const;
		//! Get if the atom is a class member (function or variable)
		bool isClassMember() const;
		//! Get if the atom is an anonymous class
		bool isAnonymousClass() const;
		//! Is a constructor
		bool isMemberVarDefaultInit() const;
		//! Is a type alias
		bool isTypeAlias() const;
		//! Get if the atom is an unit
		bool isUnit() const;
		//! Get if the atom is a constructor
		bool isCtor() const;
		//! Get if the atom is a destructor
		bool isDtor() const;
		//! Get if the atom is a clone constructor
		bool isCloneCtor() const;
		//! Get if the atom is a functor (operator ())
		bool isFunctor() const;
		//! Get if the atom is callable (can be called like a function)
		bool callable() const;
		//! Get if the atom is a captured variable
		bool isCapturedVariable() const;
		//! Get if the atom is a property (get or set)
		bool isProperty() const;
		//! Get if the atom is a property getter
		bool isPropertyGet() const;
		//! Get if the atom is a property setter
		bool isPropertySet() const;
		//! Get if the atom is a property setter (custom operator)
		bool isPropertySetCustom() const;
		//! Get if the atom is an unittest
		bool isUnittest() const;

		//! Get if the atom is publicly accessible
		bool isPublicOrPublished() const;

		//! Get if the atom has a return type
		bool hasReturnType() const;

		//! Get if the atom has generic type parameters
		bool hasGenericParameters() const;

		/// \brief Get if the atom is contextual
		///
		/// Contextual means the internal values may vary from a parent
		/// instanciation, like a class inside a function, generic types params...
		bool isContextual() const;

		//! Get if this atom can access (use) another one
		bool canAccessTo(const Atom&) const;

		//! Get if the atom can capture out-of-scope variables
		bool canCaptureVariabes() const;

		//! Get the parent atom (if any)
		Atom* parentScope();

		//! Get the parent scope
		const Atom* parentScope() const;

		/*!
		** \brief Perform a name lookup from the local scope to the top root atom
		**
		** \param[in,out] list List where all matches will be added
		** \param name The identifier name to find
		** \return True if at least one overload has been found
		*/
		bool nameLookupFromParentScope(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name);

		/*!
		** \brief Perform a name lookup from the local scope only
		**
		** \param[in,out] list List where all matches will be added
		** \param name The identifier name to find
		** \return True if at least one overload has been found
		*/
		bool nameLookupOnChildren(std::vector<std::reference_wrapper<Atom>>& list, const AnyString& name,
			bool* singleHop = nullptr);

		/*!
		** \brief Perform a name lookup on properties from the local scope only
		**
		** \param[in,out] list List where all matches will be added
		** \param name The identifier name to find
		** \return True if at least one overload has been found
		*/
		bool propertyLookupOnChildren(std::vector<std::reference_wrapper<Atom>>& list,
			const AnyString& prefix, const AnyString& name);

		//! Get if this atom has a child member
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

		/*!
		** \brief Determine whether an atom is a parent to the current atom
		*/
		bool findParent(const Atom& atom) const;

		//! Get the number of children
		uint32_t childrenCount() const;

		//! Get if this atom owns children (sub-classes, methods...)
		bool hasChildren() const;
		//@}


		//! \name sizeof
		//@{
		//! Size at runtime (sizeof)
		uint64_t runtimeSizeof() const;
		//@}

		//! \name Info / Debugging
		//@{
		//! Get the full name of the atom (without any parameters)
		YString fullname() const;
		//! Append the full name of the atom (without any parameters)
		void retrieveFullname(YString& out, const ClassdefTableView* table = nullptr,
			bool parentName = true) const;

		//! Get the full name of the atom along with its parameters
		// \internal mainly used for convenience and debugging purposes
		YString caption() const;
		//! Get the full name of the atom along with its parameter
		// \internal mainly used for convenience and debugging purposes
		YString caption(const ClassdefTableView&) const;
		//! Append the full name of the atom along with its parameters
		void retrieveCaption(YString& out, const ClassdefTableView& table) const;

		//! Print the subtree
		void printTree(const ClassdefTableView&) const;

		/*!
		** \brief Keyword (class, func, var, namespace...)
		**
		** \note The result may lack in precision, since it can depend on the current
		**  classdef attached to it
		** \see ny::ClassdefTableView::keyword()
		*/
		AnyString keyword() const;
		//@}


	public:
		struct Instances final {
			struct Ref final {
				Ref(Instances& ref, uint32_t index): m_ref(ref), m_index(index) {}
				//! Get the attached IR sequence
				ir::Sequence& sequence();
				//! Get the attached IR sequence, if any
				ir::Sequence* sequenceIfExists();
				//! Get the symbol name of the instantiation (with fully qualified types)
				AnyString symbolname() const;
				//! Instance ID
				uint32_t id() const;
				//! \brief Update atom instance
				//! \param symbol The complete symbol name (ex: "func A.foo(b: ref __i32): ref __i32")
				//! \note The content of 'symbol' will be moved to avoid memory allocation
				void update(YString&& symbol, const Classdef& rettype);
				//! Mark the instantiation (and its signature) as invalid (always returns -1)
				uint32_t invalidate(const Signature&);

			private:
				Instances& m_ref;
				uint32_t m_index;
			};

			//! Print all instanciated sequences for the atom
			YString print(const AtomMap&) const;

			//! \brief Keep an instance of the atom for a given signature
			//! \param signature The signature of the atom (parameters)
			//! \return index of the instantiation
			Ref create(const Signature& signature, Atom* remapAtom);

			//! Fetch the sequence for a given signature (if any) and update the signature
			Yuni::Tribool::Value isValid(const Signature& signature, uint32_t& iid, Classdef&, Atom*& remapAtom) const;

			//! Number of instances
			uint32_t size() const;

			//! Retrieve information about the Nth instantiation of this atom
			Ref operator [] (uint32_t index);

		private:
			void update(uint32_t index, Yuni::String&& symbol, const Classdef& rettype);
			void invalidate(uint32_t index, const Signature&);

			struct Metadata final {
				std::unique_ptr<ir::Sequence> sequence;
				Classdef rettype;
				Atom* remapAtom = nullptr;
				Yuni::String symbol;
			};
			//! All instances, indexed by their internal id
			std::vector<Metadata> m_instances;
			//! All code instances
			std::unordered_map<Signature, uint32_t> m_instancesIDs;
		}
		instances;

		//! Atom unique ID (32-bits only, used for classification)
		uint32_t atomid = 0u;
		//! Atom flags
		Yuni::Flags<Flags> flags = {Flags::suggestInReport};
		//! Atom Category
		Yuni::Flags<Category> category;

		//! Various information for vardef only (class member)
		struct {
			//! Field index within the class (before optimization)
			// \note This value is used internally for mapping
			uint16_t fieldindex = 0;
			//! Effective field index for runtime
			uint16_t effectiveFieldIndex = 0;
		}
		varinfo;

		//! Various information for classdef only
		struct {
			//! Flag to determine whether the class has already been instanciated or not
			bool isInstanciated = false;
			//! Next field index for new variable members
			// (can be used to retrieve the generic type parameter index when the atom is a typedef)
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
		//! Visibility
		nyvisibility_t visibility = nyv_public;
		//! Parent node
		Atom* parent = nullptr;

		//! Origins
		struct {
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
		uint32_t localVariablesCount = 0u;
		//! A different scope for name resolution, if not null (for plugs/outlets)
		Atom* scopeForNameResolution = nullptr;

		//! The original IR sequence
		struct {
			//! The original IR sequence
			ir::Sequence* sequence = nullptr;
			//! Offset to start within this sequence
			// \warning offset of the operands of the blueprint, not the opcode value
			uint32_t offset = 0;
			//! For capturing variables, it may be required to increase the IR stack size
			uint32_t stackSizeExtra = 0;
			//! Flag to determine whether the sequence is owned by the atom or not
			bool owned = false;
		}
		opcodes;

		//! Builtin alias (empty if none)
		// \TODO builtinalias: this property is only used twice
		AnyString builtinalias;
		//! Builtin type (!= nyt_void if this atom represents a builtin)
		nytype_t builtinMapping = nyt_void;

		//! List of potential candidates for being captured
		std::unique_ptr<std::unordered_set<AnyString>> candidatesForCapture;



	private:
		//! Default constructor
		explicit Atom(const AnyString& name, Type type);
		//! Default constructor, with a parent
		explicit Atom(Atom& rootparent, const AnyString& name, Type type);


	private:
		//! Atoms that belong to this atom (sub-classes, methods...)
		std::multimap<AnyString, Ptr> m_children;
		//! Name of the current atom
		AnyString m_name;
		// nakama !
		friend struct AtomMap;

	}; // class Atom





} // namespace ny

#include "atom.hxx"
