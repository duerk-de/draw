#ifdef CMDARG_INCLUDING
#error  CMDARG_INCLUDING
#endif //  CMDARG_INCLUDING
#define CMDARG_INCLUDING
#ifndef CMDARG_INCLUDED
#define CMDARG_INCLUDED

#include "ctrl.cpp"

struct cmdarg_t
{
	static constexpr inline bool is_whitespace( char const& c )
	{
		return( c == '\t' || c == '\r' || c == '\n' || c == ' ' );
	}

	static inline bool is_varbeg( char c )
	{
		return( ( c >= 'A' && c <= 'Z' ) ||
				( c >= 'a' && c <= 'z' ) ||
				c == '_' );
	}
	
	static inline bool is_var( char c )
	{
		return( is_varbeg( c ) ||
				( c >= '0' && c <= '9' ) );
	}
	
	static inline bool is_sep( char c )
	{
		return( c == ':' );
	}
	
	static inline bool is_comment( char c )
	{
		return( c == '#' );
	}
	
	static inline bool is_end( char c )
	{
		return( c == '\0' );
	}
	
	void parse( char const* arg )
	{
		uint p = 0;
		while( is_whitespace( arg[p] ) )
			p++;
		if( !is_varbeg( arg[p] ) )
		{
			if( is_comment( arg[p] ) || is_end( arg[p] ) )
			{
				var = nullptr;
				varlen = 0;
			} else {
				var = arg + p;
				varlen = 0;
			}
			val = nullptr;
			vallen = 0;
			return;
		}
		uint p0 = p;
		while( is_var( arg[p] ) )
			p++;
		var = arg + p0;
		varlen = p - p0;
		while( is_whitespace( arg[p] ) )
			p++;
		if( !is_sep( arg[p] ) )
		{
			if( is_end( arg[p] ) )
			{
				val = nullptr;
				vallen = 0;
			} else {
				val = arg + p;
				vallen = 0;
			}
			return;
		} else {
			p++;
		}
		while( is_whitespace( arg[p] ) )
			p++;
		p0 = p;
		while( !is_end( arg[p] ) )
			p++;
		while( p > p0 && is_whitespace( arg[p-1] ) )
			p--;
		val = arg + p0;
		vallen = p - p0;
	}
	
	char const* var;
	uint varlen;
	char const* val;
	uint vallen;
	
	cmdarg_t( cmdarg_t const& ) = default;
	cmdarg_t & operator =( cmdarg_t const& ) = default;

	cmdarg_t() :
		var( nullptr ),
		varlen( 0 ),
		val( nullptr ),
		vallen( 0 )
	{ }
	
	cmdarg_t( char const* arg ) :
		cmdarg_t()
	{
		parse( arg );
	}
	
	inline bool is_empty() const
	{
		return( !var );
	}
	
	inline bool is_cmd() const
	{
		return( var && varlen && !val );
	}
	
	inline bool is_var( char const* _var )
	{
		if( !var )
			return( false );
		uint i;
		for( i = 0; i < varlen && _var[i]; i++ )
			if( _var[i] != var[i] )
				break;
		if( !( i == varlen && !_var[i] ) )
			return( false );
		return( true );
	}
};

#endif // CMDARG_INCLUDED
#undef CMDARG_INCLUDING

