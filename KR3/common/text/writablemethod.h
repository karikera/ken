#pragma once

#include "buffer.h"

#define KR_WRITABLE_METHOD(maincls, name, szable, _copyTo, _sizeAs) \
			private: class __cls__##name : public ::kr::Bufferable<__cls__##name, ::kr::BufferInfo<::kr::AutoComponent, method::CopyTo, szable, true>>{\
				const maincls* const m_this;\
				public: __cls__##name(const maincls* _this) noexcept : m_this(_this){} \
				template <typename C> size_t $sizeAs() const noexcept { return _sizeAs; } \
				template <typename C> size_t $copyTo(C* dest) const noexcept { return _copyTo; } \
			}; \
			public: __cls__##name name() const noexcept{ return this; }
