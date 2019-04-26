#pragma once

namespace kr
{
	namespace meta
	{
		template <class base> struct attacher
		{
			template <bool b, template <class> class cls> 
			struct attach_impl;

			template <bool b2, template <class> class cls2>
			using attach = attach_impl<b2, cls2>;

			template <template <class> class cls>
			struct attach_impl<false, cls>
			{
				using type = base;
				template <bool b2, template <class> class cls2> 
				using attach = typename attacher<type>::template attach<b2, cls2>;
			};;
			template <template <class> class cls> 
			struct attach_impl<true, cls>
			{
				using type = cls<base>;
				template <bool b2, template <class> class cls2> 
				using attach = typename attacher<type>::template attach<b2,cls2>;
			};;
		};

	}
}