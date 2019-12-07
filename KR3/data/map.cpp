#include "stdafx.h"
#include "map.h"

using namespace kr;

size_t std::hash<kr::_pri_::MapKeyData>::operator ()(const kr::_pri_::MapKeyData& key) const noexcept
{
	return kr::mem::hash(key.m_buffer, key.m_size);
}

bool _pri_::MapKeyData::operator ==(const MapKeyData& other) const noexcept
{
	if (m_size != other.m_size) return false;
	return memcmp(m_buffer, other.m_buffer, m_size) == 0;
}
bool _pri_::MapKeyData::operator !=(const MapKeyData& other) const noexcept
{
	if (m_size != other.m_size) return true;
	return memcmp(m_buffer, other.m_buffer, m_size) != 0;
}

_pri_::MapKey::~MapKey() noexcept
{
	krfree((void*)m_buffer);
}

_pri_::MapKey::MapKey(MapKey&& _move) noexcept
{
	m_buffer = _move.m_buffer;
	m_size = _move.m_size;
	_move.m_buffer = nullptr;
}

_pri_::MapKeyStatic::operator const _pri_::MapKey& () const noexcept
{
	return *static_cast<const MapKey*>(static_cast<const MapKeyData*>(this));
}