#ifdef USERVAL_INCLUDING
#error USERVAL_INCLUDING
#endif // USERVAL_INCLUDING
#define USERVAL_INCLUDING
#ifndef USERVAL_INCLUDED
#define USERVAL_INCLUDED

#include "ctrl.cpp"

struct userval_t
{
	char const* val;
	uint len;
	bool isset;
	
	constexpr userval_t() :
		val( nullptr ),
		len( 0 ),
		isset( false )
	{ }
	
	constexpr userval_t( char const* _val, uint _len ) :
		val( _val ),
		len( _len ),
		isset( true )
	{ }

	constexpr userval_t( userval_t const& v, bool _isset = true ) :
		val( v.val ),
		len( v.len ),
		isset( _isset ? v.isset : false )
	{ }
	
	constexpr userval_t& operator =( userval_t const & ) = default;

	bool set( char const* _val, uint _len )
	{
		if( isset )
			return( false );
		val = _val;
		len = _len;
		isset = true;
		return( true );
	}
};


#endif // USERVAL_INCLUDED
#undef USERVAL_INCLUDING

