#pragma once
#include "intrinsic-table.h"
#include <yuni/core/static/types.h>


namespace ny
{

	template<class T> struct CTypeToNanyType {};

	template<> struct CTypeToNanyType<int8_t> final
	{
		static constexpr nytype_t type = nyt_i8;
	};

	template<> struct CTypeToNanyType<int16_t> final
	{
		static constexpr nytype_t type = nyt_i16;
	};

	template<> struct CTypeToNanyType<int32_t> final
	{
		static constexpr nytype_t type = nyt_i32;
	};

	template<> struct CTypeToNanyType<int64_t> final
	{
		static constexpr nytype_t type = nyt_i64;
	};

	template<> struct CTypeToNanyType<uint8_t> final
	{
		static constexpr nytype_t type = nyt_u8;
	};

	template<> struct CTypeToNanyType<uint16_t> final
	{
		static constexpr nytype_t type = nyt_u16;
	};

	template<> struct CTypeToNanyType<uint32_t> final
	{
		static constexpr nytype_t type = nyt_u32;
	};

	template<> struct CTypeToNanyType<uint64_t> final
	{
		static constexpr nytype_t type = nyt_u64;
	};

	template<> struct CTypeToNanyType<float> final
	{
		static constexpr nytype_t type = nyt_f32;
	};

	template<> struct CTypeToNanyType<double> final
	{
		static constexpr nytype_t type = nyt_f64;
	};

	template<> struct CTypeToNanyType<bool> final
	{
		static constexpr nytype_t type = nyt_bool;
	};

	template<class P> struct CTypeToNanyType<P*> final
	{
		static constexpr nytype_t type = nyt_ptr;
	};

	template<class P> struct CTypeToNanyType<const P*> final
	{
		static constexpr nytype_t type = nyt_ptr;
	};

	template<> struct CTypeToNanyType<void> final
	{
		static constexpr nytype_t type = nyt_void;
	};

	template<uint N, uint Max, class T> struct IntrinsicPushParameter final
	{
		static void push(Intrinsic& intrinsic)
		{
			static_assert(N > 0, "the first param is reserved for context");
			intrinsic.params[N - 1] = CTypeToNanyType<typename T::template Argument<N>::Type>::type;
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
		if (YUNI_UNLIKELY(name.empty() or (0 != pByNames.count(name))))
			return false;

		using B = Yuni::Bind<T>;
		static_assert(B::argumentCount < Config::maxPushedParameters, "too many params");

		Intrinsic::Ptr ptr = new Intrinsic(name, reinterpret_cast<void*>(callback));
		pIntrinsics.emplace_back(ptr);
		auto& intrinsic = *ptr;
		pByNames.insert(std::make_pair(AnyString{intrinsic.name}, &intrinsic));
		intrinsic.id = ((uint32_t)pIntrinsics.size() - 1);

		// return type / parameters
		if (B::hasReturnValue)
			intrinsic.rettype = CTypeToNanyType<typename B::ReturnType>::type;

		// the first argument must be the thread context
		static_assert(Yuni::Static::Type::Equal<
			typename B::template Argument<0>::Type, nyvm_t*>::Yes, "requires 'nytctx_t*'");

		if (B::argumentCount > 1)
		{
			IntrinsicPushParameter<1, B::argumentCount, B>::push(intrinsic);
			intrinsic.paramcount = static_cast<uint32_t>(B::argumentCount - 1);
		}
		return true;
	}


	inline bool IntrinsicTable::exists(const AnyString& name) const
	{
		return (0 != pByNames.count(name));
	}


	inline bool IntrinsicTable::empty() const
	{
		return pIntrinsics.empty();
	}


	inline uint32_t IntrinsicTable::size() const
	{
		return static_cast<uint32_t>(pIntrinsics.size());
	}


	inline Intrinsic::Ptr IntrinsicTable::find(const AnyString& name) const
	{
		auto it = pByNames.find(name);
		return (it != pByNames.end()) ? it->second : nullptr;
	}


	inline const Intrinsic& IntrinsicTable::operator [] (uint32_t id) const
	{
		assert(id < pIntrinsics.size());
		return *(pIntrinsics[id]);
	}


} // namespace ny
