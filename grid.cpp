#ifdef GRID_INCLUDING
#error  GRID_INCLUDING
#endif //  GRID_INCLUDING
#define GRID_INCLUDING
#ifndef GRID_INCLUDED
#define GRID_INCLUDED

#include "ctrl.cpp"
#include "userval.cpp"
#include "image.cpp"
#include "layer.cpp"
#include "drawcalc.cpp"

struct grid_t
{
	struct steps_t
	{
		struct type_t
		{
			virtual uint step( [[maybe_unused]] uint nexp ) = 0;
			virtual uint nsubsteps( [[maybe_unused]] uint nexp ) = 0;
			virtual uint substep( [[maybe_unused]] uint nexp, uint isub ) = 0;
		};
		
		struct decimal_type_t : public type_t
		{
			static constexpr uint steps_decimal = 10;
			static constexpr uint substeps_decimal[] = { 2, 5 };
			virtual uint step( [[maybe_unused]] uint nexp )
			{
				return( steps_decimal );
			}
			virtual uint nsubsteps( [[maybe_unused]] uint nexp )
			{
				return( lenof( substeps_decimal ) );
			}
			virtual uint substep( [[maybe_unused]] uint nexp, uint isub )
			{
				return( substeps_decimal[isub] );
			}
		};
		
		struct date_type_t : public type_t
		{
			template <uint n>
			struct steptype_t
			{
				uint step;
				uint nsubsteps;
				uint substeps[n];
			};
			static constexpr steptype_t<2> steps[11] = {
				{ .step =  5, .nsubsteps = 0, .substeps = { 0, 0 } },
				{ .step =  3, .nsubsteps = 0, .substeps = { 0, 0 } },
				{ .step =  4, .nsubsteps = 1, .substeps = { 2, 0 } },
				{ .step =  5, .nsubsteps = 0, .substeps = { 0, 0 } },
				{ .step =  3, .nsubsteps = 0, .substeps = { 0, 0 } },
				{ .step =  4, .nsubsteps = 1, .substeps = { 2, 0 } },
				{ .step =  6, .nsubsteps = 2, .substeps = { 2, 3 } },
				{ .step =  4, .nsubsteps = 1, .substeps = { 2, 0 } },
				{ .step =  7, .nsubsteps = 0, .substeps = { 0, 0 } },
				{ .step = 13, .nsubsteps = 0, .substeps = { 0, 0 } },
				{ .step =  4, .nsubsteps = 0, .substeps = { 0, 0 } }
			};

			inline bool instep( uint nexp )
			{
				return( nexp < lenof(steps) );
			}
			
			virtual uint step( uint nexp )
			{
				return( instep( nexp ) ? steps[nexp].step : 10 );
			}
			virtual uint nsubsteps( uint nexp )
			{
				return( instep( nexp ) ? steps[nexp].nsubsteps : 0 );
			}
			virtual uint substep( uint nexp, uint isub )
			{
				return( steps[nexp].substeps[isub] );
			}
		};
		
		double exp;
		uint nexp;
		uint mult;
		double base;
		lint ilo;
		lint ihi;
		type_t * steptype;

		static lint calc_ibound( double const& base, double v, bool upper )
		{
			static constexpr double e = 1e-4;
			double w = v / base + ( upper ? e : -e );
			lint i = (lint)w;
			if( w < 0 )
			{
				if( upper )
					i--;
			} else {
				if( !upper )
					i++;
			}
			return( i );
		}

		steps_t() = default;
		steps_t( steps_t const& ) = default;
		steps_t& operator =( steps_t const& ) = default;

		steps_t( double lo, double hi, double d, type_t * _steptype ) :
			steptype( _steptype )
		{
			d *= 1.f - 1e-6f;
			ENSURE( d > 0, "grid_t::level_t(d) <= 0\n" );
			exp = 1;
			nexp = 0;
			while( exp < d )
				exp *= steptype->step( nexp++ );
			while( exp > d )
				exp /= steptype->step( --nexp );
			uint i;
			for( i = 0; i < steptype->nsubsteps( nexp ); i++ )
			{
				if( steptype->substep( nexp, i ) * exp > d )
				{
					mult = steptype->substep( nexp, i );
					break;
				}
			}
			if( i >= steptype->nsubsteps( nexp ) )
			{
				mult = 1;
				exp *= steptype->step( nexp++ );
			}
			base = exp * mult;
			ilo = calc_ibound( base, lo, false );
			ihi = calc_ibound( base, hi, true );
		}
		
		uint tonext()
		{
			uint step = steptype->step( nexp );
			ilo = ( ilo * mult + ( ilo > 0 ? ( step - 1 ) : 0 ) ) / step;
			ihi = ( ihi * mult + ( ihi < 0 ? ( step - 1 ) : 0 ) ) / step;
			exp *= steptype->step( nexp++ );
			mult = 1;
			base = exp;
			return( num() );
		}
		
		void reset()
		{
			ilo = ihi + 1;
		}
		
		uint num() const
		{
			if( ilo > ihi )
				return( 0 );
			return( ihi + 1 - ilo );
		}
		
		inline bool innext( lint i ) const
		{
			return( ( i * mult % steptype->step( nexp ) ) == 0 );
		}
		
		void print()
		{
			d_fprint( "exp=%d mult=%i base=%d ilo=%i ihi=%i\n",
					  exp, mult, base, ilo, ihi );
			for( lint i = ilo; i <= ihi; i++ )
				d_fprint( "%4I -> %6d  %b\n", i, base * (double)i,
				          innext( i ) && ( num() > 1 && steps_t( *this ).tonext() > 0 ) );
		}
	};

	
	struct level_t
	{
		enum eplot_t { line, point };

		float u;
		float v;
		float l;
		float s;
		float h;
		uint plot;
		
		bool hassteps;
		steps_t x;
		steps_t y;
		
		level_t() = default;
		level_t( level_t const& ) = default;
		level_t& operator =( level_t const& ) = default;

		level_t( float _u, float _v, float _l, float _s, float _h, uint _plot ) :
			u( _u ),
			v( _v ),
			l( _l ),
			s( _s ),
			h( _h ),
			plot( _plot ),
			hassteps( false )
		{ }

	};

	stream<level_t> levels;
	
	double g_x;
	double g_y;
	float g_l;
	float g_dl;
	double g_xy;
	float g_uv;
	
	steps_t::decimal_type_t decimal;
	steps_t::date_type_t date;
	
	grid_t() :
		g_x( 0 ),
		g_y( 0 ),
		g_l( 1 ),
		g_dl( 1 ),
		g_xy( 1 ),
		g_uv( 1 )
	{
		static constexpr float uv = 1.2;
		levels.add( level_t( uv, uv, 0.012, 1.000, 200, level_t::line ) );
		levels.add( level_t( uv, uv, 0.020, 1.000, 140, level_t::line ) );
		levels.add( level_t( uv, uv, 0.030, 1.000,  50, level_t::line ) );
		levels.add( level_t( uv, uv, 0.040, 1.000,  80, level_t::line ) );
		levels.add( level_t( uv, uv, 0.080, 1.000,  10, level_t::line ) );
		levels.add( level_t( uv, uv, 0.100, 0.075,  90, level_t::line ) );
		levels.add( level_t( uv, uv, 0.250, 0.000,   0, level_t::line ) );
		levels.add( level_t( uv, uv, 0.500, 1.000, 190, level_t::line ) );
		levels.add( level_t( uv, uv, 0.800, 1.000, 130, level_t::line ) );
		levels.add( level_t( uv, uv, 1.200, 1.000,  60, level_t::line ) );
		levels.add( level_t( uv, uv, 3.000, 0.000,   0, level_t::line ) );
	}
	
	void defaults( drawcalc_t & calc )
	{
		if( !calc.getvar( "gridx" ) && calc.getvar( "imgx" )
		  && calc.getvar( "xmax" ) && calc.getvar( "xmin" ) )
			calc.eval( "gridx = 3 * [xmax - xmin] / imgx" );
		if( !calc.getvar( "gridy" ) && calc.getvar( "imgy" )
		  && calc.getvar( "xmay" ) && calc.getvar( "ymin" ) )
			calc.eval( "gridy = 3 * [ymax - ymin] / imgy" );
		if( !calc.getvar( "gridl" ) )
			calc.eval( "gridl = 1.5" );
		if( !calc.getvar( "griddl" ) )
			calc.eval( "griddl = 1" );
		if( !calc.getvar( "gridxy" ) )
			calc.eval( "gridxy = 1" );
		if( !calc.getvar( "griduv" ) )
			calc.eval( "griduv = 1.2" );
		if( !calc.getvar( "decimal" ) )
			calc.eval( "decimal = 0" );
		if( !calc.getvar( "date" ) )
			calc.eval( "date = 1" );
		if( !calc.getvar( "gridxtype" ) )
			calc.eval( "gridxtype = decimal" );
		if( !calc.getvar( "gridytype" ) )
			calc.eval( "gridytype = decimal" );
	}
	void setup( layer_t & layer, drawcalc_t & calc )
	{
		g_x = calc.getval( "gridx" );
		g_y = calc.getval( "gridy" );
		g_l = calc.getval( "gridl" );
		g_dl = calc.getval( "griddl" );
		g_xy = calc.getval( "gridxy" );
		g_uv = calc.getval( "griduv" );
		
		steps_t::type_t * xsteptype;
		switch( calc.getintval( "gridxtype" ) )
		{
			case 0:
				xsteptype = &decimal;
				break;
			case 1:
				xsteptype = &date;
				break;
			default:
				ENSURE( false, "gridxtype out ouf range\n" );
				break;
		}
		steps_t::type_t * ysteptype;
		switch( calc.getintval( "gridytype" ) )
		{
			case 0:
				ysteptype = &decimal;
				break;
			case 1:
				ysteptype = &date;
				break;
			default:
				ENSURE( false, "gridytype out ouf range\n" );
				break;
		}

		steps_t xsteps = steps_t( layer.xmin, layer.xmax, g_x * g_xy, xsteptype );
		steps_t ysteps = steps_t( layer.ymin, layer.ymax, g_y * g_xy, ysteptype );
		for( uint ilevel = 0; ilevel < levels.num(); ilevel++ )
		{
			level_t * level = levels.get( ilevel );
			if( xsteps.num() || ysteps.num() )
			{
				level->x = xsteps;
				level->y = ysteps;
				level->hassteps = true;
				if( xsteps.num() == 1 )
					xsteps.reset();
				else
					xsteps.tonext();
				if( ysteps.num() == 1 )
					ysteps.reset();
				else
					ysteps.tonext();
			} else {
				level->hassteps = false;
			}
		}
	}
	
	void draw( image_t & image, layer_t & layer )
	{
		double xoff = layer.xmin;
		double xscale = image.x / ( layer.xmax - layer.xmin );
		double yoff = layer.ymax;
		double yscale = image.y / ( layer.ymax - layer.ymin );
		float idl = 1;
		for( uint ilvl = 0; ilvl < levels.num(); ilvl++ )
		{
			level_t const* level = levels.get( ilvl );
			if( !level->hassteps )
				break;
			level_t const* nextlevel = nullptr;
			if( ilvl + 1 < levels.num() )
				nextlevel = levels.get( ilvl + 1 );
			float u = level->u * g_uv;
			float v = level->v * g_uv;
			float l = level->l * g_l * idl;
			float s = level->s;
			float h = level->h;
			switch( level->plot )
			{
				case 0:
					for( lint ix = level->x.ilo; ix <= level->x.ihi; ix++ )
					{
						if( !level->x.innext( ix ) || !nextlevel ||
						    !nextlevel->hassteps || !nextlevel->x.num() )
						{
							double x = ( ( level->x.base * (double)ix ) - xoff ) * xscale;
							image.vline( x, u, l, s, h );
						}
					}
					for( lint iy = level->y.ilo; iy <= level->y.ihi; iy++ )
					{
						if( !level->y.innext( iy ) || !nextlevel ||
						    !nextlevel->hassteps || !nextlevel->y.num() )
						{
							double y = ( yoff - ( level->y.base * (double)iy ) ) * yscale;
							image.hline( y, v, l, s, h );
						}
					}
					break;
				default:
					break;
			}
			idl *= g_dl;
		}
	}
};

#endif // GRID_INCLUDED
#undef GRID_INCLUDING

