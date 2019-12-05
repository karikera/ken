#pragma once

namespace kr
{
	namespace meta
	{
		namespace _pri_
		{
			template <size_t count, typename newnums, size_t ... nums> struct number_resize;

			template <size_t count, size_t fnum, size_t ... nums, size_t ... newnums>
			struct number_resize<count, numbers<newnums ...>, fnum, nums ...> :
				number_resize<count - 1, numbers<newnums ..., fnum>, nums ...>
			{
			};

			template <size_t count, size_t ... newnums>
			struct number_resize<count, numbers<newnums ...>> :
				number_resize<count - 1, numbers<newnums ..., sizeof ... (newnums)>>
			{
			};

			template <size_t fnum, size_t ... nums, size_t ... newnums>
			struct number_resize<0, numbers<newnums ...>, fnum, nums ...>
			{
				using type = numbers<newnums ...>;
			};

			template <size_t ... newnums>
			struct number_resize<0, numbers<newnums ...>>
			{
				using type = numbers<newnums ...>;
			};


			template <typename dorder, typename sorder> struct _order_util;
			template <size_t ... dstOrders, size_t ... srcOrders> struct _order_util<numbers<srcOrders ...>, numbers<dstOrders ...> >
			{
				template <typename TD, typename TS>
				static void copy(_Out_ TD * dest, _In_ const TS * src) noexcept
				{
					unpack(dest[dstOrders] = (TD)src[srcOrders]);
				}
			};
		}
		
		template <size_t ... nums>
		struct numbers
		{
			template <size_t count>
			using resize = typename _pri_::number_resize<count, numbers<>, nums ...>::type;
		};

		template <typename dorder, typename sorder, size_t count> 
		struct order_util
			:_pri_::_order_util<typename dorder::template resize<count>, typename sorder::template resize<count>>
		{
		};

	}
}
