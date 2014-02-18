#pragma once
#include "time.h"

namespace cbl
{
	class stopwatch
	{
	public:
		stopwatch();
		void	start();
		void	stop();
		time	elapsed() const;
		time	reset();

	private:
		bool				m_running;
		timespec			m_start;
		mutable timespec	m_now;
	};
}