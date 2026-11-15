#ifdef DRAWCALC_INCLUDING
#error  DRAWCALC_INCLUDING
#endif //  DRAWCALC_INCLUDING
#define DRAWCALC_INCLUDING
#ifndef DRAWCALC_INCLUDED
#define DRAWCALC_INCLUDED

#include "db_time.cpp"
#include "ctrl.cpp"
#include "calc.cpp"

struct drawcalc_t
{
	calculator_t::varlist_t vars;
	
	void setdefaults()
	{
		addvar( getseconds(), "now" );
		eval( "giga=1e3[mega=1e3[kilo=1e3]]" );
		eval( "pico=1e-3[nano=1e-3[micro=1e-3[milli=1e-3]]]" );
		eval( "quart=13[weeks=7[days=24[hours=60[mins=60[secs=1]]]]]" );
	}
	
	void addvar( calculator_t::calc_t val, char const* varname, uint varlen )
	{
		calculator_t::var_t const* var = getvar( varname, varlen );
		ENSURE( !var, "variable %*s already set\n", varname, varlen );
		vars.add( calculator_t::var_t::from_str( val, varname, varlen ) );
	}

	template <uint len>
	void addvar( calculator_t::calc_t val, char const(& varname)[len] )
	{
		static_assert( len );
		addvar( val, varname, len - 1 );
	}
	
	void eval( char const* str, uint len = -1 )
	{
		len = d_strlen( str, len );
		calculator_t calc( str, len, vars );
		if( !calc.parse() )
		{
			calc.print_errors();
			ENSURE( false, "calculation failed\n" );
		}
		calc.calc();
		vars = calc.getvars();
	}
	
	void printvar( char const* varname, uint varlen )
	{
		calculator_t::var_t const* var = getvar( varname, varlen );
		if( var )
			d_fprint( "%*s = %5d\n", var->len, var->name, var->val );
		else
			d_fprint( "%*s not set\n", varlen, varname );
	}

	void printvars()
	{
		if( !vars.num() )
		{
			d_fprint( "no variables set\n" );
		} else {
			uint maxvarlen = 0;
			for( uint i = 0; i < vars.num(); i++ )
				maxvarlen = d_max( maxvarlen, vars.get( i )->len );
			for( uint i = 0; i < vars.num(); i++ )
			{
				calculator_t::var_t * var = vars.get( i );
				d_fprint( "%*,*s = %5d\n", var->len, maxvarlen, var->name, var->val );
			}
		}
	}

	calculator_t::var_t const* getvar( char const* name, uint len ) const
	{
		for( uint i = 0; i < vars.num(); i++ )
		{
			calculator_t::var_t const* ivar = vars.get( i );
			if( ( ivar->len == len ) &&
				d_memcmp( ivar->name, name, len ) )
				return( ivar );
		}
		return( nullptr );
	}

	template <uint len>
	calculator_t::var_t const* getvar( char const(& name)[len] ) const
	{
		static_assert( len );
		return( getvar( name, len - 1 ) );
	}

	template <uint len>
	calculator_t::calc_t getval( char const(& name)[len] ) const
	{
		calculator_t::var_t const* var = getvar( name );
		ENSURE( var, "variable %s not set\n", name );
		return( var->val );
	}
	
	template <uint len>
	uint getintval( char const(& name)[len] ) const
	{
		calculator_t::var_t const* var = getvar( name );
		ENSURE( var, "variable %s not set\n", name );
		return( var->val < 0.5f ? 0 : (uint)( d_abs( var->val ) + 0.5f ) );
	}
};

#endif // DRAWCALC_INCLUDED
#undef DRAWCALC_INCLUDING

