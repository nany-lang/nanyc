#pragma once
#include <yuni/core/string.h>


namespace ny {

/*!
** \brief Identifiers' visibility
*/
enum class Visibility: uint32_t {
	/*! no valid visibility */
	undefined,
	/*! default: public or internal, according the context */
	vdefault,
	/*! private: accessible only by the class */
	vprivate,
	/*! protected: accessible only by the class and all derived classes */
	vprotected,
	/*! internal: accessible only from the correspondig target */
	vinternal,
	/*! public: accessible by everyone */
	vpublic,
	/*! published: same as public, but accessible from an IDE */
	vpublished
};

constexpr uint32_t visibilityCount = 7;

Visibility toVisibility(AnyString);

AnyString toString(Visibility);

} // namespace ny
