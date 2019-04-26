#pragma once

#include <KR3/meta/chreturn.h>

template <typename T, enum_field_types type, my_bool unsign>
inline void kr::sql::MysqlTypeImpl<T, type, unsign>::initParam(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = type;
	bind.is_unsigned = unsign;
}
template <typename T, enum_field_types type, my_bool unsign>
inline void kr::sql::MysqlTypeImpl<T, type, unsign>::bindParam(MYSQL_BIND& bind, T * param) noexcept
{
	bind.buffer_type = type;
	bind.is_unsigned = unsign;
	bind.buffer = param;
}
template <typename T, enum_field_types type, my_bool unsign>
inline void kr::sql::MysqlTypeImpl<T, type, unsign>::initResult(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = type;
	bind.is_unsigned = unsign;
}
template <typename T, enum_field_types type, my_bool unsign>
inline void kr::sql::MysqlTypeImpl<T, type, unsign>::bindResult(MYSQL_BIND& bind, T * param) noexcept
{
	bind.buffer_type = type;
	bind.is_unsigned = unsign;
	bind.buffer = param;
}
template <typename T, enum_field_types type, my_bool unsign>
inline void kr::sql::MysqlTypeImpl<T, type, unsign>::allocate(MYSQL_BIND& bind) noexcept
{
}

template <size_t sz>
inline void kr::sql::MysqlType<kr::BText<sz>>::initParam(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_STRING;
}
template <size_t sz>
inline void kr::sql::MysqlType<kr::BText<sz>>::bindParam(MYSQL_BIND& bind, BText<sz> * str) noexcept
{
	bind.buffer = (void*)str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->size());
}
template <size_t sz>
inline void kr::sql::MysqlType<kr::BText<sz>>::initResult(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_STRING;
}
template <size_t sz>
inline void kr::sql::MysqlType<kr::BText<sz>>::bindResult(MYSQL_BIND& bind, BText<sz> * str) noexcept
{
	bind.buffer = str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->capacity());
	_assert(str->size() <= (mysql_size_t)-1);
	bind.length = (mysql_size_t*)&((size_t*)str->begin())[-1];
}
template <size_t sz>
inline void kr::sql::MysqlType<kr::BText<sz>>::allocate(MYSQL_BIND& bind)
{
	throw NotEnoughSpaceException();
}
template <typename C>
inline void kr::sql::MysqlType<kr::View<C>>::initParam(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_BLOB;
}
template <typename C>
inline void kr::sql::MysqlType<kr::View<C>>::bindParam(MYSQL_BIND& bind, View<C> * str) noexcept
{
	bind.buffer = (void*)str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->size());
}
template <typename C>
inline void kr::sql::MysqlType<kr::Array<C>>::initParam(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_BLOB;
}
template <typename C>
inline void kr::sql::MysqlType<kr::Array<C>>::bindParam(MYSQL_BIND& bind, Array<C> * str) noexcept
{
	bind.buffer = (void*)str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->size());
}
template <typename C>
inline void kr::sql::MysqlType<kr::Array<C>>::initResult(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_STRING;
}
template <typename C>
inline void kr::sql::MysqlType<kr::Array<C>>::bindResult(MYSQL_BIND& bind, Array<C> * str) noexcept
{
	if (*str == nullptr) str->reserve(128);
	else str->clear();
	bind.buffer = str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->capacity());
	bind.length = (mysql_size_t*)&((size_t*)str->begin())[-1];
	bind.extension = str;
}
template <typename C>
inline void kr::sql::MysqlType<kr::Array<C>>::allocate(MYSQL_BIND& bind) noexcept
{
	mysql_size_t len = *bind.length;
	if (len <= bind.buffer_length) return;

	Array<C> * str = (Array<C>*)bind.extension;
	mysql_size_t* sizeptr = (mysql_size_t*)&((size_t*)str->begin())[-1];
	*sizeptr = bind.buffer_length;

	str->clear();
	str->reserve(len);
	bind.buffer = str->begin();
	bind.buffer_length = len;
	bind.length = (mysql_size_t*)&((size_t*)str->begin())[-1];
}
template <typename C, size_t sz>
inline void kr::sql::MysqlType<kr::BArray<C, sz>>::initParam(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_BLOB;
}
template <typename C, size_t sz>
inline void kr::sql::MysqlType<kr::BArray<C, sz>>::bindParam(MYSQL_BIND& bind, BArray<C, sz> * str) noexcept
{
	bind.buffer = (void*)str->begin();
	bind.buffer_length = intact<unsigned long>(str->size());
}
template <typename C, size_t sz>
inline void kr::sql::MysqlType<kr::BArray<C, sz>>::initResult(MYSQL_BIND& bind) noexcept
{
	bind.buffer_type = MYSQL_TYPE_STRING;
}
template <typename C, size_t sz>
inline void kr::sql::MysqlType<kr::BArray<C, sz>>::bindResult(MYSQL_BIND& bind, BArray<C, sz> * str) noexcept
{
	bind.buffer = str->begin();
	bind.buffer_length = intact<mysql_size_t>(str->capacity());
	_assert(str->size() <= 0xffffffff);
	bind.length = (unsigned long*)&((size_t*)str->begin())[-1];
}
template <typename C, size_t sz>
inline void kr::sql::MysqlType<kr::BArray<C, sz>>::allocate(MYSQL_BIND& bind)
{
	throw NotEnoughSpaceException();
}

template <typename params_t>
inline kr::sql::ParameterBind<params_t>::ParameterBind() noexcept
{
	memset(m_params, 0, sizeof(m_params));

	size_t i = 0;
	params_t::type_loop([&](auto * t){
		kr::sql::MysqlType<std::remove_reference_t<decltype(*t)>>::initParam(m_params[i++]);
	});
}
template <typename params_t>
inline kr::sql::ParameterBind<params_t>::ParameterBind(const paramPtrTypes &values) noexcept
	:ParameterBind()
{
	*this = values;
}
template <typename params_t>
inline kr::sql::ParameterBind<params_t>::ParameterBind(const paramTypes * values) noexcept
	:ParameterBind()
{
	*this = values;
}
template <typename params_t>
inline kr::sql::ParameterBind<params_t>& kr::sql::ParameterBind<params_t>::operator =(const paramPtrTypes &values) noexcept
{
	size_t i = 0;
	values.value_loop([&](auto * pvalue) {
		kr::sql::MysqlType<std::remove_pointer_t<decltype(pvalue)>>::bindParam(m_params[i++], pvalue);
	});
	return *this;
}
template <typename params_t>
inline kr::sql::ParameterBind<params_t>& kr::sql::ParameterBind<params_t>::operator =(const paramTypes * values) noexcept
{
	size_t i = 0;
	values->value_loop([&](const auto & value) {
		using type = std::remove_const_t<std::remove_reference_t<decltype(value)>>;
		kr::sql::MysqlType<type>::bindParam(m_params[i++], (type*)&value);
	});
	return *this;
}

template <typename results_t>
inline kr::sql::ResultBind<results_t>::ResultBind() noexcept
{
	memset(m_results, 0, sizeof(m_results));
	size_t i = 0;
	results_t::type_loop([&](auto * t)
	{
		kr::sql::MysqlType<std::remove_pointer_t<decltype(t)>>::initResult(m_results[i++]);
	});
}
template <typename results_t>
inline kr::sql::ResultBind<results_t>::ResultBind(const resultPtrTypes &values) noexcept
	:ResultBind()
{
	*this = values;
}
template <typename results_t>
inline kr::sql::ResultBind<results_t>::ResultBind(results_t * values) noexcept
	:ResultBind()
{
	*this = values;
}
template <typename results_t>
inline kr::sql::ResultBind<results_t>& kr::sql::ResultBind<results_t>::operator =(const resultPtrTypes &values) noexcept
{
	size_t i = 0;
	values.value_loop([&](auto * pvalue)
	{
		kr::sql::MysqlType<std::remove_pointer_t<decltype(pvalue)>>::bindResult(m_results[i++], pvalue);
	});
	return *this;
}
template <typename results_t>
inline kr::sql::ResultBind<results_t>& kr::sql::ResultBind<results_t>::operator =(results_t * values) noexcept
{
	size_t i = 0;
	values->value_loop([&](auto & value)
	{
		kr::sql::MysqlType<std::remove_reference_t<decltype(value)>>::bindResult(m_results[i++], &value);
	});
	return *this;
}

template <typename params_t, typename results_t>
kr::sql::Statement<params_t, results_t>::Fetcher::Fetcher(Statement * st) throws(ThrowRetry, SqlException)
	:m_host(st)
{
	m_host->execute();
	m_host->_storeResult();
}
template <typename params_t, typename results_t>
kr::sql::Statement<params_t, results_t>::Fetcher::Fetcher(MySQL &db, Statement * st) throws(ThrowRetry, SqlException)
	:m_host(st)
{
	m_host->execute(db);
	m_host->_storeResult(db);
}
template <typename params_t, typename results_t>
kr::sql::Statement<params_t, results_t>::Fetcher::~Fetcher() noexcept
{
	m_host->_freeResult();
}
template <typename params_t, typename results_t>
bool kr::sql::Statement<params_t, results_t>::Fetcher::fetch() throws(ThrowRetry, SqlException)
{
	return m_host->_fetch();
}
template <typename params_t, typename results_t>
bool kr::sql::Statement<params_t, results_t>::Fetcher::fetch(MySQL& db) throws(SqlException)
{
	return m_host->_fetch(db);
}
template <typename params_t, typename results_t> 
inline kr::sql::Statement<params_t,results_t>::Statement(MySQL& sql, Text text) throws(SqlException)
	:Statement<params_t, void>(sql, text)
{
}
template <typename params_t, typename results_t>
inline bool kr::sql::Statement<params_t, results_t>::_fetch() throws(ThrowRetry, SqlException)
{
	try
	{
		return PreparedStatementImpl::_fetch();
	}
	catch (ThrowAllocate&)
	{
		allocateFetch();
		return true;
	}
}
template <typename params_t, typename results_t>
inline bool kr::sql::Statement<params_t, results_t>::_fetch(MySQL& db) throws(SqlException)
{
	try
	{
		return PreparedStatementImpl::_fetch(db);
	}
	catch (ThrowAllocate&)
	{
		allocateFetch();
		return true;
	}
}
template <typename params_t, typename results_t> 
inline void kr::sql::Statement<params_t,results_t>::fetchColumn(uint index) throws(ThrowRetry, SqlException)
{
	PreparedStatementImpl::_fetchColumn(&m_resultSet.m_results[index], index);
}
template <typename params_t, typename results_t>
inline void kr::sql::Statement<params_t, results_t>::allocateFetch() throws(ThrowRetry, SqlException)
{
	uint i = 0;
	results_t::type_loop([&](auto *pvalue)
	{
		kr::sql::MysqlType<std::remove_pointer_t<decltype(pvalue)>>::allocate(m_resultSet.m_results[i]);
		fetchColumn(i++);
	});
}

template <typename params_t, typename results_t>
inline bool kr::sql::Statement<params_t, results_t>::fetchOnce() throws(ThrowRetry, SqlException)
{
	return Fetcher(this).fetch();
}
template <typename params_t, typename results_t>
inline bool kr::sql::Statement<params_t, results_t>::fetchOnce(const paramPtrTypes &param, const resultPtrTypes &res) throws(ThrowRetry, SqlException)
{
	bind(param, res);
	return fetchOnce();
}
template <typename params_t, typename results_t>
template <typename LAMBDA>
inline void kr::sql::Statement<params_t, results_t>::fetch(const paramPtrTypes &param, const resultPtrTypes &res, LAMBDA lambda) throws(ThrowRetry, SqlException)
{
	bind(param, res);
	fetch(lambda);
}
template <typename params_t, typename results_t>
template <typename LAMBDA>
inline void kr::sql::Statement<params_t, results_t>::fetch(const paramPtrTypes &param, LAMBDA lambda) throws(ThrowRetry, SqlException)
{
	bindParams(param);
	results_t res;
	bindResults(&res);
	fetch([&] { return res.call(lambda); });
}
template <typename params_t, typename results_t> 
template <typename LAMBDA> 
inline void kr::sql::Statement<params_t,results_t>::fetch(LAMBDA lambda) throws(ThrowRetry, SqlException)
{
	Fetcher fetcher = this;
	while(fetcher.fetch())
	{
		if (meta::returnBool(lambda)()) break;
	}
}
template <typename params_t, typename results_t> 
template <typename LAMBDA> 
inline int kr::sql::Statement<params_t,results_t>::fetchCount(LAMBDA lambda) throws(ThrowRetry, SqlException)
{
	Fetcher fetcher = this;
	int count = 0;
	while (fetcher.fetch())
	{
		count++;
		if (meta::returnBool(lambda)()) break;
	}
	return count;
}


template <typename params_t, typename results_t>
inline void kr::sql::Statement<params_t, results_t>::bind(const paramPtrTypes &param, const resultPtrTypes &res) noexcept
{
	bindParams(param);
	bindResults(res);
}
template <typename params_t, typename results_t>
inline void kr::sql::Statement<params_t, results_t>::bindResults(resultTypes * values) noexcept
{
	m_resultSet = values;
	PreparedStatementImpl::_bindResult(m_resultSet.m_results);
}
template <typename params_t, typename results_t>
inline void kr::sql::Statement<params_t, results_t>::bindResults(const resultPtrTypes &values) noexcept
{
	m_resultSet = values;
	PreparedStatementImpl::_bindResult(m_resultSet.m_results);
}
template <typename params_t, typename results_t>
inline bool kr::sql::Statement<params_t, results_t>::fetchOnce(MySQL & db) throws(ThrowRetry, SqlException)
{
	return Fetcher(this).fetch(db);
}
template <typename params_t, typename results_t>
inline bool kr::sql::Statement<params_t, results_t>::fetchOnce(MySQL & db, const paramPtrTypes &param, const resultPtrTypes &res) throws(ThrowRetry, SqlException)
{
	bind(param, res);
	return fetchOnce(db);
}
template <typename params_t, typename results_t>
template <typename LAMBDA>
inline void kr::sql::Statement<params_t, results_t>::fetch(MySQL & db, const paramPtrTypes &param, const resultPtrTypes &res, LAMBDA lambda) throws(ThrowRetry, SqlException)
{
	bind(param, res);
	fetch(db, lambda);
}
template <typename params_t, typename results_t>
template <typename LAMBDA> 
inline void kr::sql::Statement<params_t, results_t>::fetch(MySQL & db, const paramPtrTypes &param, LAMBDA lambda) throws(ThrowRetry, SqlException)
{
	bindParams(param);
	results_t res;
	bindResults(&res);
	fetch(db, [&]{ return res.call(lambda); });
}
template <typename params_t, typename results_t>
template <typename LAMBDA>
inline void kr::sql::Statement<params_t, results_t>::fetch(MySQL & db, LAMBDA lambda) throws(ThrowRetry, SqlException)
{
	Fetcher fetcher = this;
	while(fetcher.fetch(db))
	{
		if (meta::returnBool(lambda)()) break;
	}
}
template <typename params_t, typename results_t>
template <typename LAMBDA>
inline int kr::sql::Statement<params_t, results_t>::fetchCount(MySQL & db, LAMBDA lambda) throws(ThrowRetry, SqlException)
{
	Fetcher fetcher = this;
	int count = 0;
	while (fetcher.fetch(db))
	{
		count++;
		if (meta::returnBool(lambda)()) break;
	}
	return count;
}

template <typename params_t>
inline void kr::sql::Statement<params_t, void>::bind(const paramPtrTypes &values) noexcept
{
	bindParams(values);
}
template <typename params_t>
inline void kr::sql::Statement<params_t, void>::bindParams(const paramTypes * values) noexcept
{
	m_paramSet = values;
	PreparedStatementImpl::_bindParam(m_paramSet.m_params);
}
template <typename params_t>
inline void kr::sql::Statement<params_t, void>::bindParams(const paramPtrTypes &values) noexcept
{
	m_paramSet = values;
	PreparedStatementImpl::_bindParam(m_paramSet.m_params);
}
template <typename params_t>
inline kr::sql::Statement<params_t, void>::Statement(MySQL& sql, Text text) throws(SqlException) 
	: PreparedStatementImpl(sql,text)
{
}
template <typename params_t>
void kr::sql::Statement<params_t, void>::execute(const paramPtrTypes &values) noexcept
{
	bindParams(values);
	execute();
}
template <typename params_t>
void kr::sql::Statement<params_t, void>::execute(MySQL & db, const paramPtrTypes &values) noexcept
{
	bindParams(values);
	execute(db);
}
template <typename params_t>
void kr::sql::Statement<params_t, void>::execute(MySQL & db, const paramTypes *values) noexcept
{
	bindParams(values);
	execute(db);
}
template <typename params_t>
void kr::sql::Statement<params_t, void>::executeWith(MySQL & db, const paramTypes &values) noexcept
{
	bindParams(&values);
	execute(db);
}

