#pragma once


template <size_t size>
void kr::Random::fill(void * value) noexcept
{
	*((dword*&)value)++ = getDword();
	fill<size-4>(value);
}
template <typename T>
T kr::Random::get() noexcept
{
	return (T)getDword();
}
template <typename T>
T kr::Random::get(T value) noexcept
{
	using utype = std::make_unsigned_t<T>;
	return (T)(get<utype>() % (utype)value);
}
template <typename T>
T kr::Random::getSigned(T value) noexcept
{
	return get<T>(value*2) - value;
}
template <typename T>
T kr::Random::get(T from, T to) noexcept
{
	return get<T>(to - from) + from;
}
