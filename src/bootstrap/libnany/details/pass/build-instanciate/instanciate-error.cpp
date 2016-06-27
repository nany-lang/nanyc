#include "instanciate.h"
#include "overloaded-func-call-resolution.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	Logs::Report SequenceBuilder::emitReportEntry(void* self, Logs::Level level)
	{
		auto& sb = *(reinterpret_cast<SequenceBuilder*>(self));

		switch (level)
		{
			default:
				break;
			case Logs::Level::warning:
			{
				if (nyfalse != sb.build.cf.warnings_into_errors)
				{
					level = Logs::Level::error;
					sb.success = false;
				}
				break;
			}
			case Logs::Level::error:
			case Logs::Level::ICE:
			{
				sb.success = false;
				break;
			}
		}

		auto entry = sb.report.fromErrLevel(level);
		if (debugmode)
		{
			if (sb.currentSequence.isCursorValid(**sb.cursor))
			{
				if (level == Logs::Level::error or level == Logs::Level::ICE)
				{
					uint32_t offset = sb.currentSequence.offsetOf(**sb.cursor);
					auto h = entry.hint();
					h << "dump opcodes at +" << offset << "\n";
					auto* map = &(sb.cdeftable.originalTable().atoms);
					IR::ISA::printExtract(h.message.message, sb.currentSequence, offset, map);
				}
			}
		}
		return entry;
	}

	void SequenceBuilder::retriveReportMetadata(void* self, Logs::Level, const AST::Node*, String& filename, uint32_t& line, uint32_t& offset)
	{
		auto& sb = *(reinterpret_cast<SequenceBuilder*>(self));
		filename = sb.currentFilename;
		line     = sb.currentLine;
		offset   = sb.currentOffset;
	}




	YString SequenceBuilder::iceClassdef(const Classdef& cdef, const AnyString& msg) const
	{
		success = false;
		auto ce = (ice() << msg << ": " << cdef.clid << ' ');
		cdef.print(ce, cdeftable);
		return ce.data().message;
	}


	bool SequenceBuilder::complainRedeclared(const AnyString& name, uint32_t previousDeclaration)
	{
		success = false;
		auto err = (error() << "redeclaration of '" << name << '\'');
		err << " in '";
		err << cdeftable.keyword(frame->atom) << ' ';
		frame->atom.retrieveCaption(err.data().message, cdeftable);
		err << '\'';

		auto suggest = (err.hint() << "previous declaration of '" << name << '\'');
		auto& lr = frame->lvids[previousDeclaration];
		lr.fillLogEntryWithLocation(suggest);
		return false;
	}


	bool SequenceBuilder::complainMultipleOverloads(LVID lvid)
	{
		assert(frame != nullptr);
		auto err = (error() << "ambigous resolution");
		if (not frame->lvids[lvid].resolvedName.empty())
			err << " for '" << frame->lvids[lvid].resolvedName << "'";

		CLID clid{frame->atomid, lvid};
		if (debugmode)
			err << ' ' << clid;

		auto it = frame->partiallyResolved.find(clid);
		if (it != frame->partiallyResolved.end())
		{
			auto& solutions = it->second;
			for (size_t i = 0; i != solutions.size(); ++i)
			{
				auto& atom = solutions[i].get();
				auto hint = err.hint();
				hint.message.origins.location.resetFromAtom(atom);

				hint << '\'';
				hint << cdeftable.keyword(atom) << ' ';
				atom.retrieveCaption(hint.text(), cdeftable);
				hint << "' is a suitable candidate";
			}
		}
		return false;
	}


	bool SequenceBuilder::complainMultipleOverloads(LVID lvid, const std::vector<std::reference_wrapper<Atom>>& solutions
	   , const OverloadedFuncCallResolver& resolver)
	{
		assert(solutions.size() == resolver.suitable.size());

		success = false;

		// variable name
		String varname;
		for (auto l = frame->lvids[lvid].referer; l != 0; l = frame->lvids[l].referer)
		{
			auto& part = frame->lvids[l].resolvedName;
			if (part.empty())
				continue;

			if (not varname.empty())
				varname.prepend(".");

			if (part.first() != '^')
				varname.prepend(part);
			else
				varname << "operator " << AnyString{part, 1, part.size() - 1};
		}

		auto err = error();

		if (resolver.suitableCount > 0)
		{
			// generating the error
			err << "ambiguous call to overloaded function '" << varname;
			overloadMatch.printInputParameters(err.data().message);
			err << '\'';

			for (size_t i = 0; i != solutions.size(); ++i)
			{
				if (resolver.suitable[i])
				{
					auto& atom = solutions[i].get();
					auto hint = err.hint();
					hint.message.origins.location.resetFromAtom(atom);

					hint << '\'';
					hint << cdeftable.keyword(atom) << ' ';
					atom.retrieveCaption(hint.text(), cdeftable);
					hint << "' is a suitable candidate";

					hint.appendEntry(resolver.subreports[i]); // subreport can be null
				}
			}
		}
		else
		{
			err << "no matching function to call '" << varname;
			overloadMatch.printInputParameters(err.data().message);
			err << '\'';

			for (size_t i = 0; i != solutions.size(); ++i)
			{
				auto& atom = solutions[i].get();
				if (atom.flags(Atom::Flags::suggestInReport))
				{
					auto hint  = (err.hint() << "see ");
					hint.message.origins.location.resetFromAtom(atom);

					hint << '\'';
					hint << cdeftable.keyword(atom) << ' ';
					atom.retrieveCaption(hint.text(), cdeftable);
					hint << '\'';

					hint.appendEntry(resolver.subreports[i]); // subreport can be null
				}
			}
		}
		return false;
	}


	bool SequenceBuilder::complainMultipleDefinitions(const Atom& atom, const AnyString& funcOrOpName)
	{
		auto err = (error() << atom.keyword() << ' ');
		atom.retrieveFullname(err.data().message);
		err << ": multiple definition for " << funcOrOpName;
		return false;
	}


	bool SequenceBuilder::complainBuiltinIntrinsicDoesNotAccept(const AnyString& name, const AnyString& what)
	{
		error() << "builtin intrinsic '" << name << "' does not accept " << what;
		return false;
	}


	bool SequenceBuilder::complainUnknownIntrinsic(const AnyString& name)
	{
		error() << "unknown intrinsic '!!" << name << '\'';
		return false;
	}


	bool SequenceBuilder::complainIntrinsicWithNamedParameters(const AnyString& name)
	{
		assert(not pushedparams.func.named.empty() and "Uh ?");
		auto err = (error());
		err << "piko: intrinsic '" << name << "': named parameters are not allowed";
		if (debugmode)
		{
			for (auto nmparm: pushedparams.func.named)
				err.hint() << "parameter name '" << nmparm.name << '\'';
		}
		return false;
	}


	bool SequenceBuilder::complainIntrinsicWithGenTypeParameters(const AnyString& name)
	{
		error() << "intrinsic '" << name << "': generic type parameters are not allowed";
		return false;
	}


	bool SequenceBuilder::complainIntrinsicParameterCount(const AnyString& name, uint32_t count)
	{
		uint32_t c = static_cast<uint32_t>(pushedparams.func.indexed.size());
		assert(c != count);

		auto err = (error() << "intrinsic '" << name << "': ");
		if (unlikely(c > count))
			err << "too many parameters";
		else
			err << "not enough parameters";

		err << " (got " << c << " instead of " << count << ')';
		return false;
	}


	bool SequenceBuilder::complainIntrinsicParameter(const AnyString& name, uint32_t pindex, const Classdef& got,
		const AnyString& expected)
	{
		success = false;
		auto& element = pushedparams.func.indexed[pindex];

		auto err = (error() << "intrinsic '" << name << "': invalid type for parameter " << (1 + pindex) << ", got '");
		got.print(err, cdeftable);
		err << "', expected ";
		if (not expected.empty())
			err << expected;
		else
			err << "non-void builtin (__bool, __i32, ...)";

		err.message.origins.location.filename   = currentFilename;
		err.message.origins.location.pos.line   = element.line;
		err.message.origins.location.pos.offset = element.offset;
		return false;
	}


	bool SequenceBuilder::complainOperand(const IR::Instruction& operands, AnyString msg)
	{
		success = false;
		auto message = ice();
		// example: ice: unexpected opcode 'resolveAttribute': from 'ref %4 = resolve %3."()"'
		if (not msg.empty())
			message << msg;
		else
			message << "unexpected opcode";

		message << ": '" << IR::ISA::print(currentSequence, operands) << '\'';
		// stop reading the opcodes
		currentSequence.invalidateCursor(*cursor);
		return false;
	}


	void SequenceBuilder::complainUnusedVariable(const AtomStackFrame& frame, uint32_t lvid) const
	{
		auto& lvidinfo = frame.lvids[lvid];
		if (not lvidinfo.errorReported)
		{
			auto err = warning();

			lvidinfo.fillLogEntryWithLocation(err);
			err << "unused variable '" << lvidinfo.userDefinedName << "' in '";
			err << cdeftable.keyword(frame.atom) << ' ';
			frame.atom.retrieveCaption(err.data().message, cdeftable);
			err << '\'';
		}
	}


	bool SequenceBuilder::complainInvalidMemberRequestNonClass(const AnyString& name, nytype_t kind) const
	{
		assert(not name.empty());
		success = false;
		auto e = (error() << "request member '");
		if (name.first() == '^')
			e << "operator " << AnyString{name, 1, name.size() - 1};
		else
			e << name;
		e << "' on a non-class type '" << nany_type_to_cstring(kind) << '\'';
		return false;
	}


	bool SequenceBuilder::complainUnknownBuiltinType(const AnyString& name) const
	{
		error() << "unknown builtin type '" << name << '\'';
		return false;
	}


	bool SequenceBuilder::complainInvalidSelfRefForVariableAssignment(uint32_t lvid) const
	{
		assert(frame != nullptr);
		auto& cdef = cdeftable.classdef(CLID{frame->atomid, lvid});
		auto ce = (ice() << "invalid 'self' reference for variable assignment (self.= <expr>) ");
		cdef.print(ce.data().message, cdeftable, false);
		return false;
	}


	bool SequenceBuilder::complainMissingOperator(Atom& atom, const AnyString& name) const
	{
		auto ce = (ice() << "missing operator '" << name << "' for '");
		ce << atom.keyword() << ' ';
		atom.retrieveFullname(ce.data().message);
		ce << '\'';
		return false;
	}


	bool SequenceBuilder::complainInvalidType(const char* origin, const Classdef& from, const Classdef& to)
	{
		auto err = (error() << origin << "cannot convert '");
		if (debugmode)
			err << from.clid << ':';
		from.print(err.data().message, cdeftable, false);

		err << "' to '";

		if (debugmode)
			err << to.clid << ':';
		to.print(err.data().message, cdeftable, false);
		err << '\'';
		return false;
	}


	void SequenceBuilder::complainTypealiasCircularRef(const Atom& original, const Atom& responsible)
	{
		auto err = (error() << "cannot resolve type alias '");
		original.retrieveCaption(err.data().message, cdeftable);
		err << "': circular dependcy";

		if (responsible.atomid != original.atomid)
		{
			auto hint = (err.hint() << "when trying to resolve '");
			responsible.retrieveCaption(hint.data().message, cdeftable);
			hint << '\'';
		}
	}


	void SequenceBuilder::complainTypedefDeclaredAfter(const Atom& original, const Atom& responsible)
	{
		auto err = error() << "cannot resolve type alias '";
		original.retrieveCaption(err.data().message, cdeftable);
		err << "': type alias '";
		responsible.retrieveCaption(err.data().message, cdeftable);
		err << "' not resolved yet";

		auto hint = (err.hint() << "when trying to resolve '");
		responsible.retrieveCaption(hint.data().message, cdeftable);
		hint << '\'';
	}


	void SequenceBuilder::complainTypedefUnresolved(const Atom& original)
	{
		auto err = error() << "cannot resolve type alias '";
		original.retrieveCaption(err.data().message, cdeftable);
		err << "'";
	}


	bool SequenceBuilder::complainCannotCall(Atom& atom, FuncOverloadMatch& overloadMatch)
	{
		auto err = (error() << "cannot call '" << cdeftable.keyword(atom) << ' ');
		atom.retrieveCaption(err.data().message, cdeftable);
		err << '\'';

		// no match, re-launching the process with error-enabled logging for user-reporting
		overloadMatch.report = &err;
		overloadMatch.validateWithErrReport(atom);
		return false;
	}


	void SequenceBuilder::complainReturnTypeMissing(const Classdef* expected, const Classdef* usertype)
	{
		if (usertype != nullptr or expected != nullptr)
		{
			// one of the return type is void, but not both
			if (usertype)
			{
				error() << "return-statement with a value, in function returning 'void'";
			}

			if (expected)
			{
				String tstr;
				expected->print(tstr, cdeftable);
				error() << "return-statement with no value, in function returning '" << tstr << '\'';
				return;
			}
		}
		else
			ice() << "mismatch return";
	}


	void SequenceBuilder::complainReturnTypeMismatch(const Classdef& expected, const Classdef& usertype)
	{
		auto err = (error() << "type mismatch in 'return' statement");
		auto hint = err.hint();
		if (debugmode)
		{
			usertype.print((hint << "got '%" << usertype.clid << ':'), cdeftable);
			expected.print((hint << "', expected '%" << expected.clid << ':'), cdeftable);
		}
		else
		{
			usertype.print((hint << "got '"), cdeftable);
			expected.print((hint << "', expected '"), cdeftable);
		}
		hint << '\'';
	}


	void SequenceBuilder::complainReturnTypeImplicitConv(const Classdef& expected, const Classdef& usertype, uint32_t line, uint32_t offset)
	{
		auto err = (error() << "implicit conversion is not allowed in 'return'");
		auto hint = err.hint();
		if (debugmode)
		{
			usertype.print((hint << "got '%" << usertype.clid << ':'), cdeftable);
			expected.print((hint << "', expected '%" << expected.clid << ':'), cdeftable);
		}
		else
		{
			usertype.print((hint << "got '"), cdeftable);
			expected.print((hint << "', expected '"), cdeftable);
		}
		hint << '\'';

		if (line != 0)
			hint.origins().location.pos.line   = line;
		if (offset != 0)
			hint.origins().location.pos.offset = offset;
	}


	void SequenceBuilder::complainReturnTypeMultiple(const Classdef& expected, const Classdef& usertype, uint32_t line, uint32_t offset)
	{
		auto err = (error() << "multiple return types in 'return'");
		auto hint = err.hint();

		if (debugmode)
		{
			usertype.print((hint << "got '%" << usertype.clid << ':'), cdeftable);
			expected.print((hint << "', expected '%" << expected.clid << ':'), cdeftable);
		}
		else
		{
			usertype.print((hint << "got '"), cdeftable);
			expected.print((hint << "', expected '"), cdeftable);
		}
		hint << '\'';

		if (line != 0)
			hint.origins().location.pos.line   = line;
		if (offset != 0)
			hint.origins().location.pos.offset = offset;
	}


	void SequenceBuilder::complainPushedSynthetic(const CLID& clid, uint32_t paramindex, const AnyString& paramname)
	{
		auto err = error();
		err << "cannot push synthetic object '";

		if (debugmode)
			err << clid << ' ';

		auto& cdef = cdeftable.classdef(clid);
		cdef.print(err.message.message, cdeftable, false);

		if (paramname.empty())
			err << "' for parameter " << paramindex;
		else
			err << "' for parameter '" << paramname << '\'';
	}


	void SequenceBuilder::complainInvalidParametersAfterSignatureMatching(Atom& atom, FuncOverloadMatch& overloadMatch)
	{
		// fail - try again to produce error message, hint, and any suggestion
		auto err = (error());
		err << "invalid parameters, failed to instanciate '" << atom.keyword() << ' ';
		atom.retrieveFullname(err.data().message);
		err << '\'';

		overloadMatch.report = &err;
		overloadMatch.validateWithErrReport(atom);
	}





} // namespace Instanciate
} // namespace Pass
} // namespace Nany
