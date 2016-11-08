#include "instanciate.h"
#include "instanciate-error.h"
#include "type-check.h"
#ifdef YUNI_OS_UNIX
#include <unistd.h>
#endif

using namespace Yuni;




namespace ny
{
namespace Pass
{
namespace Instanciate
{


	namespace {


	bool intrinsicReinterpret(SequenceBuilder& seq, uint32_t lvid)
	{
		assert(seq.pushedparams.func.indexed.size() == 2);

		uint32_t lhs    = seq.pushedparams.func.indexed[0].lvid;
		uint32_t tolvid = seq.pushedparams.func.indexed[1].lvid;

		auto& cdef = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, tolvid});

		// copy the type, without any check
		auto& spare = seq.cdeftable.substitute(lvid);
		spare.import(cdef);
		spare.qualifiers = cdef.qualifiers;

		if (seq.canGenerateCode())
			seq.out->emitStore(lvid, lhs);

		auto& lvidinfo = seq.frame->lvids(lvid);
		lvidinfo.synthetic = false;

		if (seq.canBeAcquired(cdef) and seq.canGenerateCode()) // re-acquire the object
		{
			seq.out->emitRef(lvid);
			lvidinfo.autorelease = true;
			lvidinfo.scope = seq.frame->scope;
		}
		return true;
	}


	bool intrinsicMemcheckerHold(SequenceBuilder& seq, uint32_t lvid)
	{
		seq.cdeftable.substitute(lvid).mutateToVoid();

		uint32_t ptrlvid = seq.pushedparams.func.indexed[0].lvid;
		auto& cdefptr = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, ptrlvid});
		if (not cdefptr.isRawPointer())
			return seq.complainIntrinsicParameter("__nanyc_memchecker_hold", 0, cdefptr, "'__pointer'");

		uint32_t sizelvid = seq.pushedparams.func.indexed[1].lvid;
		auto& cdefsize = seq.cdeftable.classdefFollowClassMember(CLID{seq.frame->atomid, sizelvid});
		if (not cdefsize.isBuiltinU64())
			return seq.complainIntrinsicParameter("__nanyc_memchecker_hold", 0, cdefsize, "'__u64'");

		if (seq.canGenerateCode())
			seq.out->emitMemcheckhold(ptrlvid, sizelvid);
		return true;
	}




	using BuiltinIntrinsic = bool (*)(SequenceBuilder&, uint32_t);

	static const std::unordered_map<AnyString, std::pair<uint32_t, BuiltinIntrinsic>> builtinDispatch =
	{
		{"__reinterpret",            { 2,  &intrinsicReinterpret, }},
		{"__nanyc_memchecker_hold",  { 2,  &intrinsicMemcheckerHold, }},
	};

	} // anonymous namespace




	Tribool::Value SequenceBuilder::instanciateBuiltinIntrinsicSpecific(const AnyString& name, uint32_t lvid, bool canProduceError)
	{
		assert(not name.empty());
		assert(frame != nullptr);

		auto it = builtinDispatch.find(name);
		if (unlikely(it == builtinDispatch.end()))
		{
			if (canProduceError)
				complain::unknownIntrinsic(name);
			return Tribool::Value::indeterminate;
		}

		// checking for parameters
		uint32_t count = it->second.first;
		if (unlikely(not checkForIntrinsicParamCount(name, count)))
			return Tribool::Value::no;

		frame->lvids(lvid).synthetic = false;

		// intrinsic builtin found !
		return ((it->second.second))(*this, lvid) ? Tribool::Value::yes : Tribool::Value::no;
	}




} // namespace Instanciate
} // namespace Pass
} // namespace ny
