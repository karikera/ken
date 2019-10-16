#include "stdafx.h"
#include "cascader.h"

kr::ThreadCascader::ThreadCascader() noexcept
	:m_promise(nullptr)
{
}

kr::ThreadCascader::PromiseImpl::PromiseImpl(ThreadCascader * that) noexcept
	:m_that(that)
{
	m_target = PostTarget::getCurrent();
}

void kr::ThreadCascader::PromiseImpl::resolve() noexcept
{
	m_target->post([this](void*) {
		_resolve();
		if (m_that->m_promise == this)
		{
			m_that->m_promise = nullptr;
		}
	});
}

void kr::ThreadCascader::PromiseImpl::reject(std::exception_ptr v) noexcept
{
	new(_rejectValue()) std::exception_ptr(move(v));
	m_target->post([this](void*) {
		_rejectCommit();
		if (m_that->m_promise == this)
		{
			m_that->m_promise = nullptr;
		}
	});
}
