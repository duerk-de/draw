#ifdef READER_INCLUDING
#error  READER_INCLUDING
#endif //  READER_INCLUDING
#define READER_INCLUDING
#ifndef READER_INCLUDED
#define READER_INCLUDED

#include "ctrl.cpp"
#include "db_file.cpp"
#include "db_str.cpp"

struct reader_line_t
{
	virtual void set( char const* ptr, uint l ) = 0;
	virtual void add( uint b, uint e ) = 0;
};

class reader_t
{
protected:
	buffileclass file;
	lint fpos;

	reader_t( char const* filename, uint filenamelen = -1 )
	{
		set( filename, filenamelen );
	}
	
	void set( char const* filename, uint filenamelen = -1 )
	{
		file.close();
		ENSURE( filename, "no file name given\n" );
		file.set( filename, 'r', 1 << 20, 1 << 20, filenamelen );
		ENSURE( file.open(), "could not open file %s\n", filename );
		fpos = 0;
	}
	
public:
	virtual ~reader_t()
	{
		file.close();
	}
	
	static constexpr inline bool is_whitespace( char const& c )
	{
		return( c == '\t' || c == '\r' || c == ' ' );
	}
	
	static constexpr inline bool is_lineend( char const& c )
	{
		return( c == '\n' );
	}
	
protected:
	virtual void nextline( byte const* & ptr, uint & len ) = 0;
	virtual bool data_available() const = 0;
	
public:
	bool parsenextline( reader_line_t & line )
	{
		if( !data_available() )
		{
			line.set( nullptr, 0 );
			return( false );
		}
		byte const* ptr;
		uint len;
		nextline( ptr, len );
		line.set( (char const*)ptr, len );
		uint i = 0;
		do
		{
			while( i < len && is_whitespace( ptr[i] ) )
				i++;
			uint b = i;
			while( i < len && !is_whitespace( ptr[i] ) )
				i++;
			uint e = i;
			if( b == e )
				break;
			line.add( b, e );
		}
		while( i < len );
		return( true );
	}
};

class reader_forward_t : public reader_t
{
public:
	reader_forward_t( char const* filename, uint filenamelen = -1 ) :
		reader_t( filename, filenamelen )
	{
		fpos = 0;
	}
	 
	virtual ~reader_forward_t()
	{ }
	
protected:
	virtual bool data_available() const
	{
		return( fpos < file.file.size );
	}

	virtual void nextline( byte const* & ptr, uint & len )
	{
		len = file.file.size - fpos < file.bufsize2 ? (int)( file.file.size - fpos ) : file.bufsize2;
		ENSURE( len > 0, "no space left to find any line, fpos: %l, bufsize2: %i, len: %i\n", fpos, file.bufsize2, len );
		ptr = file.get( fpos );
		ENSURE( ptr != nullptr, "could not read from position %l of %s\n", fpos, file.file.name );
		uint i = 0;
		for( ; i < len && !is_lineend( ptr[i] ); i++ );
		ENSURE( i < len || file.file.size == fpos + len + 1, "line from position %l larger than buffer of %i\n", fpos, file.bufsize2 );
		len = i;
		fpos += len + 1;
	}
};

class reader_backward_t : public reader_t
{
public:
	reader_backward_t( char const* filename, uint filenamelen = -1 ) :
		reader_t( filename, filenamelen )
	{
		fpos = file.file.size;
		if( fpos )
			--fpos;
	}
	 
	virtual ~reader_backward_t()
	{ }
	
protected:
	virtual bool data_available() const
	{
		return( fpos > 0 );
	}

	virtual void nextline( byte const* & ptr, uint & len )
	{
		len = fpos < file.bufsize2 ? (int)fpos : file.bufsize2;
		ENSURE( len > 0, "no space left to find any line, fpos: %l, bufsize2: %i, len: %i\n", fpos, file.bufsize2, len );
		ptr = file.get( fpos - len );
		ENSURE( ptr != nullptr, "could not read from position %l of %s\n", fpos - len, file.file.name );
		int i = len;
		for( i--; i >= 0 && !is_lineend( ptr[i] ); i-- );
		ENSURE( i >= 0 || len == fpos, "line in front of position %l larger than buffer of %i\n", fpos, file.bufsize2 );
		fpos -= len - i;
		i++;
		len -= i;
		ptr += i;
	}
};

#endif // READER_INCLUDED
#undef READER_INCLUDING

