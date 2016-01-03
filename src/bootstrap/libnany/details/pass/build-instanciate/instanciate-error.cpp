#include "instanciate.h"

using namespace Yuni;




namespace Nany
{
namespace Pass
{
namespace Instanciate
{

	Logs::Report ProgramBuilder::error() const
	{
		success = false;
		auto err = report.error();
		err.message.origins.location.filename   = currentFilename;
		err.message.origins.location.pos.line   = currentLine;
		err.message.origins.location.pos.offset = currentOffset;
		return err;
	}

	Logs::Report ProgramBuilder::warning() const
	{
		auto wrn = report.warning();
		wrn.message.origins.location.filename   = currentFilename;
		wrn.message.origins.location.pos.line   = currentLine;
		wrn.message.origins.location.pos.offset = currentOffset;
		return wrn;
	}


	Logs::Report ProgramBuilder::ICE() const
	{
		success = false;
		auto ice = report.ICE();
		ice.message.origins.location.filename   = currentFilename;
		ice.message.origins.location.pos.line   = currentLine;
		ice.message.origins.location.pos.offset = currentOffset;

		// assert on the variable 'success' to not make the compiler
		// complain on the attribute ‘noreturn’
		//assert(success and "ICE raise");
		return ice;
	}


	YString ProgramBuilder::ICE(const Classdef& cdef, const AnyString& msg) const
	{
		success = false;
		auto ice = (ICE() << msg << ": " << cdef.clid << ' ');
		cdef.print(ice, cdeftable);
		return ice.data().message;
	}


	bool ProgramBuilder::complainRedeclared(const AnyString& name, uint32_t previousDeclaration)
	{
		success = false;
		auto& frame = atomStack.back();
		auto err = (error() << "redeclaration of '" << name << '\'');
		err << " in '";
		err << cdeftable.keyword(frame.atom) << ' ';
		frame.atom.appendCaption(err.data().message, cdeftable);
		err << '\'';

		auto suggest = (err.hint() << "previous declaration of '" << name << '\'');
		auto& lr = frame.lvids[previousDeclaration];
		lr.fillLogEntryWithLocation(suggest);
		return false;
	}


	bool ProgramBuilder::complainMultipleOverloads(LVID lvid, const std::vector<std::reference_wrapper<Atom>>& solutions
	   , const OverloadedFuncCallResolver& resolver)
	{
		assert(solutions.size() == resolver.suitable.size());

		success = false;

		// the current frame
		auto& frame = atomStack.back();
		// variable name
		String varname;
		for (auto l = frame.lvids[lvid].referer; l != 0; l = frame.lvids[l].referer)
		{
			auto& part = frame.lvids[l].resolvedName;
			if (part.empty())
				continue;

			if (not varname.empty())
				varname.prepend(".");
			varname.prepend(part);
		}

		auto err = error();

		if (resolver.suitableCount != 0)
		{
			// generating the error
			err << "ambiguous call to overloaded function '" << varname << '(';
			overloadMatch.printInputParameters(err.data().message);
			err << ")'";

			for (uint i = 0; i != (uint) solutions.size(); ++i)
			{
				if (not resolver.suitable[i])
					continue;

				auto& atom = solutions[i].get();
				auto hint = err.hint();
				hint.message.origins.location.filename   = atom.origin.filename;
				hint.message.origins.location.pos.line   = atom.origin.line;
				hint.message.origins.location.pos.offset = atom.origin.offset;

				hint << '\'';
				hint << cdeftable.keyword(atom) << ' ';
				atom.appendCaption(hint.text(), cdeftable);
				hint << "' is a suitable candidate";

				hint.appendEntry(resolver.subreports[i]); // subreport can be null
			}
		}
		else
		{
			err << "no matching function to call '" << varname << '(';
			overloadMatch.printInputParameters(err.data().message);
			err << ")'";

			for (uint i = 0; i != solutions.size(); ++i)
			{
				auto& atom = solutions[i].get();
				auto hint  = (err.hint() << "see ");
				hint.message.origins.location.filename   = atom.origin.filename;
				hint.message.origins.location.pos.line   = atom.origin.line;
				hint.message.origins.location.pos.offset = atom.origin.offset;

				hint << '\'';
				hint << cdeftable.keyword(atom) << ' ';
				atom.appendCaption(hint.text(), cdeftable);
				hint << '\'';

				hint.appendEntry(resolver.subreports[i]); // subreport can be null
			}
		}
		return false;
	}


	bool ProgramBuilder::checkForIntrinsicParamCount(const AnyString& name, uint32_t count)
	{
		if (unlikely(lastPushedIndexedParameters.size() > count))
		{
			error() << "intrinsic '" << name << "': too many parameters (got "
				<< lastPushedIndexedParameters.size()
				<< " instead of " << count << ')';
			return false;
		}

		if (unlikely(lastPushedIndexedParameters.size() < count))
		{
			error() << "intrinsic '" << name << "': not enough parameters (got "
				<< lastPushedIndexedParameters.size()
				<< " instead of " << count << ')';
			return false;
		}
		return true;
	}


	bool ProgramBuilder::complainIntrinsicParameter(const AnyString& name, uint32_t pindex, const Classdef& got, const AnyString& expected)
	{
		success = false;
		auto& element = lastPushedIndexedParameters[pindex];

		auto err = (error() << "intrinsic '" << name << "': invalid type for parameter " << (1 + pindex) << ", got '");
		got.print(err, cdeftable);
		err << "', expected " << expected;
		err.message.origins.location.filename   = currentFilename;
		err.message.origins.location.pos.line   = element.line;
		err.message.origins.location.pos.offset = element.offset;
		return false;
	}


	bool ProgramBuilder::complainOperand(const IR::Instruction& operands, AnyString msg)
	{
		success = false;
		auto message = ICE();
		// example: ICE: unexpected opcode 'resolveAttribute': from 'ref %4 = resolve %3."()"'
		if (not msg.empty())
			message << msg;
		else
			message << "unexpected opcode";

		message << ": '" << IR::ISA::print(currentProgram, operands) << '\'';
		// stop reading the opcodes
		currentProgram.invalidateCursor(*cursor);
		return false;
	}


	void ProgramBuilder::complainUnusedVariable(const AtomStackFrame& frame, uint32_t lvid) const
	{
		auto& lvidinfo = frame.lvids[lvid];
		auto err = warning();

		lvidinfo.fillLogEntryWithLocation(err);
		err << "unused variable '" << lvidinfo.userDefinedName << "' in '";
		err << cdeftable.keyword(frame.atom) << ' ';
		frame.atom.appendCaption(err.data().message, cdeftable);
		err << '\'';
	}


	bool ProgramBuilder::complainInvalidMemberRequestNonClass(const AnyString& name, nytype_t kind) const
	{
		assert(not name.empty());
		success = false;
		auto e = (error() << "request member '");
		if (name.first() == '^')
			e << "operator " << AnyString{name, 1, name.size() - 1};
		else
			e << name;
		e << "' on a non-class type '"
			<< nany_type_to_cstring(kind) << '\'';
		return false;
	}


	bool ProgramBuilder::complainUnknownBuiltinType(const AnyString& name) const
	{
		error() << "unknown builtin type '" << name << '\'';
		return false;
	}


	bool ProgramBuilder::complainInvalidSelfRefForVariableAssignment(uint32_t lvid) const
	{
		assert(not atomStack.empty());
		auto& frame = atomStack.back();
		auto& cdef = cdeftable.classdef(CLID{frame.atomid, lvid});
		auto ice = (ICE() << "invalid 'self' reference for variable assignment (self.= <expr>) ");
		cdef.print(ice.data().message, cdeftable, false);
		return false;
	}


	bool ProgramBuilder::complainMissingOperator(Atom& atom, const AnyString& name) const
	{
		auto ice = (ICE() << "missing operator '" << name << "' for '");
		ice << atom.keyword() << ' ';
		atom.printFullname(ice.data().message, false);
		ice << '\'';
		return false;
	}




} // namespace Instanciate
} // namespace Pass
} // namespace Nany
