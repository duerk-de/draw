#ifdef GRAPH_INCLUDING
#error  GRAPH_INCLUDING
#endif //  GRAPH_INCLUDING
#define GRAPH_INCLUDING
#ifndef GRAPH_INCLUDED
#define GRAPH_INCLUDED

#include "ctrl.cpp"
#include "userval.cpp"
#include "layer.cpp"
#include "image.cpp"
#include "data.cpp"
#include "datacalc.cpp"
#include "drawcalc.cpp"

struct graph_t
{
	userval_t x;
	userval_t y;
	userval_t sx;
	userval_t sy;
	userval_t l;
	userval_t s;
	userval_t h;
	userval_t cond;
	uint plot;
	float linestep;
	
	calculator_t *c_x;
	calculator_t *c_y;
	calculator_t *c_u;
	calculator_t *c_v;
	calculator_t *c_l;
	calculator_t *c_s;
	calculator_t *c_h;
	calculator_t *c_cond;
	
	struct pt_t
	{
		float x;
		float y;
		float u;
		float v;
		float l;
		float s;
		float h;
	};
	
	stream<pt_t> pts;

	graph_t() :
		plot( 0 ),
		linestep( 1.f ),
		c_x( nullptr ),
		c_y( nullptr ),
		c_u( nullptr ),
		c_v( nullptr ),
		c_l( nullptr ),
		c_s( nullptr ),
		c_h( nullptr ),
		c_cond( nullptr )
	{ }
	
	graph_t( graph_t const& g ) :
		x( g.x, false ),
		y( g.y, false ),
		sx( g.sx, false ),
		sy( g.sy, false ),
		l( g.l, false ),
		s( g.s, false ),
		h( g.h, false ),
		cond( g.cond, false ),
		plot( g.plot ),
		linestep( g.linestep ),
		c_x( nullptr ),
		c_y( nullptr ),
		c_u( nullptr ),
		c_v( nullptr ),
		c_l( nullptr ),
		c_s( nullptr ),
		c_h( nullptr ),
		c_cond( nullptr )
	{ }
	
	~graph_t()
	{
		delete c_x;
		delete c_y;
		delete c_u;
		delete c_v;
		delete c_l;
		delete c_s;
		delete c_h;
		delete c_cond;
	}
		
	void defaults( drawcalc_t & calc )
	{
		calc.eval( "points = 0" );
		calc.eval( "line = 1" );
		calc.eval( "spline = 2" );
		calc.eval( "type = points" );
		calc.eval( "linestep = 1" );
	}
	
	void setup( drawcalc_t const& calc, data_t & data )
	{
		plot = calc.getintval( "type" );
		if( plot )
			linestep = calc.getval( "linestep" );
		
		delete c_x;
		delete c_y;
		delete c_u;
		delete c_v;
		delete c_l;
		delete c_s;
		delete c_h;
		datacalc_t dcalc( calc.vars );
		ENSURE( x.val, "datax not set\n" );
		ENSURE( y.val, "datay not set\n" );
		ENSURE( sx.val, "plotx not set\n" );
		ENSURE( sy.val, "ploty not set\n" );
		ENSURE( l.val, "lum not set\n" );
		ENSURE( s.val, "sat not set\n" );
		ENSURE( h.val, "hue not set\n" );
		c_x = dcalc.mk( x.val, x.len, data );
		c_y = dcalc.mk( y.val, y.len, data );
		c_u = dcalc.mk( sx.val, sx.len, data );
		c_v = dcalc.mk( sy.val, sy.len, data );
		c_l = dcalc.mk( l.val, l.len, data );
		c_s = dcalc.mk( s.val, s.len, data );
		c_h = dcalc.mk( h.val, h.len, data );
		if( cond.val )
			c_cond = dcalc.mk( cond.val, cond.len, data );
		pts.drop();
	}
	
	void print()
	{
		d_fprint( "x:%*s, ", x.len, x.val );
		d_fprint( "y:%*s, ", y.len, y.val );
		d_fprint( "sx:%*s, ", sx.len, sx.val );
		d_fprint( "sy:%*s, ", sy.len, sy.val );
		d_fprint( "l:%*s, ", l.len, l.val );
		d_fprint( "s:%*s, ", s.len, s.val );
		d_fprint( "h:%*s, ", h.len, h.val );
		d_fprint( "plot:%u\n", plot );
	}

	bool draw( image_t & image, layer_t & layer, bool data_available )
	{
		// d_fprint( "draw data_available:%b plot:%i pts.num():%i\n", data_available, plot, pts.num() );
		pt_t p;
		if( data_available )
		{
			p.x = ( c_x->calc() - layer.xoff ) * layer.xscale;
			p.y = ( layer.yoff - c_y->calc() ) * layer.yscale;
			p.u = c_u->calc() * layer.xscale;
			p.v = c_v->calc() * layer.yscale;
			p.l = c_l->calc();
			p.s = c_s->calc();
			p.h = c_h->calc();
			if( c_cond )
				if( d_abs( c_cond->calc() ) < 0.5f )
					return( false );
			while( pts.num() >= 4 )
				pts.del( pts.num() - 1 );
			pts.ins( p, 0 );
		} else {
			p.x = -1;
			if( !pts.num() )
				return( false );
			pts.del( pts.num() - 1 );
		}
		
		switch( plot )
		{
			case 0:
				if( !data_available )
					return( false );
				image.point( p.x, p.y, p.u, p.v, p.l, p.s, p.h );
				break;
			case 1:
				if( !data_available )
					return( false );
				if( pts.num() >= 2 )
				{
					pt_t const* pl = pts.get( 1 );
					// d_fprint( "line: %d x %d %d x %d\n", p.x, p.y, pl->x, pl->y );
					image.line( pl->x, pl->y, pl->u, pl->v,
					            pl->l * linestep, pl->s, pl->h,
					            p.x, p.y, p.u, p.v,
					            p.l * linestep, p.s, p.h, linestep );
				} else {
					// d_fprint( "point: %d x %d\n", p.x, p.y );
					image.point( p.x, p.y, p.u, p.v, p.l, p.s, p.h );
				}
				break;
			case 2:
				switch( pts.num() )
				{
					case 1:
						if( !data_available )
							d_fprint_err( "spline data underflow\n" );
						else
							image.point( p.x, p.y, p.u, p.v, p.l, p.s, p.h );
						break;
					case 2:
						if( !data_available )
							d_fprint_err( "spline data underflow\n" );
						break;
					case 3:
					{
						if( !data_available )
						{
							pt_t const* p00 = pts.get( 2 );
							pt_t const* p0 = pts.get( 1 );
							pt_t const* p1 = pts.get( 0 );
							float dy0 = ( p1->y - p00->y ) 
							          / ( p1->x - p00->x ) * ( p1->x - p0->x );
							float dy1 = ( p1->y - p0->y );
							image.spline( dy0, p0->x, p0->y, p0->u, p0->v,
							              p0->l * linestep, p0->s, p0->h,
										  dy1, p1->x, p1->y, p1->u, p1->v, 
										  p1->l * linestep, p1->s, p1->h,
										  linestep );
							return( false );
						} else {
							pt_t const* p0 = pts.get( 2 );
							pt_t const* p1 = pts.get( 1 );
							pt_t const* p11 = pts.get( 0 );
							float dy0 = ( p1->y - p0->y );
							float dy1 = ( p11->y - p0->y ) 
							          / ( p11->x - p0->x ) * ( p1->x - p0->x );
							image.spline( dy0, p0->x, p0->y, p0->u, p0->v, 
										  p0->l * linestep, p0->s, p0->h,
										  dy1, p1->x, p1->y, p1->u, p1->v, 
										  p1->l * linestep, p1->s, p1->h,
										  linestep );
						}
						break;
					}
					case 4:
					{
						// ENSURE( data_available, "points to plot out of range\n" );
						pt_t const* p00 = pts.get( 3 );
						pt_t const* p0 = pts.get( 2 );
						pt_t const* p1 = pts.get( 1 );
						pt_t const* p11 = pts.get( 0 );
						float dy0 = ( p1->y - p00->y ) 
						          / ( p1->x - p00->x ) * ( p1->x - p0->x );
						float dy1 = ( p11->y - p0->y ) 
						          / ( p11->x - p0->x ) * ( p1->x - p0->x );
						image.spline( dy0, p0->x, p0->y, p0->u, p0->v, 
									  p0->l * linestep, p0->s, p0->h,
									  dy1, p1->x, p1->y, p1->u, p1->v, 
									  p1->l * linestep, p1->s, p1->h,
									  linestep );
						break;
					}
					default:
						ENSURE( false, "points to plot (%i) out of range\n", pts.num() );
						break;
				}
				break;
			default:
				ENSURE( false, "plot out ouf range\n" );
				break;
		}
		return( p.x >= 0 && p.x < image.x );
	}
};

#endif // GRAPH_INCLUDED
#undef GRAPH_INCLUDING

