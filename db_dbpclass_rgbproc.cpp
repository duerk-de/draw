__GIT_SYMLINKS__
target=../_inc/db_dbpclass_rgbproc.cpp
target_resolved=/dibom/src/c/_inc/db_dbpclass_rgbproc.cpp
kind=file
sha256=22968f7a03e7f4906bd557b80b4e138dfd9ed7b34c48a06d365f9a904d0ecd39
size=10517
created=2026-11-15T11:20:55.362680
--END-HEADER--
#ifndef DB_DBPCLASS_RGBPROC_INCLUDED
#define DB_DBPCLASS_RGBPROC_INCLUDED


// hsb: hue - saturation - brightness
// hue (in degrees) from 0-6*255 (0:red, 510:green, 1020:blue, 255:yellow, 765:cyan, 1275:magenta
// saturation from 0 to 255, brightness from 0 to 512
// saturation 255 and brightness 255 will produce pure colors ( 0,255,255 = 255,0,0 ... )
// brightness 0 is black, brightness 510 is white, independant from saturation ( hue is always preserved exept for black and white, of course )
// brightness values above 255 may change saturation if suggested color is not representable, saturation is not always preserved
// equal brightness values may produce different color luminosity at different hue values, e.g. magenta is twice as lucid as red
// saturation 0 will equalize r-g-b-values (gray)
inline void hsb( int hue, int saturation, int brightness, int & r, int & g, int & b )
{
	const static int d1 = 255, d2 = 2*255, d3 = 3*255, d4 = 4*255, d5 = 5*255, d6 = 6*255;
	int mean;

	if( hue < 0 )
	{
		if( hue < -d6 )
			hue %= d6;
		hue += d6;
	} else {
		if( hue > d6 )
			hue %= d6;
	}
	if( hue < d1 )
	{
		r = d1;
		g = hue;
		b = 0;
	} else if( hue < d2 ) {
		r = d2-hue;
		g = d1;
		b = 0;
	} else if( hue < d3 ) {
		r = 0;
		g = d1;
		b = hue-d2;
	} else if( hue < d4 ) {
		r = 0;
		g = d4-hue;
		b = d1;
	} else if( hue < d5 ) {
		r = hue-d4;
		g = 0;
		b = d1;
	} else {
		r = d1;
		g = 0;
		b = d6-hue;
	}
	
	if( saturation != d1 )
	{
		mean = (r+g+b)/3;
		if( saturation < 0 ) saturation = 0;
		else if( saturation > d1 ) saturation = d1;
		r = saturation * ( r - mean ) / d1 + mean;
		g = saturation * ( g - mean ) / d1 + mean;
		b = saturation * ( b - mean ) / d1 + mean;
	}
	if( brightness != d1 )
	{
		if( brightness < d1 )
		{
			if( brightness < 0 )
				brightness = 0;
			r = r * brightness / d1;
			g = g * brightness / d1;
			b = b * brightness / d1;
		} else {
			if( brightness > d2 )
				brightness = d2;
			brightness = d2 - brightness;
			r = 0xff - ( 0xff - r ) * brightness / d1;
			g = 0xff - ( 0xff - g ) * brightness / d1;
			b = 0xff - ( 0xff - b ) * brightness / d1;
		}
	}
}

// hsb: hue - saturation - brightness
// hue (in degrees) from 0-360 (0:red, 120:green, 240:blue, 60:yellow, 180:cyan, 300:magenta
// saturation from 0 to 1, brightness from 0 to 2
// saturation 1 and brightness 1 will produce pure colors ( 0,1,1 = 255,0,0 ... )
// brightness 0 is black, brightness 2 is white, independant from saturation ( hue is always preserved exept for black and white, of course )
// brightness values above 1 may change saturation if suggested color is not representable, saturation is not always preserved
// equal brightness values may produce different color luminosity at different hue values, e.g. magenta is twice as lucid as red
// saturation 0 will equalize r-g-b-values (gray)
void hsb( double hue, double saturation, double brightness, int & ri, int & gi, int & bi )
{
	double r, g, b, mean, d;
	hue = hue - ((int)(hue/360.0))*360.0;
	if( hue<0 ) hue = -hue;
	d = hue;
	if( d > 180.0 )
		d = 360.0 - d;
	if( d > 60.0 )
		if( d < 120.0 )
			r = 2.0-d/60.0;
		else
			r = 0.0;
	else
		r = 1.0;
	d = hue - 120.0;
	if( d < 0.0 )
		d += 360.0;
	if( d > 180.0 )
		d = 360.0 - d;
	if( d > 60.0 )
		if( d < 120.0 )
			g = 2.0-d/60.0;
		else
			g = 0.0;
	else
		g = 1.0;
	d = hue - 240.0;
	if( d < 0.0 )
		d += 360.0;
	if( d > 180.0 )
		d = 360.0 - d;
	if( d > 60.0 )
		if( d < 120.0 )
			b = 2.0-d/60.0;
		else
			b = 0.0;
	else
		b = 1.0;
	mean = (r+g+b)/3.0;

	if( saturation < 0.0 ) saturation = 0.0;
	else if( saturation > 1.0 ) saturation = 1.0;
	r = saturation * ( r - mean ) + mean;
	g = saturation * ( g - mean ) + mean;
	b = saturation * ( b - mean ) + mean;

	if( brightness > 2.0 )
		brightness = 2.0;
    else if( brightness < 0.0 )
		brightness = 0.0;
	if( brightness > 1.0 )
		brightness = ( brightness - 1.0 ) / mean + 2.0 - brightness;
	r *= brightness;
	g *= brightness;
	b *= brightness;
	mean *= brightness;

	if( r > g )
		d = r;
	else
		d = g;
	if( b > d )
		d = b;
	if( d > 1.0 )
	{
		d = ( 1.0 - mean ) / ( d - mean );
		r = d * ( r - mean ) + mean;
		g = d * ( g - mean ) + mean;
		b = d * ( b - mean ) + mean;
	}
	ri = (int)(r*255);
	gi = (int)(g*255);
	bi = (int)(b*255);
}

inline void ihsb( int r, int g, int b, double & hue, double & saturation, double & brightness )
{
	int min, max;

	brightness = ( r + g + b ) / 3.0 / 255.0 * 2.0;

	if( brightness <= 0.0 )
	{
		hue = 0.0;
	} else {
		min = r;
		if( g < min )
			min = g;
		if( b < min )
			min = b;
		r -= min;
		g -= min;
		b -= min;

		max = r;
		if( g > max )
			max = g;
		if( b > max )
			max = b;
		r = (int)( (double)r*255.0/(double)max + 0.5 );
		g = (int)( (double)g*255.0/(double)max + 0.5 );
		b = (int)( (double)b*255.0/(double)max + 0.5 );

		saturation = (double)max / 255.0;

		if( b == 0 )
		{
			if( r == 255 )
				hue = ( 0.0 + (double)g/255.0 ) * 60.0;
			else
				hue = ( 2.0 - (double)r/255.0 ) * 60.0;
		} else if( r == 0 )
		{
			if( b == 255 )
				hue = ( 2.0 + (double)b/255.0 ) * 60.0;
			else
				hue = ( 4.0 - (double)g/255.0 ) * 60.0;
		} else {
			if( r == 255 )
				hue = ( 4.0 + (double)r/255.0 ) * 60.0;
			else
				hue = ( 6.0 - (double)b/255.0 ) * 60.0;
		}
	}
}


/*
inline void rgb_lsh( const double &r, const double &g, const double &b, double &l, double &s, double &h, const double &prec = 1e-6 )
{
	const static double sq2 = d_sqrt( 2 );
	const static double sq3 = d_sqrt( 3 );
	
	double rgb = r+g+b;
	double rrgb = 2.0*r-(g+b);
	double rrgbsq3 = rrgb*sq3;
	double gb = g-b;
	
	l = rgb/sq3;
	
	if( rrgb )
	{
		h = d_arctan( 3.0*gb/rrgbsq3, prec );
		if( rrgb < 0.0 )
			h += 180.0;
		if( h < 0.0 )
			h += 360.0;
	} else {
		h = 0.0;
	}
	
	double cosh = d_cos(h,prec);
	if( cosh )
		s = rrgbsq3*sq2/(6.0*cosh);
	else
		s = gb/(sq2*d_sin(h,prec));
}

inline void lsh_rgb( double l, double s, double h, double &r, double &g, double &b, const double &prec = 1e-6 )
{
	const static double sq2 = d_sqrt( 2 );
	const static double sq3 = d_sqrt( 3 );
	
	h = h - (double)(((int)(h/360.0))*360);
	if( h < 0 )
		h += 360.0;
	
	if( l > sq3 )
		l = sq3;
	if( l < -sq3 )
		l = -sq3;
	if( s < 0.0 )
		s = 0.0;
	
	double limit = 1.00;
	
	double cosh = d_cos( h, prec );
	double sinh = d_sin( h, prec );
	double lsq3 = l/sq3;
	
	double plsq3sq2 = (limit-lsq3)*sq2;
	double nlsq3sq2 = (-limit-lsq3)*sq2;
	double coshsq3 = cosh/sq3;
	double psinhcoshsq3 = sinh-coshsq3;
	double nsinhcoshsq3 = -sinh-coshsq3;
	double coshsq32 = 2.0*coshsq3;
	
	double srp = plsq3sq2/coshsq32;
	double srn = nlsq3sq2/coshsq32;
	double sgp = plsq3sq2/psinhcoshsq3;
	double sgn = nlsq3sq2/psinhcoshsq3;
	double sbp = plsq3sq2/nsinhcoshsq3;
	double sbn = nlsq3sq2/nsinhcoshsq3;
	
	if( srp >= 0.0 && srp < s )
		s = srp;
	if( srn >= 0.0 && srn < s )
		s = srn;
	if( sgp >= 0.0 && sgp < s )
		s = sgp;
	if( sgn >= 0.0 && sgn < s )
		s = sgn;
	if( sbp >= 0.0 && sbp < s )
		s = sbp;
	if( sbn >= 0.0 && sbn < s )
		s = sbn;
	
	double sq2s = sq2/s;
	
	r = lsq3 + 2.0*coshsq3/sq2s;
	g = lsq3 + psinhcoshsq3/sq2s;
	b = lsq3 + nsinhcoshsq3/sq2s;
	
}
*/


template <typename f_t>
inline void rgb_lsh( const f_t &r, const f_t &g, const f_t &b, f_t &l, f_t &s, f_t &h, const f_t &prec = 1e-6 )
{
	const static f_t sq2 = d_sqrt<f_t>( 2.0 );
	const static f_t sq3 = d_sqrt<f_t>( 3.0 );
	
	f_t rrgb = 2.0*r-(g+b);
	
	l = (r+g+b)/sq3;
	if( rrgb )
	{
		h = d_arctan( sq3*(g-b)/rrgb, prec );
		if( rrgb < 0.0 )
			h += 180.0;
		if( h < 0.0 )
			h += 360.0;
	} else {
		h = 0.0;
	}
	s = (rrgb*d_cos(h,prec)/sq3+(g+b)*d_sin(h,prec))/sq2;
	
	/*
	f_t cosh = d_cos(h,prec);
	if( cosh )
		s = (rrgb*sq3)*sq2/(6.0*cosh);
	else
		s = (g-b)/(sq2*d_sin(h,prec));
	*/
}

template <typename f_t>
inline void lsh_rgb( f_t l, f_t s, f_t h, f_t &r, f_t &g, f_t &b, const f_t &prec = 1e-6 )
{
	const static f_t sq2 = d_sqrt<f_t>( 2.0 );
	const static f_t sq3 = d_sqrt<f_t>( 3.0 );
	
	h = h - (f_t)(((int)(h/360.0))*360);
	if( h < 0 )
		h += 360.0;
	
	if( l > sq3 )
		l = sq3;
	if( l < -sq3 )
		l = -sq3;
	if( s < 0.0 )
		s = 0.0;
	
	f_t limit = 1.00;
	
	f_t cosh = d_cos( h, prec );
	f_t sinh = d_sin( h, prec );
	f_t lsq3 = l/sq3;
	
	f_t plsq3sq2 = (limit-lsq3)*sq2;
	f_t nlsq3sq2 = (-limit-lsq3)*sq2;
	f_t coshsq3 = cosh/sq3;
	f_t psinhcoshsq3 = sinh-coshsq3;
	f_t nsinhcoshsq3 = -sinh-coshsq3;
	f_t coshsq32 = 2.0*coshsq3;
	
	if( coshsq32 != 0.0 )
	{
		f_t srp = plsq3sq2/coshsq32;
		f_t srn = nlsq3sq2/coshsq32;
		if( srp >= 0.0 && srp < s )
			s = srp;
		if( srn >= 0.0 && srn < s )
			s = srn;
	}
	if( psinhcoshsq3 != 0.0 )
	{
		f_t sgp = plsq3sq2/psinhcoshsq3;
		f_t sgn = nlsq3sq2/psinhcoshsq3;
		if( sgp >= 0.0 && sgp < s )
			s = sgp;
		if( sgn >= 0.0 && sgn < s )
			s = sgn;
	}
	if( nsinhcoshsq3 != 0.0 )
	{
		f_t sbp = plsq3sq2/nsinhcoshsq3;
		f_t sbn = nlsq3sq2/nsinhcoshsq3;
		if( sbp >= 0.0 && sbp < s )
			s = sbp;
		if( sbn >= 0.0 && sbn < s )
			s = sbn;
	}
	
	if( s > 0.0 )
	{
		f_t sq2s = sq2/s;
		r = lsq3 + 2.0*coshsq3/sq2s;
		g = lsq3 + psinhcoshsq3/sq2s;
		b = lsq3 + nsinhcoshsq3/sq2s;
	} else {
		r = lsq3;
		g = lsq3;
		b = lsq3;
	}
}

inline void rgbd2i( const double &r, const double &g, const double &b, int &ir, int &ig, int &ib, const double &gamma = 2.2, const double &prec = 1e-6 )
{
	// const static double a = 255.0/2.0;
	
	ir = (int)(d_hoch((r+1.0)/2.0,1.0/gamma,prec)*255.0);
	ig = (int)(d_hoch((g+1.0)/2.0,1.0/gamma,prec)*255.0);
	ib = (int)(d_hoch((b+1.0)/2.0,1.0/gamma,prec)*255.0);
	
	// ir = (int)((d_hoch(r,igamma,prec)+1.0)*a);
	// ig = (int)((d_hoch(g,igamma,prec)+1.0)*a);
	// ib = (int)((d_hoch(b,igamma,prec)+1.0)*a);
}

inline void rgbi2d( const int &ir, const int &ig, const int &ib, double &r, double &g, double &b, const double &gamma = 2.2, const double &prec = 1e-6 )
{
	// const static double a = 2.0/255.0;
	
	r = d_hoch(((double)ir)/255.0,gamma,prec)*2.0-1.0;
	g = d_hoch(((double)ig)/255.0,gamma,prec)*2.0-1.0;
	b = d_hoch(((double)ib)/255.0,gamma,prec)*2.0-1.0;
	
	// r = d_hoch(ir*a-1.0,gamma,prec);
	// g = d_hoch(ig*a-1.0,gamma,prec);
	// b = d_hoch(ib*a-1.0,gamma,prec);
}


#endif // DB_DBPCLASS_RGBPROC_INCLUDED
