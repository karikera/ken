#pragma once

#include <KR3/main.h>
#include <KR3/meta/array.h>
#include <KR3/meta/types.h>
#include <KR3/meta/text.h>
#include <KR3/data/binarray.h>

#include "db.h"
#include "exception.h"

namespace kr
{
	namespace sql
	{
		using mysql_size_t = unsigned long;
		static_assert(sizeof(mysql_size_t) <= sizeof(size_t), "Size overflow");

		template <typename params_t, typename results_t> class Statement;

		template <typename T, enum_field_types type, my_bool unsign> struct MysqlTypeImpl
		{
			static void initParam(MYSQL_BIND& bind) noexcept;
			static void bindParam(MYSQL_BIND& bind, T * param) noexcept;
			static void initResult(MYSQL_BIND& bind) noexcept;
			static void bindResult(MYSQL_BIND& bind, T * param) noexcept;
			static void allocate(MYSQL_BIND& bind) noexcept;
		};
		template <typename T> struct MysqlType;
		template <> struct MysqlType<int>:MysqlTypeImpl<int, MYSQL_TYPE_LONG, false>{};
		template <> struct MysqlType<long long> :MysqlTypeImpl<long long, MYSQL_TYPE_LONGLONG, false>{};
		template <> struct MysqlType<long> :MysqlTypeImpl<long, (sizeof(unsigned long) > 4 ? MYSQL_TYPE_LONGLONG : MYSQL_TYPE_LONG), false>{};
		template <> struct MysqlType<short> :MysqlTypeImpl<short, MYSQL_TYPE_SHORT, false>{};
		template <> struct MysqlType<char>:MysqlTypeImpl<char, MYSQL_TYPE_TINY, false>{};
		template <> struct MysqlType<uint>:MysqlTypeImpl<uint, MYSQL_TYPE_LONG, true>{};
		template <> struct MysqlType<unsigned long long>:MysqlTypeImpl<unsigned long long, MYSQL_TYPE_LONGLONG, true>{};
		template <> struct MysqlType<unsigned long>:MysqlTypeImpl<unsigned long, (sizeof(unsigned long) > 4 ? MYSQL_TYPE_LONGLONG : MYSQL_TYPE_LONG), true>{};
		template <> struct MysqlType<unsigned short>:MysqlTypeImpl<unsigned short, MYSQL_TYPE_SHORT, true>{};
		template <> struct MysqlType<unsigned char>:MysqlTypeImpl<unsigned char, MYSQL_TYPE_TINY, true>{};
		template <> struct MysqlType<bool> :MysqlTypeImpl<bool, MYSQL_TYPE_TINY, true> {};
		template <> struct MysqlType<float> :MysqlTypeImpl<float, MYSQL_TYPE_FLOAT, false> {};
		template <> struct MysqlType<double> :MysqlTypeImpl<double, MYSQL_TYPE_DOUBLE, false> {};
		template <> struct MysqlType<Text>
		{
			static void initParam(MYSQL_BIND& bind) noexcept;
			static void bindParam(MYSQL_BIND& bind, Text * str) noexcept;
		};;
		template <> struct MysqlType<AText>
		{
			static void initParam(MYSQL_BIND& bind) noexcept;
			static void bindParam(MYSQL_BIND& bind, AText * str) noexcept;
			static void initResult(MYSQL_BIND& bind) noexcept;
			static void bindResult(MYSQL_BIND& bind, AText * str) noexcept;
			static void allocate(MYSQL_BIND& bind) noexcept;
		};;
		template <> struct MysqlType<Buffer>
		{
			static void initParam(MYSQL_BIND& bind) noexcept;
			static void bindParam(MYSQL_BIND& bind, Buffer * str) noexcept;
		};;
		template <> struct MysqlType<ABuffer>
		{
			static void initParam(MYSQL_BIND& bind) noexcept;
			static void bindParam(MYSQL_BIND& bind, ABuffer * str) noexcept;
			static void initResult(MYSQL_BIND& bind) noexcept;
			static void bindResult(MYSQL_BIND& bind, ABuffer * str) noexcept;
			static void allocate(MYSQL_BIND& bind) noexcept;
		};;
		template <size_t sz> struct MysqlType<BText<sz>>
		{
			static void initParam(MYSQL_BIND& bind) noexcept;
			static void bindParam(MYSQL_BIND& bind, BText<sz> * str) noexcept;
			static void initResult(MYSQL_BIND& bind) noexcept;
			static void bindResult(MYSQL_BIND& bind, BText<sz> * str) noexcept;
			static void allocate(MYSQL_BIND& bind);
		};
		template <typename C> struct MysqlType<View<C>>
		{
			static_assert(sizeof(internal_component_t<C>) == 1, "Component size must be 1");
			static void initParam(MYSQL_BIND& bind) noexcept;
			static void bindParam(MYSQL_BIND& bind, View<C> * str) noexcept;
		};;
		template <typename C> struct MysqlType<Array<C>>
		{
			static_assert(sizeof(internal_component_t<C>) == 1, "Component size must be 1");
			static void initParam(MYSQL_BIND& bind) noexcept;
			static void bindParam(MYSQL_BIND& bind, Array<C> * str) noexcept;
			static void initResult(MYSQL_BIND& bind) noexcept;
			static void bindResult(MYSQL_BIND& bind, Array<C> * str) noexcept;
			static void allocate(MYSQL_BIND& bind) noexcept;
		};;
		template <typename C, size_t sz> struct MysqlType<BArray<C, sz>>
		{
			static_assert(sizeof(internal_component_t<C>) == 1, "Component size must be 1");
			static void initParam(MYSQL_BIND& bind) noexcept;
			static void bindParam(MYSQL_BIND& bind, BArray<C, sz> * str) noexcept;
			static void initResult(MYSQL_BIND& bind) noexcept;
			static void bindResult(MYSQL_BIND& bind, BArray<C, sz> * str) noexcept;
			static void allocate(MYSQL_BIND& bind);
		};

		class PreparedStatementImpl
		{
		public:
			qword getInsertId() noexcept;
			qword affactedRows() noexcept;
			void execute() throws(ThrowRetry, SqlException);
			void execute(MySQL & db) throws(SqlException);

		protected:
			PreparedStatementImpl(MySQL& sql, Text query) throws(SqlException);
			~PreparedStatementImpl() noexcept;
			
			void _bindParam(MYSQL_BIND * bind) throws(SqlException);
			void _bindResult(MYSQL_BIND * bind) throws(SqlException);
			void _storeResult() throws(ThrowRetry, SqlException);
			void _storeResult(MySQL & db) throws(SqlException);
			void _freeResult() noexcept;
			bool _fetch() throws(ThrowRetry, ThrowAllocate, SqlException);
			bool _fetch(MySQL & db) throws(ThrowAllocate, SqlException);
			void _fetchColumn(MYSQL_BIND * bind, uint index) throws(ThrowRetry, SqlException);

		private:
			MYSQL_STMT * m_stmt;
			Text m_query;
		};

		template <size_t index, typename ... ARGS> class StatementParam;
		template <size_t index> class StatementParam<index>:public PreparedStatementImpl
		{
		public:
			static constexpr size_t count = index;

			using PreparedStatementImpl::PreparedStatementImpl;
		};
		template <> class StatementParam<0>:public PreparedStatementImpl
		{
		public:
			static constexpr size_t count = 0;

			using PreparedStatementImpl::PreparedStatementImpl;
		};


		template <typename params, typename results = void> class Statement;

		template <typename params> class ParameterBind
		{
			template <typename params_t, typename results_t>
			friend class Statement;
		private:
			meta::array<MYSQL_BIND, params::size> m_params;

		public:
			using paramTypes = params;
			using paramPtrTypes = meta::casts<params, add_pointer_t>;

			ParameterBind() noexcept;
			ParameterBind(const paramPtrTypes &values) noexcept;
			ParameterBind(const paramTypes * values) noexcept;
			ParameterBind& operator =(const paramPtrTypes &values) noexcept;
			ParameterBind& operator =(const paramTypes * values) noexcept;
		};

		template <typename results> class ResultBind
		{
			template <typename params_t, typename results_t> 
			friend class Statement;
		private:
			meta::array<MYSQL_BIND, results::size> m_results;

		public:
			using resultTypes = results;
			using resultPtrTypes = meta::casts<results, add_pointer_t>;

			ResultBind() noexcept;
			ResultBind(const resultPtrTypes &values) noexcept;
			ResultBind(results * values) noexcept;
			ResultBind& operator =(const resultPtrTypes &values) noexcept;
			ResultBind& operator =(results * values) noexcept;
		};
		
		template <typename params_t, typename results_t> class Statement:public Statement<params_t, void>
		{
		private:
			ResultBind<results_t> m_resultSet;

			using PreparedStatementImpl::execute;

			bool _fetch() throws(ThrowRetry, SqlException);
			bool _fetch(MySQL& db) throws(SqlException);
			
		public:
			class Fetcher
			{
			private:
				Statement * const m_host;
			
			public:
				Fetcher(Statement * st) throws(ThrowRetry, SqlException);
				Fetcher(MySQL & db, Statement * st) throws(ThrowRetry, SqlException);
				~Fetcher() noexcept;
				bool fetch() throws(ThrowRetry, SqlException);
				bool fetch(MySQL& db) throws(SqlException);
			};
			
			using resultTypes = results_t;
			using resultPtrTypes = meta::casts<results_t, add_pointer_t>;
			using Super = Statement<params_t, void>;
			using Super::bindParams;
			using typename Super::paramPtrTypes;

			void bind(const paramPtrTypes &param, const resultPtrTypes &res) noexcept;
			void bindResults(resultTypes * values) noexcept;
			void bindResults(const resultPtrTypes &values) noexcept;
			Statement(MySQL& sql, Text text) throws(SqlException);
			void fetchColumn(uint index) throws(ThrowRetry, SqlException);
			void allocateFetch() throws(ThrowRetry, SqlException);

			bool fetchOnce() throws(ThrowRetry, SqlException);
			bool fetchOnce(const paramPtrTypes &param, const resultPtrTypes &res) throws(ThrowRetry, SqlException);
			template <typename LAMBDA> void fetch(const paramPtrTypes &param, const resultPtrTypes &res, LAMBDA lambda) throws(ThrowRetry, SqlException);
			template <typename LAMBDA> void fetch(const paramPtrTypes &param, LAMBDA lambda) throws(ThrowRetry, SqlException);
			template <typename LAMBDA> void fetch(LAMBDA lambda) throws(ThrowRetry, SqlException);
			template <typename LAMBDA> int fetchCount(LAMBDA lambda) throws(ThrowRetry, SqlException);

			bool fetchOnce(MySQL & db) throws(ThrowRetry, SqlException);
			bool fetchOnce(MySQL & db, const paramPtrTypes &param, const resultPtrTypes &res) throws(ThrowRetry, SqlException);
			template <typename LAMBDA> void fetch(MySQL & db, const paramPtrTypes &param, const resultPtrTypes &res, LAMBDA lambda) throws(ThrowRetry, SqlException);
			template <typename LAMBDA> void fetch(MySQL & db, const paramPtrTypes &param, LAMBDA lambda) throws(ThrowRetry, SqlException);
			template <typename LAMBDA> void fetch(MySQL & db, LAMBDA lambda) throws(ThrowRetry, SqlException);
			template <typename LAMBDA> int fetchCount(MySQL & db, LAMBDA lambda) throws(ThrowRetry, SqlException);
		};;

		template <typename params_t> class Statement<params_t, void>:public PreparedStatementImpl
		{
		private:
			ParameterBind<params_t> m_paramSet;

		public:
			using paramTypes = params_t;
			using paramPtrTypes = meta::casts<params_t, add_pointer_t>;

			void bind(const paramPtrTypes &values) noexcept;
			void bindParams(const paramTypes * values) noexcept;
			void bindParams(const paramPtrTypes &values) noexcept;
			Statement(MySQL& sql, Text text) throws(SqlException);
			void execute(const paramPtrTypes &values) noexcept;
			void execute(MySQL & db, const paramPtrTypes &values) noexcept;
			void execute(MySQL & db, const paramTypes *values) noexcept;
			void executeWith(MySQL & db, const paramTypes &values) noexcept;
			using PreparedStatementImpl::execute;
		};;

	}
}

#include "statement.inl"