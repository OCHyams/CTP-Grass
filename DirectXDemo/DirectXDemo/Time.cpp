#include "Time.h"
#include "dxerr.h"

Clock::Clock()
{
	m_frequency = getFrequency();
	reset();
}

const LARGE_INTEGER& Clock::getStart() const
{
	return m_start;
}

const LARGE_INTEGER& Clock::getLast() const
{
	return m_last;
}

const LARGE_INTEGER& Clock::getCurrent() const
{
	return m_current;
}

double Clock::getFrequency() const
{
	LARGE_INTEGER f;

	if (QueryPerformanceFrequency(&f) == false)
	{
		DXTRACE_MSG(L"QueryPerformanceFrequency() failed");
	}
	return (double)f.QuadPart;

}

void Clock::reset()
{
	getTime(m_start);
	m_current = m_start;
	m_last = m_start;
}

void Clock::getTime(LARGE_INTEGER& time) const
{
	QueryPerformanceCounter(&time);
}

void Clock::updateGameTime(Time& _t)
{
	getTime(m_current);
	_t.time = (m_current.QuadPart - m_start.QuadPart) / m_frequency;
	_t.deltaTime = (m_current.QuadPart - m_last.QuadPart) / m_frequency;
	m_last = m_current;
}
