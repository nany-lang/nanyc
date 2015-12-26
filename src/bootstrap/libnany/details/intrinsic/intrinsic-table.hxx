#pragma once
#include "intrinsic-table.h"



namespace Nany
{

	template<class T> struct CTypeToNanyType;

	template<> struct CTypeToNanyType<std::int8_t> final
	{
		static constexpr nytype_t type = nyt_i8;
	};

	template<> struct CTypeToNanyType<std::int16_t> final
	{
		static constexpr nytype_t type = nyt_i16;
	};

	template<> struct CTypeToNanyType<std::int32_t> final
	{
		static constexpr nytype_t type = nyt_i32;
	};

	template<> struct CTypeToNanyType<std::int64_t> final
	{
		static constexpr nytype_t type = nyt_i64;
	};

	template<> struct CTypeToNanyType<std::uint8_t> final
	{
		static constexpr nytype_t type = nyt_u8;
	};

	template<> struct CTypeToNanyType<std::uint16_t> final
	{
		static constexpr nytype_t type = nyt_u16;
	};

	template<> struct CTypeToNanyType<std::uint32_t> final
	{
		static constexpr nytype_t type = nyt_u32;
	};

	template<> struct CTypeToNanyType<std::uint64_t> final
	{
		static constexpr nytype_t type = nyt_u64;
	};

	template<> struct CTypeToNanyType<bool> final
	{
		static constexpr nytype_t type = nyt_bool;
	};

	template<class P> struct CTypeToNanyType<P*> final
	{
		static constexpr nytype_t type = nyt_pointer;
	};

	template<class P> struct CTypeToNanyType<const P*> final
	{
		static constexpr nytype_t type = nyt_pointer;
	};

	template<> struct CTypeToNanyType<Yuni::None> final
	{
		static constexpr nytype_t type = nyt_void;
	};

	template<uint N, uint Max, class T> struct IntrinsicPushParameter final
	{
		static void push(Intrinsic& intrinsic)
		{
			intrinsic.params[N] = CTypeToNanyType<typename T::template Argument<N>::Type>::type;
			IntrinsicPushParameter<N + 1, Max, T>::push(intrinsic);
		}
	};

	template<uint N, class T> struct IntrinsicPushParameter<N, N, T> final
	{
		static void push(Intrinsic&) {}
	};


	template<class T>
	inline bool IntrinsicTable::add(const AnyString& name, T callback)
	{
		typedef Yuni::Bind<T> B;
		if (not name.empty())
		{
			pIntrinsics.emplace_back(new Intrinsic(name));
			auto& intrinsic = *(pIntrinsics.back());
			pByNames.insert(std::make_pair(AnyString{intrinsic.name}, std::ref(intrinsic)));

			// return type / parameters
			if (B::hasReturnValue)
				intrinsic.rettype = CTypeToNanyType<typename B::ReturnType>::type;

			if (B::argumentCount > 0)
				IntrinsicPushParameter<0, B::argumentCount, B>::push(intrinsic);
			intrinsic.paramcount = static_cast<uint>(B::argumentCount);
			intrinsic.callback = reinterpret_cast<void*>(callback);
			return true;
		}
		return false;
	}


	inline bool IntrinsicTable::exists(const AnyString& name) const
	{
		return (0 != pByNames.count(name));
	}


	inline bool IntrinsicTable::empty() const
	{
		return pIntrinsics.empty();
	}


	inline void IntrinsicTable::registerStdCore()
	{
		registerBool();
	}


	inline const Intrinsic* IntrinsicTable::find(const AnyString& name) const
	{
		auto it = pByNames.find(name);
		return (it != pByNames.end()) ? &(it->second.get()) : nullptr;
	}



} // namespace Nany
