#pragma once
#include "../reporting/report.h"


namespace ny {

// forward declarations
namespace AST {
class Node;
}



/*!
** \brief Generate a new error report
*/
Logs::Report error();

/*!
** \brief Generate an error message
** \return Always false
*/
bool error(const AnyString& msg);

/*!
** \brief Generate a new error report
*/
Logs::Report error(const AST::Node&);

/*!
** \brief Generate a new error report
*/
void error(const AST::Node&, const AnyString& msg);


/*!
** \brief Generate a new warning report
*/
Logs::Report warning();

/*!
** \brief Generate a new warning report
*/
Logs::Report warning(const AST::Node&);

/*!
** \brief Generate a new info report
*/
Logs::Report info();

/*!
** \brief Generate a new hint report
*/
Logs::Report hint();

/*!
** \brief Generate a new ICE report
*/
Logs::Report ice();

/*!
** \brief Generate a new ICE report
*/
Logs::Report ice(const AST::Node&);

/*!
** \brief Generate a new trace report
*/
Logs::Report trace();

/*!
** \brief Generate a new verbose report
*/
Logs::Report verbose();


/*!
** \brief Emit ICE for unknown node
*/
bool unexpectedNode(const AST::Node&, const AnyString& extra = nullptr);





namespace Logs {


//! Get the current user handler
void* userHandlerPointer();

/*!
** \brief Get the current user handler
*/
template<class T> T* userHandler() {
	return reinterpret_cast<T*>(userHandlerPointer());
}


//! Callback for generating reporting
typedef Report (*Callback)(void*, Logs::Level);
//! Callback for metadata
typedef void (*CallbackMetadata)(void*, Logs::Level, const AST::Node*, Yuni::String& filename, uint32_t& line,
	 uint32_t& offset);


struct Handler final {
private:
	struct State {
		void* userdefined;
		Callback callback;
	};

public:
	//! Install a new handler
	explicit Handler(void* userdefined, Callback callback);
	//! Install the same handler than another thread
	explicit Handler(const State& state)
		: Handler(state.userdefined, state.callback) {}
	//! Dtor, pop the last handler state
	~Handler();


	//! Export the current thread state
	State exportThreadState();


private:
	State previousState;
}; // class Handler


struct MetadataHandler final {
private:
	struct State {
		void* userdefined;
		CallbackMetadata callback;
	};

public:
	//! Install a new handler
	explicit MetadataHandler(void* userdefined, CallbackMetadata callback);
	//! Dtor, pop the last handler state
	~MetadataHandler();


private:
	State previousState;
}; // class Handler





} // namespace Logs
} // namespace ny
