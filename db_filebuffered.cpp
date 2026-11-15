__GIT_SYMLINKS__
target=../_inc/db_filebuffered.cpp
target_resolved=/dibom/src/c/_inc/db_filebuffered.cpp
kind=file
sha256=73c7bb16be0ae878d7eaa38f81c059742fb766fc519f9bd62fe313e1c28c91cc
size=2041
created=2026-11-15T11:20:55.371756
--END-HEADER--
#ifndef FILEBUFFERED_INCLUDED
#define FILEBUFFERED_INCLUDED

#include "db_mem.cpp"
#include "db_lz4i.cpp"
#include "db_lz4threaded.cpp"

template <typename buffered_type> class filebuffered
{
protected:
	filebuffered( const filebuffered &_filebuffered ) = delete;
	filebuffered operator =( const filebuffered &_filebuffered ) = delete;
	
	char const* filename;
	
public:
	filebuffered( char const* _filename )
	{
		filename = makebuf( _filename );
	}
	
	~filebuffered()
	{
		dropa( filename );
	}

	buffered_type* read()
	{
		buffered_type* buffer = nullptr;
		if( filename )
		{
			byte* filebuf = nullptr;
			int filebufsize = 0;
			int filelen = 0;
			byte* data = nullptr;
			int datasize = 0;
			int datalen = 0;
			if( lz4::decompress_threaded( filename, &data, &datasize, &datalen, 
										  &filebuf, &filebufsize, &filelen, null, 0 ) )
			{
				if( datalen == sizeof( *buffer ) )
				{
					buffer = (buffered_type*)data;
					data = nullptr;
					datasize = 0;
				}
			}
			dropa( data );
			dropa( filebuf );
		}
		return( buffer );
	}
	
	buffered_type* read_create()
	{
		buffered_type* buffer = read();
		if( !buffer )
		{
			buffer = new buffered_type();
			write( buffer );
		}
		return( buffer );
	}
	
	static buffered_type* load( char const* _filename = nullptr )
	{
		buffered_type* buffer = nullptr;
		if( _filename )
		{
			filebuffered _filebuffered( _filename );
			buffer = _filebuffered.read();
			if( !buffer )
				buffer = new buffered_type();
			_filebuffered.write( buffer );
		} else {
			buffer = new buffered_type();
		}
		return( buffer );
	}
	
	bool write( buffered_type* buffer )
	{
		bool okay = false;
		byte* filebuf = nullptr;
		int filebufsize = 0;
		int filelen = 0;
		if( buffer && filename )
		{
			filelen = lz4::compress( (byte const*)buffer, sizeof( *buffer ),
			                              &filebuf, &filebufsize, lz4::max_level );
			if( filelen > 0 )
				okay = writefile( filebuf, filelen, filename );
		}
		dropa( filebuf );
		return( okay );
	}
};


#endif // FILEBUFFERED_INCLUDED

