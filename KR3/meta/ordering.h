#pragma once

namespace kr
{
	namespace meta
	{
		template <class parent>
		struct crtp_order
		{
			template <template <class> class crtp, bool condition>
			struct attach_impl;

			template <template <class> class next_crtp, bool condition>
			using attach = attach_impl<next_crtp, condition>;

			template <template <class> class crtp>
			struct attach_impl<crtp, true>
			{
				using type = crtp<parent>;

				template <template <class> class next_crtp, bool condition>
				using attach = typename crtp_order<type>::template attach<next_crtp, condition>;
			};
			template <template <class> class crtp>
			struct attach_impl<crtp, false>
			{
				using type = parent;

				template <template <class> class next_crtp, bool condition>
				using attach = typename crtp_order<type>::template attach<next_crtp, condition>;
			};
		};
	}
}