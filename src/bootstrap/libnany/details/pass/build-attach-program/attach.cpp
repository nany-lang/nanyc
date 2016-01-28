#include "details/context/isolate.h"
#include "details/atom/classdef-table.h"
#include "details/reporting/report.h"
#include "libnany-config.h"
#include <unordered_map>
#include "mapping.h"

using namespace Yuni;





namespace Nany
{


	bool Isolate::attach(IR::Sequence& sequence, Logs::Report& report, bool owned)
	{
		// keep the sequence somewhere
		{
			MutexLocker locker{mutex};
			pAttachedSequences.push_back(AttachedSequenceRef{&sequence, owned});
		}

		Pass::Mapping::SequenceMapping mapper{report, *this, sequence};
		return mapper.map(classdefTable.atoms.root);
	}



} // namespace Nany
