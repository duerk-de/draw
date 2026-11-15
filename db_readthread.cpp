__GIT_SYMLINKS__
target=../_inc/db_readthread.cpp
target_resolved=/dibom/src/c/_inc/db_readthread.cpp
kind=file
sha256=b881528269a4594545ec840cbf8a3b400d738117b957f3f26af210e2f87974ef
size=5340
created=2026-11-15T11:20:55.389342
--END-HEADER--
#ifndef DB_READTHREAD_INCLUDED
#define DB_READTHREAD_INCLUDED


#include "db_file.cpp"
#include "db_str.cpp"
#include "db_threadclass.cpp"

#include <atomic>

class splitbuffer
{
protected:
	byte* buf;
	std::atomic<int> readpos;
	std::atomic<int> writepos;
	std::atomic<int> size;
	splitbuffer( const splitbuffer &_t ) = delete;
	splitbuffer &operator =( const splitbuffer &_t ) = delete;
public:
	splitbuffer()
	{
		buf = nullptr;
		writepos = 0;
		readpos = 0;
		size = 0;
	}
	~splitbuffer()
	{
		delete [] buf;
	}
	bool alloc_move( int newsize )
	{
		std::atomic_thread_fence( std::memory_order_release );
		if( newsize > writepos )
		{
			byte* newbuf = new byte[newsize];
			if( !newbuf )
				return( false );

			if( readpos )
			{
				cpy( newbuf, getreadbuf(), getreadsize() );
				writepos -= readpos;
				readpos = 0;
			}
			delete [] buf;
			buf = newbuf;
			size = newsize;
		} else {
			move();
		}
		std::atomic_thread_fence( std::memory_order_acquire );
		return( true );
	}
	void move()
	{
		std::atomic_thread_fence( std::memory_order_acquire );
		if( readpos )
		{
			cpy( buf, getreadbuf(), getreadsize() );
			writepos -= readpos;
			readpos = 0;
		}
		std::atomic_thread_fence( std::memory_order_release );
	}
	
	inline byte* getreadbuf() const
	{
		return( buf + readpos );
	}
	inline int getreadpos() const
	{
		return( readpos );
	}
	inline int getreadsize() const
	{
		return( writepos - readpos );
	}
	inline void setread( int length )
	{
		readpos += length;
	}
	
	inline byte* getwritebuf() const
	{
		return( buf + writepos );
	}
	inline int getwritepos() const
	{
		return( writepos );
	}
	inline int getwritesize() const
	{
		return( size - writepos );
	}
	inline void setwritten( int length )
	{
		writepos += length;
	}
};

class semaphore
{
protected:
	std::atomic <uint> counter;
	std::mutex mutex;

public:
	semaphore( uint _counter = 1 ) : counter( _counter )
	{
	}
	
	void acquire()
	{
		while( true )
		{
			while( !counter );
			mutex.lock();
			if( counter )
			{
				counter--;
				mutex.unlock();
				return;
			}
			mutex.unlock();
		}
	}
	
	void release()
	{
		mutex.lock();
		counter++;
		mutex.unlock();
	}
};


class readthread : public threadclass
{
protected:
	enum e_state : int { state_notinitialized,
						state_idle,
						state_readblock,
						state_finished,
						state_file_not_openable,
						state_error };
	std::atomic <int> state;
	std::atomic <int> size_to_read;
	std::atomic <int> size_read;
	std::atomic <byte*> buf;
	
	fileclass file;
	
	semaphore loop;
	semaphore data;
	int totalreadsize;
	char msg[4096];

	e_state open( char const* filename )
	{
		msg[0] = '\0';
		e_state istate = state_idle;
		file.set( filename, 'r' );
		if( !file.open() )
		{
			istate = state_file_not_openable;
			d_fstrn( msg, lenof(msg), "could not open file %s", file.name );
		}
		return( istate );
	}

	e_state read( byte* _buf, int to_read, int* _read )
	{
		msg[0] = '\0';
		e_state istate = state_idle;
		if( file.pos + to_read > file.size )
			to_read = file.size - file.pos;
		lint filepos = file.pos;
		if( file.read( _buf, to_read ) )
		{
			*_read = to_read;
			if( file.pos == file.size )
				istate = state_finished;
		} else {
			*_read = 0;
			d_fstrn( msg, lenof(msg), "error reading file %s %l+%i=%l/%l", 
					file.name, filepos, to_read, filepos + to_read, file.size );
			istate = state_error;
		}
		totalreadsize = (int)file.pos;
		return( istate );
	}

	virtual void main()
	{
		loop.acquire();
		if( state == state_readblock )
		{
			std::atomic_thread_fence( std::memory_order_acquire );
			int _read;
			e_state istate = read( (byte*)buf, (int)size_to_read, &_read );
			std::atomic_thread_fence( std::memory_order_release );
			size_read = _read;
			if( istate != state_idle )
				keeprunning = false;
			state = istate;
			data.release();
		}
	}
	virtual void stopping()
	{
		file.close();
	}
public:
	bool init( char const* _filename )
	{
		totalreadsize = 0;
		e_state istate = open( _filename );
		state = istate;
		return( istate == state_idle || state == state_finished );
	}
	bool read_inline( byte* _buf, int _size_to_read, int* _size_read )
	{
		e_state istate;
		if( state == state_idle )
		{
			istate = read( _buf, _size_to_read, _size_read );
		} else {
			d_fstrn( msg, lenof(msg), "not in idle state while trying to read" );
			*_size_read = 0;
			istate = state_error;
		}
		state = istate;
		return( istate == state_idle || state == state_finished );
	}
	void read_threaded( byte* _buf, int _size_to_read )
	{
		if( state == state_idle )
		{
			size_to_read = _size_to_read;
			buf = _buf;
			data.acquire();
			state = state_readblock;
			loop.release();
		} else {
			d_fstrn( msg, lenof(msg), "not in idle state while trying to read" );
			state = state_error;
		}
	}
	int wait_for_reading()
	{
		data.acquire();
		data.release();
		return( (int)size_read );
	}
	void cancel_loop()
	{
		keeprunning = false;
		loop.release();
	}
	inline bool ready() const
	{
		return( keeprunning && state == state_idle );
	}
	inline bool file_not_openable() const
	{
		return( state == state_file_not_openable );
	}
	inline bool finished()
	{
		return( state == state_finished );
	}
	inline char const* geterrmsg() const
	{
		return( msg );
	}
	inline int const& gettotalreadsize() const
	{
		return( totalreadsize );
	}
};


#endif // DB_READTHREAD_INCLUDED

