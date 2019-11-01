#pragma once

#include "../main.h"
#include "math.h"
#include "if.h"

namespace kr
{
	namespace meta
	{
		// type list
		template <size_t... args> struct numlist;
		template <typename ... args> class types;
		using types_empty = types<>;
		namespace _pri_
		{
			template <typename numlist_from, typename numlist_to = numlist<>>
			struct numlist_remove_back;

			template <size_t firstnum, size_t ... numlist_from, size_t ... numlist_to>
			struct numlist_remove_back<numlist<firstnum, numlist_from...>, numlist<numlist_to...>>
				:numlist_remove_back<numlist<numlist_from ...>, numlist<numlist_to ..., firstnum>>
			{
			};
			template <size_t firstnum, size_t ... numlist_to>
			struct numlist_remove_back<numlist<firstnum>, numlist<numlist_to...>>
			{
				static constexpr size_t value = firstnum;
				using type = numlist<numlist_to ...>;
			};
			template <>
			struct numlist_remove_back<numlist<>, numlist<>>
			{
				// invalid numlist_remove_back
				// static constexpr size_t value = -1;
				// using type = numlist<>;
			};
			template <typename numbers>
			struct numlist_remove_front;
			template <size_t firstnum, size_t ... numbers>
			struct numlist_remove_front<numlist<firstnum, numbers...>>
			{
				static constexpr size_t value = firstnum;
				using type = numlist<numbers ...>;
			};
			template <>
			struct numlist_remove_front<numlist<>>
			{
				static constexpr size_t value = -1;
				using type = numlist<>;
			};
			template <size_t idx, typename numbers>
			struct numlist_get;
			template <size_t idx, size_t first, size_t ... numbers>
			struct numlist_get<idx, numlist<first, numbers ...>>:numlist_get<idx -1, numlist<numbers ...>>
			{
			};
			template <size_t first, size_t ... numbers>
			struct numlist_get<0, numlist<first, numbers ...>>
			{
				static constexpr size_t value = first;
			};
		}

		template <size_t... numbers> struct numlist
		{
			using increase = numlist<numbers ..., sizeof ... (numbers)>;
			using remove_back = typename _pri_::numlist_remove_back<numlist>::type;
			using remove_front = typename _pri_::numlist_remove_front<numlist>::type;
			static constexpr size_t back = _pri_::numlist_remove_back<numlist>::value;
			static constexpr size_t front =  _pri_::numlist_remove_front<numlist>::value;
			template <size_t idx>
			static constexpr size_t get() noexcept
			{
				return _pri_::numlist_get<idx, numlist>::value;
			}

			template <template <size_t ... > class T>
			using expand = T<numbers ...>;

			template <typename LAMBDA, typename ... args_t>
			static void loop(LAMBDA &&lambda, args_t & ... args)
			{
				using next = remove_front;
				lambda(args.template get<front>() ...);
				return next::loop(lambda, args ...);
			}
		};
		template <> struct numlist<>
		{
			using increase = numlist<0>;

			template <template <size_t ... > class T>
			using expand = T<>;

			template <typename LAMBDA, typename ... args_t>
			static void loop(LAMBDA &&lambda, args_t & ... args)
			{
			}
		};
		namespace _pri_
		{
			template <size_t count, size_t ... values>
			struct make_numlist :make_numlist<count - 1, count - 1, values ...>
			{
			};
			template <size_t ... values>
			struct make_numlist<0, values ...>
			{
				using type = numlist<values ... >;
			};
			template <typename t, size_t sz, typename ... ts> struct _types_n : _types_n<t, sz - 1, t, ts ...>
			{
			};
			template <typename t, typename ... ts> struct _types_n<t, 0, ts ...>
			{
				using type = types <ts ...>;
			};
		}
		template <size_t count>
		using make_numlist_counter = typename _pri_::make_numlist<count>::type;

		template <typename T, size_t sz>
		using types_n = typename _pri_::_types_n<T, sz>::type;

		namespace _pri_
		{
			template <typename typelist, size_t index> struct types_skip_32;
			template <typename typelist, size_t index> struct types_skip_8;
			template <typename typelist, size_t index> struct types_skip_1;
			template <typename typelist, size_t index> struct types_skip;
			template <
				typename t1, typename t2, typename t3, typename t4, typename t5, typename t6, typename t7, typename t8,
				typename t9, typename t10, typename t11, typename t12, typename t13, typename t14, typename t15, typename t16,
				typename t17, typename t18, typename t19, typename t20, typename t21, typename t22, typename t23, typename t24,
				typename t25, typename t26, typename t27, typename t28, typename t29, typename t30, typename t31, typename t32,
				typename ... type, size_t index> struct types_skip_32<
				types<
					t1, t2, t3, t4, t5, t6, t7, t8, 
					t9, t10, t11, t12, t13, t14, t15, t16,
					t17, t18, t19, t20, t21, t22, t23, t24,
					t25, t26, t27, t28, t29, t30, t31, t32, 
					type ...
				>, index>:types_skip<types<type ...>, index - 32> {
			};
			template <
				typename t1, typename t2, typename t3, typename t4,
				typename t5, typename t6, typename t7, typename t8,
				typename ... type, size_t index> struct types_skip_8 <
					types<t1, t2, t3, t4, t5, t6, t7, t8, type ...>
				, index>:types_skip<types<type ...>, index - 8> {
			};
			template <typename t1, typename ... type, size_t index> 
			struct types_skip_1<types<t1, type ...>, index> 
				:types_skip<types<type ...>, index - 1> {
			};
			template <typename typelist, size_t index> 
			struct types_skip : meta::if_t<
				(index < 8),
				types_skip_1<typelist, index>,
				meta::if_t<
					(index < 32),
					types_skip_8<typelist, index>,
					types_skip_32<typelist, index>
				>
			>
			{
			};

			template <typename typelist>
			struct types_skip<typelist, 0>
			{
				using type = typelist;
			};

			template <typename ... args> class itypes;

			template <typename T, typename ... args>
			struct itypes_index_of
			{
				static constexpr size_t value = -1;
			};
			template <typename T, typename _first, typename ... args>
			struct itypes_index_of<T, _first, args ...>
			{
				using next = itypes_index_of<T, args ...>;
				static constexpr size_t value = next::value == -1 ? -1 : next::value + 1;
			};
			template <typename _first, typename ... args>
			struct itypes_index_of<_first, _first, args ...>
			{
				static constexpr size_t value = 0;
			};
			template <typename _type, typename next>
			struct itypes_back_test
			{
				using type = typename next::back_t;
			};
			template <typename _type>
			struct itypes_back_test<_type, types_empty>
			{
				using type = _type;
			};

			template <typename _first, typename ... args>
			class itypes<_first, args ...> : public types<args ...>
			{
			public:
				_first value;
				using type = _first;
				using next = types<args ...>;
				using front_t = _first;
				using back_t = typename itypes_back_test<type, next>::type;

				template <typename T>
				struct index_of : itypes_index_of<T, _first, args ...>
				{
				};
				struct _findcls_return_this
				{
					using type = itypes;
				};
				template <typename TARGET> struct _findcls
					:if_same_t<TARGET, front_t, _findcls_return_this, typename next::template _findcls<TARGET> >
				{
				};
				itypes();
				itypes(front_t firstv, args ... values);

				front_t& front() noexcept;
				const front_t& front() const noexcept;
				back_t& back() noexcept;
				const back_t& back() const noexcept;
			};
			template <> class itypes<>
			{
			public:
				// using next = void;
				// using type = void;
				// using front_t = void;
				// using back_t = void;

				template <typename T> struct index_of
				{
					static constexpr size_t value = -1;
				};
				itypes() = default;

				template <typename TARGET> struct _findcls
				{
					using type = itypes;
				};
			};

			template <size_t count, typename typesin, typename typesout = types_empty>
			struct types_cut;
			template <size_t count, typename firstin, typename ... argsin, typename ... argsout>
			struct types_cut<count, types<firstin, argsin ...>, types<argsout ...>>
				:types_cut<count - 1, types<argsin ...>, types<argsout ..., firstin> >
			{
			};
			template <typename firstin, typename ... argsin, typename ... argsout>
			struct types_cut<0, types<firstin, argsin ...>, types<argsout ...> >
			{
				using type = types<argsout ...>;
			};
			template <typename firstin, typename ... argsin, typename ... argsout>
			struct types_cut<(size_t)-1, types<firstin, argsin ...>, types<argsout ...> >
			{
				using type = types<firstin, argsin ...>;
			};
			template <typename types, size_t pos, size_t count>
			struct types_subarr :types_cut<count, typename types_skip<types, pos>::type>
			{
			};
			template <typename types, size_t pos>
			struct types_subarr<types, pos, (size_t)-1> :types_skip<types, pos>
			{
			};

			template <typename types_in, typename types_out>
			struct types_reverse;
			template <typename ti1, typename ... ti, typename ... to>
			struct types_reverse<types<ti1, ti ...>, types<to ...>>:types_reverse<types<ti ...>, types<ti1, to ...>>
			{
			};
			template <typename ... to>
			struct types_reverse<types<>, types<to ...>>
			{
				using type = types<to ...>;
			};

			template <typename types, template<typename...> class dest> struct types_expand;
			template <typename ... ti, template<typename...> class dest> struct types_expand<types<ti...>, dest> {
				using type = dest<ti ...>;
			};

			template <typename types, template<size_t...> class dest> struct types_expand_index;
			template <typename ... ti, template<size_t...> class dest> struct types_expand_index<types<ti...>, dest> {
				using type = typename make_numlist_counter<(sizeof ... (ti))>::template expand<dest>;
			};
		}

		template <typename typelist, size_t index> using typesAt = typename _pri_::types_skip<typelist, index>::type;
		template <typename typelist, size_t index> using typeAt = typename _pri_::types_skip<typelist, index>::type::type;
		template <typename typelist, size_t count> using typesCut = typename _pri_::types_cut<count, typelist>::type;
		template <typename typelist, size_t pos, size_t count> using typesSubArr = typename _pri_::types_subarr<typelist, pos, count>::type;
		template <typename typelist> using typesReverse = typename _pri_::types_reverse<typelist, types<>>::type;
		template <typename typelist, template <typename ...> class dest> using typesExpand = typename _pri_::types_expand<typelist, dest>::type;
		template <typename typelist, template <size_t ...> class dest> using typesExpandIndex = typename _pri_::types_expand_index<typelist, dest>::type;
		
		template <typename ... args> class types :public _pri_::itypes<args ...>
		{
			using super = _pri_::itypes<args ...>;
		public:
			template <size_t count>
			using cut_t = typesCut<types, count>;

			template <size_t from, size_t count = (size_t)-1>
			using subarr_t = typesSubArr<types, from, count>;

		private:
			template<size_t... nums> struct _callcls
			{
				template <typename LAMBDA>
				static auto _call(types * host, LAMBDA &&lambda) -> decltype(lambda((*(remove_reference_t<args>*)0) ...));
				template <typename LAMBDA>
				static auto _call(const types * host, LAMBDA &&lambda) -> decltype(lambda((*(remove_reference_t<args>*)0) ...));
				template <typename LAMBDA>
				static void _value_loop(types * host, LAMBDA &&lambda);
				template <typename LAMBDA>
				static void _value_loop(const types * host, LAMBDA &&lambda);
				template <typename LAMBDA>
				static void _value_loop_r(types * host, LAMBDA &&lambda);
				template <typename LAMBDA>
				static void _value_loop_r(const types * host, LAMBDA &&lambda);
				template <typename LAMBDA, typename ... args_t>
				static void _value_loop_with(types * host, LAMBDA &&lambda, const args_t & ... with);
				template <typename LAMBDA, typename ... args_t>
				static void _value_loop_with(const types * host, LAMBDA &&lambda, const args_t & ... with);
			};
		public:
			static constexpr size_t size = sizeof ... (args);

			static const size_t(&getSizeArray() noexcept)[size]
			{
				static const size_t sizes[] = { sizeof(args)... };
				return sizes;
			}

			template <template<typename...> class dest>
			using expand_type = typesExpand<types, dest>;
			template <template<size_t...> class dest>
			using expand_idx = typesExpandIndex<types, dest>;
			using expand_func = expand_idx<_callcls>;

			// min(argssize, size) 상수 표현식을 사용하니 인텔리 센스가 죽는다.
			template <size_t argssize>
			using expand_min_func = typename make_numlist_counter<argssize < size ? argssize : size>::template expand<_callcls>;
			
		public:
			using super::super;

			template <typename LAMBDA> auto call(LAMBDA && lambda) -> decltype(lambda((*(remove_reference_t<args>*)0) ...));
			template <typename LAMBDA> auto call(LAMBDA && lambda) const -> decltype(lambda((*(remove_reference_t<args>*)0) ...));
			template <typename LAMBDA> static auto castCall(LAMBDA && lambda, args ... v) -> decltype(lambda((*(remove_reference_t<args>*)0) ...));

			template <typename LAMBDA> static void type_loop(LAMBDA && lambda);
			template <typename LAMBDA> static void type_loop_r(LAMBDA && lambda);
			template <typename LAMBDA> static void type_switch(size_t idx, LAMBDA && lambda);
			template <typename LAMBDA> static void type_switch(size_t idx, LAMBDA && lambda, void * value);
			template <typename LAMBDA> static void type_switch(size_t idx, LAMBDA && lambda, const void * value);
			static size_t type_sizeof_at(size_t idx) noexcept;
			template <typename LAMBDA> void value_loop(LAMBDA && lambda) const;
			template <typename LAMBDA> void value_loop(LAMBDA && lambda);
			template <typename LAMBDA> void value_loop_r(LAMBDA && lambda) const;
			template <typename LAMBDA> void value_loop_r(LAMBDA && lambda);
			template <typename LAMBDA> void value_loop_with(LAMBDA && lambda, const args & ... p) const;
			template <typename LAMBDA> void value_loop_with(LAMBDA && lambda, const args & ... p);
			template <typename LAMBDA, typename ... types_list> void value_loop_with(LAMBDA && lambda, const types_list & ... types_args) const;
			template <typename LAMBDA, typename ... types_list> void value_loop_with(LAMBDA && lambda, const types_list & ... types_args);

			template <size_t from>
			subarr_t<from>& subarr() noexcept;
			template <size_t from>
			const subarr_t<from>& subarr() const noexcept;
			template <size_t count>
			const cut_t<count> cut_copy() const noexcept;
			template <size_t from, size_t count = (size_t)-1>
			const subarr_t<from, count> subarr_copy() const noexcept;
			template <typename T> T * find() noexcept;
			template <size_t index> typeAt<types, index>& get() noexcept;
			template <size_t index> const typeAt<types, index>& get() const noexcept;
			template <size_t index> void set(typeAt<types, index> value) const;
		};

		template <typename ... args>
		template <size_t ... nums>
		template <typename LAMBDA>
		auto types<args ... >::_callcls<nums ...>::_call(types<args ...> * host, LAMBDA && lambda) -> decltype(lambda((*(remove_reference_t<args>*)0) ...))
		{
			return lambda(host->get<nums>() ...);
		}
		template <typename ... args>
		template <size_t ... nums>
		template <typename LAMBDA>
		auto types<args ... >::_callcls<nums ...>::_call(const types<args ...> * host, LAMBDA && lambda) -> decltype(lambda((*(remove_reference_t<args>*)0) ...))
		{
			return lambda(host->get<nums>() ...);
		}
		template <typename ... args>
		template <size_t ... nums>
		template <typename LAMBDA>
		void types<args ... >::_callcls<nums ...>::_value_loop(types * host, LAMBDA && lambda)
		{
			unpack(lambda(host->get<nums>()));
		}
		template <typename ... args>
		template <size_t ... nums>
		template <typename LAMBDA>
		void types<args ... >::_callcls<nums ...>::_value_loop(const types * host, LAMBDA && lambda)
		{
			unpack(lambda(host->get<nums>()));
		}
		template <typename ... args>
		template <size_t ... nums>
		template <typename LAMBDA>
		void types<args ... >::_callcls<nums ...>::_value_loop_r(types * host, LAMBDA && lambda)
		{
			unpackR(lambda(host->get<nums>()));
		}
		template <typename ... args>
		template <size_t ... nums>
		template <typename LAMBDA>
		void types<args ... >::_callcls<nums ...>::_value_loop_r(const types * host, LAMBDA && lambda)
		{
			unpackR(lambda(host->get<nums>()));
		}
		template <typename ... args>
		template <size_t ... nums>
		template <typename LAMBDA, typename ... args_t>
		void types<args ... >::_callcls<nums ...>::_value_loop_with(types * host, LAMBDA && lambda, const args_t & ... with)
		{
			unpack(lambda(host->get<nums>(), with));
		}
		template <typename ... args>
		template <size_t ... nums>
		template <typename LAMBDA, typename ... args_t>
		void types<args ... >::_callcls<nums ...>::_value_loop_with(const types * host, LAMBDA && lambda, const args_t & ... with)
		{
			unpack(lambda(host->get<nums>(), with));
		}

		template <typename first, typename ... args>
		_pri_::itypes<first, args ... >::itypes()
		{
		}
		template <typename first, typename ... args>
		_pri_::itypes<first, args ... >::itypes(first firstv, args ... values)
			:value(move(firstv)), next(move(values) ...)
		{
		}
		template<typename _first, typename ...args>
		typename _pri_::itypes<_first, args ... >::front_t& _pri_::itypes<_first, args ... >::front() noexcept
		{
			return value;
		}
		template<typename _first, typename ...args>
		const typename _pri_::itypes<_first, args ... >::front_t& _pri_::itypes<_first, args ... >::front() const noexcept
		{
			return value;
		}
		template<typename _first, typename ...args>
		typename _pri_::itypes<_first, args ... >::back_t& _pri_::itypes<_first, args ... >::back() noexcept
		{
			return static_cast<types<back_t>*>(this)->value;
		}
		template<typename _first, typename ...args>
		const typename _pri_::itypes<_first, args ... >::back_t& _pri_::itypes<_first, args ... >::back() const noexcept
		{
			return static_cast<const types<back_t>*>(this)->value;
		}

		template <typename ... args>
		template <typename LAMBDA>
		auto types<args ... >::call(LAMBDA && lambda) -> decltype(lambda((*(remove_reference_t<args>*)0) ...))
		{
			return expand_func::_call(this, lambda);
		}
		template <typename ... args>
		template <typename LAMBDA>
		auto types<args ... >::call(LAMBDA && lambda) const -> decltype(lambda((*(remove_reference_t<args>*)0) ...))
		{
			return expand_func::_call(this, lambda);
		}
		template <typename ... args>
		template <typename LAMBDA>
		auto types<args ... >::castCall(LAMBDA && lambda, args ... v) -> decltype(lambda((*(remove_reference_t<args>*)0) ...))
		{
			return lambda(v ...);
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::type_loop(LAMBDA && lambda)
		{
			unpack(lambda((args*)0));
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::type_loop_r(LAMBDA && lambda)
		{
			unpackR(lambda((args*)0));
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::type_switch(size_t idx, LAMBDA && lambda)
		{
			size_t i = 0;
			unpack(([&]() { if (idx == i++) lambda((args*)0); }()));
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::type_switch(size_t idx, LAMBDA && lambda, void * value)
		{
			size_t i = 0;
			unpack(([&]() { if (idx == i++) lambda((args*)value); }()));
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::type_switch(size_t idx, LAMBDA && lambda, const void * value)
		{
			size_t i = 0;
			unpack(([&]() { if (idx == i++) lambda((const args*)value); }()));
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::value_loop(LAMBDA && lambda) const
		{
			expand_func::_value_loop(this, lambda);
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::value_loop(LAMBDA && lambda)
		{
			expand_func::_value_loop(this, lambda);
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::value_loop_r(LAMBDA && lambda) const
		{
			expand_func::_value_loop_r(this, lambda);
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::value_loop_r(LAMBDA && lambda)
		{
			expand_func::_value_loop_r(this, lambda);
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::value_loop_with(LAMBDA && lambda, const args &... p) const
		{
			expand_func::_value_loop_with(this, lambda, p ...);
		}
		template <typename ... args>
		template <typename LAMBDA>
		void types<args ... >::value_loop_with(LAMBDA && lambda, const args &... p)
		{
			expand_func::_value_loop_with(this, lambda, p ...);
		}
		template <typename ... args>
		template <typename LAMBDA, typename ... types_list>
		void types<args ... >::value_loop_with(LAMBDA && lambda, const types_list & ... types) const
		{
			constexpr size_t minvalue = mint(size, (types_list::size) ...);
			make_numlist_counter<minvalue>::loop(lambda, *this, types ...);
		}
		template <typename ... args>
		template <typename LAMBDA, typename ... types_list>
		void types<args ... >::value_loop_with(LAMBDA && lambda, const types_list & ... types_args)
		{
			constexpr size_t minvalue = mint(size, (types_list::size) ...);
			make_numlist_counter<minvalue>::loop(lambda, *this, types_args ...);
		}
		
		template<typename ...args>
		template <size_t from>
		auto types<args ... >::subarr() noexcept->subarr_t<from>&
		{
			return *static_cast<subarr_t<from>*>(this);
		}
		template<typename ...args>
		template <size_t from>
		auto types<args ... >::subarr() const noexcept->const subarr_t<from>&
		{
			return *static_cast<const subarr_t<from>*>(this);
		}
		template<typename ...args>
		template <size_t count>
		auto types<args ... >::cut_copy() const noexcept->const cut_t<count>
		{
			cut_t<count> values;
			values.value_loop_with([](auto & dest, const auto &src) { dest = src;  }, *this);
			return values;
		}
		template<typename ...args>
		template <size_t from, size_t count>
		auto types<args ... >::subarr_copy() const noexcept->const subarr_t<from, count>
		{
			return subarr<from>().template cut_copy<count>();
		}

		template<typename ...args>
		template<typename T>
		inline T * types<args ... >::find() noexcept
		{
			using finded = typename super::template _findcls<T>::type;
			return &static_cast<finded*>(this)->value;
		}

		template <typename ... args>
		template <size_t index>
		typeAt<types<args ... >, index>& types<args ... >::get() noexcept
		{
			return typesAt<types, index>::value;
		}
		template <typename ... args>
		template <size_t index> 
		const typeAt<types<args ... >, index>& types<args ... >::get() const noexcept
		{
			return typesAt<types, index>::value;
		}
		template <typename ... args>
		template <size_t index> 
		void types<args ... >::set(typeAt<types, index> value) const
		{
			typesAt<types, index>::value = value;
		}

		// type list concat
		namespace _pri_
		{
			template <typename ... typeses> struct type_concat;

			template <typename ... args1, typename ... args2, typename ... left>
			struct type_concat<types<args1 ...>, types<args2 ...>, left ...>
				:type_concat<types<args1 ..., args2 ...>, left ...>{};

			template <typename single> struct type_concat<single>
			{
				using type = single;
			};
		}

		template <typename ... types_list> using type_concat = typename _pri_::type_concat<types_list ...>::type;

		// type list push
		namespace _pri_
		{
			template <typename types, typename ... push> struct type_push;

			template <typename ... typescomps, typename ... push>
			struct type_push<types<typescomps ...>, push ...>
			{
				using type = types<typescomps ..., push...>;
			};
		}

		template <typename types, typename ... push> using type_push = typename _pri_::type_push<types, push ...>::type;

		// type list insert
		namespace _pri_
		{
			template <typename ... insert> struct type_insert_back;
			template <typename ... insert, typename checkf, typename ... check>
			struct type_insert_back<types<insert ...>, checkf, check ...>: type_insert_back<types<insert ..., checkf>, check ...>
			{
			};
			template <typename ... insert, typename ... ready>
			struct type_insert_back<types<insert ...>, types<ready ...>>
			{
				using type = types<insert ..., ready ...>;
			};

			template <typename ... insert> struct type_insert:
				public type_insert_back<types_empty, insert ...>
			{
			};
		}

		template <typename ... insert> using type_insert = typename _pri_::type_insert<insert ...>::type;

		// type list cast
		namespace _pri_
		{
			template <typename types, template<typename> class cast> struct casts;
			template <typename ... args, template<typename> class cast> struct casts<types<args ...>, cast>
			{
				using type = types<cast<args>...>;
			};
		}
		template <typename types, template<typename> class cast> using casts = typename _pri_::casts<types, cast>::type;
	}
}
