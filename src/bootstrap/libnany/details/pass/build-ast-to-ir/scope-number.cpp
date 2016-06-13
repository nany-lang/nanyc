#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"
#include "details/ast/ast.h"
#include <limits>
#include "libnany-config.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{

	namespace // anonymous
	{

		struct NumberDef final
		{
			// Is the number signed or unsigned ?
			bool isUnsigned = false;
			// Is the number a floating-point number ?
			bool isFloat = false;
			// how many bits used by the number ? (32 by default)
			uint bits = 32;
			// Sign of the number: ' ', '+', '-'
			char sign = ' ';

			//! The first part of the number
			uint64_t part1 = 0;
			//! Second part of the number
			AnyString part2; // the second part may have additional zero
		};


		static constexpr inline bool validNumberOfBits(uint32_t bits)
		{
			return bits == 32 or bits == 64 or bits == 16 or bits == 8;
		}


		static bool convertASTNumberToDouble(double& value, uint64 part1, const AnyString& part2, char sign)
		{
			if (part1 == 0 and part2.empty()) // obvious reason
			{
				value = 0.;
				return true;
			}

			ShortString128 tmp;
			if (sign == '-') // append the sign of the number
				tmp += '-';

			tmp << part1;
			if (not part2.empty())
				tmp << '.' << part2;

			if (unlikely(not tmp.to<double>(value)))
			{
				value = 0.;
				return false;
			}
			return true;
		}


	} // anonymous namespace






	template<bool BuiltinT, class DefT>
	inline bool Scope::generateNumberCode(uint32_t& localvar, const DefT& numdef, const AST::Node& node)
	{
		// checking for invalid float values
		nytype_t type = nyt_void;
		// class name
		AnyString cn;
		uint32_t hardcodedlvid;

		if (not numdef.isFloat)
		{
			if (not numdef.isUnsigned)
			{
				switch (numdef.bits)
				{
					case 64: type = nyt_i64; if (not BuiltinT) cn = "i64"; break;
					case 32: type = nyt_i32; if (not BuiltinT) cn = "i32"; break;
					case 16: type = nyt_i16; if (not BuiltinT) cn = "i16"; break;
					case  8: type = nyt_i8;  if (not BuiltinT) cn = "i8";  break;
				}
			}
			else
			{
				switch (numdef.bits)
				{
					case 64: type = nyt_u64; if (not BuiltinT) cn = "u64"; break;
					case 32: type = nyt_u32; if (not BuiltinT) cn = "u32"; break;
					case 16: type = nyt_u16; if (not BuiltinT) cn = "u16"; break;
					case  8: type = nyt_u8;  if (not BuiltinT) cn = "u8";  break;
				}
			}

			if (numdef.part1 != 0)
			{
				bool invalidcast = false;

				if (numdef.sign == ' ' or numdef.sign == '+')
				{
					if (not numdef.isUnsigned)
					{
						switch (numdef.bits)
						{
							case 64: invalidcast = (numdef.part1 > std::numeric_limits<int32_t>::max()); break;
							case 32: invalidcast = (numdef.part1 > std::numeric_limits<int32_t>::max()); break;
							case 16: invalidcast = (numdef.part1 > std::numeric_limits<int16_t>::max()); break;
							case  8: invalidcast = (numdef.part1 > std::numeric_limits< int8_t>::max()); break;
						}
					}
					else
					{
						switch (numdef.bits)
						{
							case 64: break;
							case 32: invalidcast = (numdef.part1 > std::numeric_limits<uint32_t>::max()); break;
							case 16: invalidcast = (numdef.part1 > std::numeric_limits<uint16_t>::max()); break;
							case  8: invalidcast = (numdef.part1 > std::numeric_limits< uint8_t>::max()); break;
						}
					}
				}
				else
				{
					if (not numdef.isUnsigned)
					{
						if (numdef.part1 < static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
						{
							int64_t sv = static_cast<int64_t>(numdef.part1);
							switch (numdef.bits)
							{
								case 64: invalidcast = (sv < std::numeric_limits<int32_t>::min()); break;
								case 32: invalidcast = (sv < std::numeric_limits<int32_t>::min()); break;
								case 16: invalidcast = (sv < std::numeric_limits<int16_t>::min()); break;
								case  8: invalidcast = (sv < std::numeric_limits< int8_t>::min()); break;
							}
						}
						else
							invalidcast = true;
					}
					else
						invalidcast = true;
				}

				if (unlikely(invalidcast))
				{
					error(node) << "invalid " << ((numdef.isUnsigned) ? "unsigned " : "signed ")
						<< numdef.bits << "bits integer";
					return false;
				}
			}

			hardcodedlvid = createLocalBuiltinInt64(node, type, numdef.part1);
		}
		else
		{
			// converting the number into a double
			double value;
			if (unlikely(not convertASTNumberToDouble(value, numdef.part1, numdef.part2, numdef.sign)))
				return (error(node) << "invalid floating point number");

			type = (numdef.bits == 32) ? nyt_f32 : nyt_f64;
			if (not BuiltinT)
				cn.adapt((numdef.bits == 32) ? "f32" : "f64", 3);
			hardcodedlvid = createLocalBuiltinFloat64(node, type, value);
		}


		if (BuiltinT)
		{
			localvar = hardcodedlvid;
			return true;
		}
		else
		{
			if (!context.reuse.literal.node)
				context.prepareReuseForLiterals();

			assert(not cn.empty());
			context.reuse.literal.classname->text = cn;
			ShortString16 lvidstr;
			lvidstr = hardcodedlvid;
			context.reuse.literal.lvidnode->text = lvidstr;

			bool success = visitASTExprNew(*(context.reuse.literal.node), localvar);
			//if (debugmode)
			//{
			//	// to not print invalid data in the debugger
			//	context.reuse.literal.classname->text.clear();
			//	context.reuse.literal.lvidnode->text.clear();
			//}
			return success;
		}
	}




	bool Scope::visitASTExprNumber(const AST::Node& node, yuint32& localvar)
	{
		assert(node.rule == AST::rgNumber);
		assert(not node.children.empty());

		// Number definition
		NumberDef numdef;
		// is a builtin ? (__i32, __f64...)
		// (always generate builtin types when not importing the NSL)
		bool builtin = (not Config::importNSL);

		emitDebugpos(node);

		for (auto& childptr: node.children)
		{
			switch (childptr->rule)
			{
				case AST::rgNumberValue: // standard number definition
				{
					bool firstPart = true;
					for (auto& subnodeptr: childptr->children)
					{
						switch (subnodeptr->rule)
						{
							case AST::rgInteger:
							{
								if (likely(firstPart))
								{
									if (not subnodeptr->text.to<uint64>(numdef.part1))
									{
										error(*subnodeptr) << "invalid integer value";
										return false;
									}
									firstPart = false;
								}
								else
								{
									numdef.part2 = subnodeptr->text;
									numdef.part2.trimRight('0'); // remove useless zeros
									// as far as we know, it is a float64 by default
									numdef.isFloat = true;
								}
								break;
							}

							default:
							{
								ice(*subnodeptr) << "[expr-number]";
								return false;
							}
						}
					}
					break;
				}

				case AST::rgNumberSign: // + -
				{
					assert(not childptr->text.empty() and "invalid ast");
					numdef.sign = childptr->text[0];
					assert(numdef.sign == '+' or numdef.sign == '-');
					break;
				}

				case AST::rgNumberQualifier: // unsigned / signed / float
				{
					for (auto& subnodeptr: childptr->children)
					{
						auto& subnode = *(subnodeptr);
						switch (subnode.rule)
						{
							case AST::rgNumberQualifierType:
							{
								assert(not subnode.text.empty());
								uint offset = 0;
								if (subnode.text.first() == '_' and subnode.text[1] == '_')
								{
									offset = 2;
									builtin = true;
								}
								for (uint i = offset; i < subnode.text.size(); ++i)
								{
									switch (subnode.text[i])
									{
										case 'i': numdef.isUnsigned = false; break;
										case 'u': numdef.isUnsigned = true; break;
										case 'f': numdef.isFloat = true; break;
										default: error(node) << "invalid number qualifier. Got '"
											<< subnode.text.first() << "', expected 'i', 'u' or 'f'";
									}
								}
								break;
							}
							case AST::rgInteger:
							{
								if (likely(subnode.text.size() < 10))
								{
									numdef.bits = subnode.text.to<uint>();
									if (unlikely(not validNumberOfBits(numdef.bits)))
										return (error(subnode) << "invalid integer size");
								}
								else
								{
									error(subnode) << "integer too large";
									return false;
								}
								break;
							}
							default:
							{
								ice(subnode) << "[expr-number]";
								return false;
							}
						}
					}

					break;
				}

				default:
				{
					ice(*childptr) << "[expr-number]";
					return false;
				}
			}
		}


		assert(numdef.bits == 64 or numdef.bits == 32 or numdef.bits == 16 or numdef.bits == 8);

		return (not builtin)
			? generateNumberCode<false>(localvar, numdef, node)
			: generateNumberCode<true> (localvar, numdef, node);
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
