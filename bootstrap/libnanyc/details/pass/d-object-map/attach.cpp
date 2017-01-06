#include "details/context/build.h"
#include "details/atom/classdef-table.h"
#include "details/reporting/report.h"
#include <unordered_map>
#include "mapping.h"

using namespace Yuni;


namespace ny {


bool Build::attach(ir::Sequence& sequence, bool owned) {
	{
		MutexLocker locker{mutex};
		m_attachedSequences.push_back(AttachedSequenceRef{&sequence, owned});
	}
	Pass::MappingOptions options;
	return Pass::map(cdeftable.atoms.root, cdeftable, mutex, sequence, options);
}


} // namespace ny
