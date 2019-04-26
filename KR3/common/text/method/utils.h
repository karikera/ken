#pragma once

namespace kr
{
	template<class Derived, typename Component, class Info>
	inline OutStream<Derived, Component, Info>& endl(OutStream<Derived, Component, Info>& _Ostr)
	{
		_Ostr << (Component)'\n';
		return (_Ostr);
	}
}