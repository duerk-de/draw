#ifdef DATACALC_INCLUDING
#error  DATACALC_INCLUDING
#endif //  DATACALC_INCLUDING
#define DATACALC_INCLUDING
#ifndef DATACALC_INCLUDED
#define DATACALC_INCLUDED

#include "ctrl.cpp"
#include "data.cpp"
#include "calc.cpp"

struct datacalc_t
{
	struct op_col_t : public calculator_t::op_t
	{
		data_t & data;
		op_col_t( data_t & _data ) :
			op_t( "col", right, false ),
			data( _data )
		{ }
		virtual calculator_t::calc_t calc( calculator_t::calc_t const(& args)[calculator_t::maxargs] ) const
		{
			calculator_t::calc_t d = args[0];
			if( d < -0.5 )
				return( 0 );
			uint u = (uint)( d + 0.5 );
			if( !u )
				return( data.geti() );
			return( data.getval( u - 1 ) );
		}
	};

	struct op_maxcol_t : public calculator_t::op_t
	{
		data_t & data;
		op_maxcol_t( data_t & _data ) :
			op_t( "colmax", right, false ),
			data( _data )
		{ }
		virtual calculator_t::calc_t calc( calculator_t::calc_t const(& args)[calculator_t::maxargs] ) const
		{
			calculator_t::calc_t d = args[0];
			if( d < -0.5 )
				return( 0 );
			uint u = (uint)( d + 0.5 );
			if( !u )
				return( 0 );
			return( data.getmaxval( u - 1 ) );
		}
	};

	struct op_mincol_t : public calculator_t::op_t
	{
		data_t & data;
		op_mincol_t( data_t & _data ) :
			op_t( "colmin", right, false ),
			data( _data )
		{ }
		virtual calculator_t::calc_t calc( calculator_t::calc_t const(& args)[calculator_t::maxargs] ) const
		{
			calculator_t::calc_t d = args[0];
			if( d < -0.5 )
				return( 0 );
			uint u = (uint)( d + 0.5 );
			if( !u )
				return( 0 );
			return( data.getminval( u - 1 ) );
		}
	};

	struct op_scol_t : public calculator_t::op_t
	{
		data_t & data;
		op_scol_t( data_t & _data ) :
			op_t( "var", right, false ),
			data( _data )
		{ }
		virtual calculator_t::calc_t calc( calculator_t::calc_t const(& args)[calculator_t::maxargs] ) const
		{
			calculator_t::calc_t d = args[0];
			if( d < -0.5 )
				return( 0 );
			uint u = (uint)( d + 0.5 );
			if( !u )
				return( 0 );
			return( data.getsval( u - 1 ) );
		}
	};

	struct op_dcol_t : public calculator_t::op_t
	{
		data_t & data;
		op_dcol_t( data_t & _data ) :
			op_t( "deri", right, false ),
			data( _data )
		{ }
		virtual calculator_t::calc_t calc( calculator_t::calc_t const(& args)[calculator_t::maxargs] ) const
		{
			calculator_t::calc_t d = args[0];
			if( d < -0.5 )
				return( 0 );
			uint u = (uint)( d + 0.5 );
			if( !u )
				return( data.getdi() );
			return( data.getdval( u - 1 ) );
		}
	};

	calculator_t::varlist_t vars;

	datacalc_t( calculator_t::varlist_t const& _vars ) :
		vars( _vars )
	{ }
	
	/*
	datacalc_t( drawcalc_t const& drawcalc )
	{
		vars = drawcalc.vars;
	}

	template <uint len>
	void addvar( calculator_t::calc_t val, char const(& varname)[len] )
	{
		vars.add( drawcalc_t::varadd( val, varname ) );
	}
	*/
	
	calculator_t * mk( char const* str, uint len, data_t & _data )
	{
		calculator_t * calc = new calculator_t( str, d_strlen( str, len ), vars );
		calc->ops.last()->add( new op_col_t( _data ) );
		calc->ops.last()->add( new op_maxcol_t( _data ) );
		calc->ops.last()->add( new op_mincol_t( _data ) );
		calc->ops.last()->add( new op_scol_t( _data ) );
		calc->ops.last()->add( new op_dcol_t( _data ) );
		if( !calc->parse() )
		{
			calc->print_errors();
			ENSURE( false, "calculation failed\n" );
		}
		return( calc );
	}
};

#endif // DATACALC_INCLUDED
#undef DATACALC_INCLUDING

