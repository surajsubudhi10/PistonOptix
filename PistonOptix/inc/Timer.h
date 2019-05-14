// This code is part of the NVIDIA nvpro-pipeline https://github.com/nvpro-pipeline/pipeline

#pragma once

#ifndef TIMER_H
#define TIMER_H

#if defined(_WIN32)
#include <Windows.h>
#else
#include <sys/time.h>
#endif


/*! \brief A simple timer class.
  * This timer class can be used on Windows and Linux systems to
  * measure time intervals in seconds.
  * The timer can be started and stopped several times and accumulates
  * time elapsed between the start() and stop() calls. */
class Timer
{
public:
	//! Default constructor. Constructs a Timer, but does not start it yet. 
	Timer();

	//! Default destructor.
	~Timer();

	//! Starts the timer.
	void start();

	//! Stops the timer.
	void stop();

	//! Resets the timer.
	void reset();

	//! Resets the timer and starts it.
	void restart();

	//! Returns the current time in seconds.
	double getTime() const;

	//! Return whether the timer is still running.
	bool isRunning() const { return m_running; }

private:
#if defined(_WIN32)
	typedef LARGE_INTEGER Time;
#else
	typedef timeval Time;
#endif

private:
	double calcDuration(Time begin, Time end) const;

private:
#if defined(_WIN32)
	LARGE_INTEGER m_freq;
#endif
	Time   m_begin;
	bool   m_running;
	double m_seconds;
};

#endif // TIMER_H
