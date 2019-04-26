#include "stdafx.h"

#ifdef WIN32

#include "framesync.h"
#include <KRMessage/pump.h>

kr::FrameSync::FrameSync()
{
	m_wait = 0_s;
	m_skip = 4;
}
kr::FrameSync::FrameSync(duration framewait)
{
	m_wait = framewait;
	m_skip = 4;
	m_next = timepoint::now();
}
bool kr::FrameSync::sync()
{
	timepoint now = timepoint::now();
	duration left = m_next - now;

	if(m_skip >= 4 || left < -1_s)
	{
		m_next = now + m_wait;
	}
	else if(left >= -5_s)
	{
		m_next += m_wait;
		if (left > 0_s)
		{
			EventPump::getInstance()->sleep(left);
		}
	}
	else
	{
		m_skip++;
		return false;
	}

	m_skip=0;
	return true;
}

#endif