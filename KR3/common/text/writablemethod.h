#pragma once

#include "buffer.h"

#define KR_WRITABLE_METHOD(name, szable, _copyTo, _sizeAs) \
			private: \
			using __this_##name = This;\
			class __cls__##name : public ::kr::Bufferable<__cls__##name, ::kr::BufferInfo<::kr::AutoComponent, method::CopyTo, szable, true>>{\
				const __this_##name* const m_this;\
				public: __cls__##name(const __this_##name* _this) noexcept : m_this(_this){} \
				template <typename C> size_t $sizeAs() const noexcept { return _sizeAs; } \
				template <typename C> size_t $copyTo(C* dest) const noexcept { return _copyTo; } \
			}; \
			public: __cls__##name name() const noexcept{ return this; }
