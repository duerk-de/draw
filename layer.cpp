#ifdef LAYER_INCLUDING
#error  LAYER_INCLUDING
#endif //  LAYER_INCLUDING
#define LAYER_INCLUDING
#ifndef LAYER_INCLUDED
#define LAYER_INCLUDED

#include "ctrl.cpp"
#include "userval.cpp"
#include "drawcalc.cpp"
#include "image.cpp"

struct layer_t
{
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	
	float xoff;
	float xscale;
	float yoff;
	float yscale;
	
	void defaults( drawcalc_t & calc, bool force )
	{
		if( ( !calc.getvar( "xpxls" ) || force )
		  && calc.getvar( "imgx" ) 
		  && calc.getvar( "xmax" ) && calc.getvar( "xmin" ) )
			calc.eval( "xpxls = [xmax - xmin] / imgx" );
		if( ( !calc.getvar( "ypxls" ) || force )
		  && calc.getvar( "imgy" )
		  && calc.getvar( "ymax" ) && calc.getvar( "ymin" ) )
			calc.eval( "ypxls = [ymax - ymin] / imgy" );
	}
	
	void border( drawcalc_t & calc, bool force )
	{
		if( ( !calc.getvar( "borderset" ) || !calc.getintval( "borderset" ) || force )
		  && calc.getvar( "border" )
		  && calc.getvar( "xmax" ) && calc.getvar( "xmin" )
		  && calc.getvar( "ymax" ) && calc.getvar( "ymin" )
		  && calc.getvar( "xpxls" ) && calc.getvar( "ypxls" ) )
		{
			calc.eval( "xmin = xmin - border * xpxls" );
			calc.eval( "xmax = xmax + border * xpxls" );
			calc.eval( "ymin = ymin - border * ypxls" );
			calc.eval( "ymax = ymax + border * ypxls" );
			calc.eval( "borderset = true" );
		}
	}
	
	void setup( image_t const& image, drawcalc_t const& calc )
	{
		xmin = calc.getval( "xmin" );
		xmax = calc.getval( "xmax" );
		ymin = calc.getval( "ymin" );
		ymax = calc.getval( "ymax" );
		
		xoff = xmin;
		xscale = image.x / ( xmax - xmin );
		yoff = ymax;
		yscale = image.y / ( ymax - ymin );
	}
};

#endif // LAYER_INCLUDED
#undef LAYER_INCLUDING

