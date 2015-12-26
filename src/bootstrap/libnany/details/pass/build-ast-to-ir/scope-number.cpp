#include "details/pass/build-ast-to-ir/scope.h"
#include "details/grammar/nany.h"

using namespace Yuni;




namespace Nany
{
namespace IR
{
namespace Producer
{

	namespace // anonymous
	{

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




	bool Scope::visitASTExprNumber(Node& node, yuint32& localvar)
	{
		assert(node.rule == rgNumber);
		assert(not node.children.empty());

		bool success = true;

		//! Is the number signed or unsigned ?
		bool isUnsigned = false;
		//! Is the number a floating-point number ?
		bool isFloat = false;
		//! how many bits used by the number ? (32 by default)
		uint bits = 32;
		//! Sign of the number: ' ', '+', '-'
		char sign = ' ';

		//! The first part of the number
		uint64 part1 = 0;
		//! Second part of the number
		AnyString part2; // the second part may have additional zero


		for (auto& childptr: node.children)
		{
			switch (childptr->rule)
			{
				case rgNumberValue: // standard number definition
				{
					bool firstPart = true;
					for (auto& subnodeptr: childptr->children)
					{
						switch (subnodeptr->rule)
						{
							case rgInteger:
							{
								if (likely(firstPart))
								{
									part1 = subnodeptr->text.to<uint64>();
									firstPart = false;
								}
								else
								{
									part2 = subnodeptr->text;
									// as far as we know, it is a float64 by default
									isFloat = true;
									bits = 64;
								}
								break;
							}

							default:
							{
								ICE(*subnodeptr) << "[expr-number]";
								success = false;
							}
						}
					}
					break;
				}

				case rgNumberSign: // + -
				{
					assert(not childptr->text.empty() and "invalid ast");
					sign = childptr->text[0];
					assert(sign == '+' or sign == '-');
					break;
				}

				case rgNumberQualifier: // unsigned / signed / float
				{
					for (auto& subnodeptr: childptr->children)
					{
						auto& subnode = *(subnodeptr);
						switch (subnode.rule)
						{
							case rgNumberQualifierType:
							{
								for (uint i = subnode.text.size(); i--; )
								{
									switch (subnode.text[i])
									{
										case 'i': isUnsigned = false; break;
										case 'u': isUnsigned = true; break;
										case 'f': isFloat = true; break;
										default: success = false; error(subnode) << "invalid number qualifier";
									}
								}
								break;
							}
							case rgInteger:
							{
								if (likely(subnode.text.size() < 10))
								{
									bits = subnode.text.to<uint>();
									if (unlikely(bits != 8 and bits != 16 and bits != 32 and bits != 64))
									{
										error(subnode) << "invalid integer size";
										success = false;
									}
								}
								else
								{
									error(subnode) << "integer too large";
									success = false;
								}
								break;
							}
							default:
							{
								ICE(subnode) << "[expr-number]";
								success = false;
							}
						}
					}

					break;
				}

				default:
				{
					ICE(*childptr) << "[expr-number]";
					success = false;
				}
			}
		}


		// checking for invalid float values

		if (not isFloat)
		{
			nytype_t type = nyt_void;
			switch (bits)
			{
				case 64: type = (isUnsigned) ? nyt_u64 : nyt_i64; break;
				case 32: type = (isUnsigned) ? nyt_u32 : nyt_i32; break;
				case 16: type = (isUnsigned) ? nyt_u16 : nyt_i16; break;
				case  8: type = (isUnsigned) ? nyt_u8  : nyt_i8; break;
			}

			localvar = createLocalBuiltinInt64(node, type, part1);
		}
		else
		{
			if (bits != 32 and bits != 64)
			{
				error(node) << "a floatting point number can only be 32 or 64 bits";
				success = false;
				bits = 64; // default value
			}

			// converting the number into a double
			double value;
			if (unlikely(not convertASTNumberToDouble(value, part1, part2, sign)))
			{
				error(node) << "invalid floating point number";
				success = false;
			}

			nytype_t type = (bits == 32) ? nyt_f32 : nyt_f64;
			localvar = createLocalBuiltinFloat64(node, type, value);
		}

		return success;
	}






} // namespace Producer
} // namespace IR
} // namespace Nany
