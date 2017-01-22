#pragma once
#include <yuni/core/tribool.h>


namespace ny {
namespace semantic {
namespace intrinsic {


yuni::Tribool::Value langOrNanycSpecifics(Analyzer&, const AnyString& name, uint32_t lvid, bool produceError = true);

yuni::Tribool::Value nanycSpecifics(Analyzer& analyzer, const AnyString& name, uint32_t lvid, bool produceError);


} // namespace intrinsic
} // namespace semantic
} // namespace ny
