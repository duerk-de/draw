__GIT_SYMLINKS__
target=../_inc/db_threadclass.cpp
target_resolved=/dibom/src/c/_inc/db_threadclass.cpp
kind=file
sha256=6c623f831a64eb75db65ac190035e9f7e2ca6dc123b6e3f6a3825e9d6a4b8321
size=4093
created=2026-11-15T11:20:55.395481
--END-HEADER--
#ifndef DB_THREADCLASS_INCLUDED
#define DB_THREADCLASS_INCLUDED

#include "db_time.cpp"

#include <atomic>
#include <mutex>

#ifdef LINUX
	#include <pthread.h>
#else
	// #include <thread>
	// #include "mingw.thread.h"
	#include "mingw.mutex.h"
	#include <process.h>
#endif

/// provides thread memory locking functionality
class lockclass
{
protected:
	std::mutex mutex;
	std::atomic<bool> dataaccess;						///< true if locked
public:
	inline lockclass()		///< constructor
	{
		dataaccess = false;
	}
	inline void lock()								///< waits for unlocked state and locks
	{
		mutex.lock();
		dataaccess = true;
	}
	inline bool try_lock()
	{
		bool okay;
		if( ( okay = mutex.try_lock() ) )
			dataaccess = true;
		return( okay );
	}
	inline void unlock()							///< unlocks (from any state)
	{
		dataaccess = false;
		mutex.unlock();
	}
	inline bool islocked()
	{
		return( dataaccess );
	}
};

/// virtual class, which can run in an own thread
class threadclass
{
public:
	threadclass()
	{
		keeprunning = true;
		isrunning = false;
		isstarting = false;
		isstopping = false;
		dothreadedstart = true;
		dothreadedstop = true;
		useconds_tosleep = 1000;
		#ifdef LINUX
		tid = 0;
		#endif
	}
	~threadclass()
	{
		stop();
	}
	void start( bool waitforcompletion = true, int sleep_us = -1 )			///< call for starting in an new thread
	{
		if( sleep_us > -1 )
			set_sleep_us( sleep_us );
		if( waitforcompletion )
			dothreadedstart = false;
		bool dostart = !dothreadedstart;
		
		if( dostart )
		{
			isstarting = true;
			starting();
			isstarting = false;
		}
		keeprunning = true;
		
		#ifdef LINUX
		while( pthread_create( &tid, nullptr, runthread, (void*)this ) )
			sleepus( 10000 );
		#else
		_beginthread( runthread, 0, (void*)this );
		#endif
		
		if( !dostart && waitforcompletion )
			while( isstarting )
				sleepus( 10000 );
	}
	void stop( bool kill = false )											///< call for stopping the thread
	{
		if( keeprunning )
		{
			if( kill )
				dothreadedstop = false;
			keeprunning = false;
			if( kill )
			{
				#ifdef LINUX
				sleepus( 10000 );
				pthread_exit( &tid );
				#endif
			} else {
				while( isrunning || isstopping )
					sleepus( 10000 );
			}
			if( !dothreadedstop )
				stopping();
			#ifdef LINUX
			tid = 0;
			#endif
		}
	}
	inline bool runs()
	{
		return( isrunning );
	}
	inline bool isstarted()
	{
		return( isrunning || isstarting || isstopping || keeprunning );
	}
	inline void set_sleep_us( uint const& _useconds_tosleep )
	{
		useconds_tosleep = _useconds_tosleep;
	}
protected:
	threadclass( const threadclass &_t ) = delete;
	threadclass &operator =( const threadclass &_t ) = delete;
	std::atomic<bool> isstarting;
	std::atomic<bool> isstopping;
	std::atomic<bool> isrunning;											///< true while thread is created
	std::atomic<bool> keeprunning;											///< while true main is called
	std::atomic<bool> dothreadedstart;
	std::atomic<bool> dothreadedstop;
	uint useconds_tosleep;
	#ifdef LINUX
	pthread_t tid;
	static void *runthread( void *ptr )										///< helper function, called by start
	{
		((threadclass*)ptr)->run();
		return( nullptr );
	}
	#else
	static void runthread( void *ptr )										///< helper function, called by start
	{
		((threadclass*)ptr)->run();
	}
	#endif
	virtual void starting() {}
	virtual void stopping() {}
	virtual void main() {}
	void run()																///< helper function, called by runthread
	{
		bool dostart = dothreadedstart;
		if( dostart )
		{
			isstarting = true;
			starting();
		}
		isrunning = true;
		if( dostart )
			isstarting = false;
		bool dorun = keeprunning;
		while( dorun )
		{
			main();
			dorun = keeprunning;
			if( dorun && useconds_tosleep )
				sleepus( useconds_tosleep );
		}
		bool dostop = dothreadedstop;
		if( dostop )
		{
			isstopping = true;
			stopping();
		}
		isrunning = false;
		if( dostop )
			isstopping = false;
	}
};


#endif // DB_THREADCLASS_INCLUDED
