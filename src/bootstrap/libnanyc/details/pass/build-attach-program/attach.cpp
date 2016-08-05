#include "details/context/build.h"
#include "details/atom/classdef-table.h"
#include "details/reporting/report.h"
#include <unordered_map>
#include "mapping.h"

using namespace Yuni;





namespace Nany
{


	bool Build::attach(IR::Sequence& sequence, bool owned)
	{
		// keep the sequence somewhere
		{
			MutexLocker locker{mutex};
			pAttachedSequences.push_back(AttachedSequenceRef{&sequence, owned});
		}

		Pass::Mapping::SequenceMapping mapper{cdeftable, mutex, sequence};
		return mapper.map(cdeftable.atoms.root);
	}





} // namespace Nany
