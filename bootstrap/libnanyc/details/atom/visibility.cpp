#include "visibility.h"
#include <cassert>


namespace ny {

Visibility toVisibility(AnyString text) {
	text.trimRight();
	switch (text.size()) {
		case 6: {
			if (text == "public")
				return Visibility::vpublic;
			break;
		}
		case 7: {
			if (text == "private")
				return Visibility::vprivate;
			break;
		}
		case 8: {
			if (text == "internal")
				return Visibility::vinternal;
			break;
		}
		case 9: {
			if (text == "published")
				return Visibility::vpublished;
			if (text == "protected")
				return Visibility::vprotected;
		}
	}
	return Visibility::undefined;
}

AnyString toString(Visibility visibility) {
	switch (visibility) {
		case Visibility::vpublished: return "published";
		case Visibility::vpublic:    return "public";
		case Visibility::vinternal:  return "internal";
		case Visibility::vprotected: return "protected";
		case Visibility::vprivate:   return "private";
		case Visibility::vdefault:   return "default";
		case Visibility::undefined:  break;
	}
	return "undefined";
}

} // namespace ny
