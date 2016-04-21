#include "details/context/build.h"
#include "details/atom/classdef-table.h"
#include "details/reporting/report.h"
#include "libnany-config.h"
#include <unordered_map>
#include "mapping.h"

using namespace Yuni;





namespace Nany
{


	bool Build::attach(IR::Sequence& sequence, Logs::Report& report, bool owned)
	{
		// keep the sequence somewhere
		{
			MutexLocker locker{mutex};
			pAttachedSequences.push_back(AttachedSequenceRef{&sequence, owned});
		}

		Pass::Mapping::SequenceMapping mapper{cdeftable, mutex, report, sequence};
		return mapper.map(cdeftable.atoms.root);
	}





} // namespace Nany
