__GIT_SYMLINKS__
target=../_inc/db_lsh.cpp
target_resolved=/dibom/src/c/_inc/db_lsh.cpp
kind=file
sha256=40a7c86de37eebfdb5ace4f85473aa2cb8f16dde8cb8704297bed524ca2084e9
size=4196
created=2026-11-15T11:20:55.375838
--END-HEADER--
#ifndef LSH_INCLUDED
#define LSH_INCLUDED

#include "db_defines.cpp"
#include "db_gammapaint.cpp"
#include "db_filebuffered.cpp"


template <typename f_t = double, uint milli_gamma = 2200>
class lsh_conv
{
protected:
	lsh_conv() = delete;
	lsh_conv( lsh_conv const& ) = delete;
	lsh_conv &operator =( lsh_conv const& ) = delete;

	gammapaintclass<milli_gamma>* gammapaint;
	static constexpr f_t n16max1 = 1u << 16;
	static constexpr f_t n16max = ( 1u << 16 ) - 1u;
public:
	static const f_t sq2;
	static const f_t sq3;
	lsh_conv( char const* filebuffer )
	{
		gammapaint = filebuffered<gammapaintclass<milli_gamma>>::load( filebuffer );
	}
	~lsh_conv()
	{
		dropp( gammapaint );
	}
	
	inline gammapaintclass<milli_gamma> const* gp() const
	{
		return( gammapaint );
	}

	static inline void rgb_lsh( f_t const& r, f_t const& g, f_t const& b,
	                            f_t& l, f_t& s, f_t& h, f_t const& prec = 1e-6 )
	{
		f_t rrgb = (f_t)2.0*r-(g+b);
		
		l = (r+g+b)/sq3;
		
		if( rrgb )
		{
			h = d_arctan( sq3*(g-b)/rrgb, prec );
		} else {
			if( g < b )
				h = (f_t)-90.0;
			else
				h = (f_t)90.0;
		}
		if( rrgb < (f_t)0.0 )
			h += (f_t)180.0;
		if( h < (f_t)0.0 )
			h += (f_t)360.0;

		f_t cosh = d_cos<f_t>(h,prec);
		f_t sinh = d_sin<f_t>(h,prec);
		s = (rrgb*cosh/sq3+(g+b)*sinh)/sq2;
		if( cosh )
			s = (rrgb*sq3)*sq2/((f_t)6.0*cosh);
		else
			s = (g-b)/(sq2*sinh);
	}

	static inline void lsh_rgb( f_t l, f_t s, f_t h,
	                            f_t& r, f_t& g, f_t& b, f_t const& prec = 1e-6 )
	{
		l = l * 2.0 - sq3;
		s *= 2.0;
		h = h - (f_t)(((int)(h/(f_t)360.0))*360);
		if( h < 0 )
			h += (f_t)360.0;
		
		if( l > sq3 )
			l = sq3;
		if( l < -sq3 )
			l = -sq3;
		if( s < (f_t)0.0 )
			s = (f_t)0.0;
		
		f_t limit = (f_t)1.00;
		
		f_t cosh = d_cos<f_t>( h, prec );
		f_t sinh = d_sin<f_t>( h, prec );
		f_t lsq3 = l/sq3;
		
		f_t plsq3sq2 = (+limit-lsq3)*sq2;
		f_t nlsq3sq2 = (-limit-lsq3)*sq2;
		f_t coshsq3 = cosh/sq3;
		f_t psinhcoshsq3 = +sinh-coshsq3;
		f_t nsinhcoshsq3 = -sinh-coshsq3;
		f_t coshsq32 = (f_t)2.0*coshsq3;
		
		if( coshsq32 != (f_t)0.0 )
		{
			f_t srp = plsq3sq2/coshsq32;
			f_t srn = nlsq3sq2/coshsq32;
			if( srp >= (f_t)0.0 && srp < s )
				s = srp;
			if( srn >= (f_t)0.0 && srn < s )
				s = srn;
		}
		if( psinhcoshsq3 != (f_t)0.0 )
		{
			f_t sgp = plsq3sq2/psinhcoshsq3;
			f_t sgn = nlsq3sq2/psinhcoshsq3;
			if( sgp >= (f_t)0.0 && sgp < s )
				s = sgp;
			if( sgn >= (f_t)0.0 && sgn < s )
				s = sgn;
		}
		if( nsinhcoshsq3 != (f_t)0.0 )
		{
			f_t sbp = plsq3sq2/nsinhcoshsq3;
			f_t sbn = nlsq3sq2/nsinhcoshsq3;
			if( sbp >= (f_t)0.0 && sbp < s )
				s = sbp;
			if( sbn >= (f_t)0.0 && sbn < s )
				s = sbn;
		}
		
		if( s > (f_t)0.0 )
		{
			f_t sq2s = sq2/s;
			r = lsq3 + (f_t)2.0*coshsq3/sq2s;
			g = lsq3 + psinhcoshsq3/sq2s;
			b = lsq3 + nsinhcoshsq3/sq2s;
		} else {
			r = lsq3;
			g = lsq3;
			b = lsq3;
		}
		
		r = ( r + (f_t)1.0 ) * (f_t)0.5;
		g = ( g + (f_t)1.0 ) * (f_t)0.5;
		b = ( b + (f_t)1.0 ) * (f_t)0.5;
	}
	
	static inline void n16tolsh( int const& n16r, int const& n16g, int const& n16b,
	                             f_t& l, f_t& s, f_t& h )
	{
		f_t dr = (f_t)n16r / n16max;
		f_t dg = (f_t)n16g / n16max;
		f_t db = (f_t)n16b / n16max;
		rgb_lsh( dr, dg, db, l, s, h );
	}
	
	static inline void lshton16( f_t const& l, f_t const& s, f_t const& h,
	                             int& n16r, int& n16g, int& n16b )
	{
		f_t dr, dg, db;
		lsh_rgb( l, s, h, dr, dg, db );
		n16r = (int)( dr * n16max1 );
		n16g = (int)( dg * n16max1 );
		n16b = (int)( db * n16max1 );
	}
	
	inline void from_g8( int r, int g, int b,
	                     f_t& l, f_t& s, f_t& h ) const
	{
		gammapaint->g8ton16( r, g, b );
		n16tolsh( r, g, b, l, s, h );
	}
	
	inline void to_g8( f_t const& l, f_t const& s, f_t const& h,
	                   int& r, int& g, int& b ) const
	{
		lshton16( l, s, h, r, g, b );
		gammapaint->tominmax( r, g, b );
		gammapaint->n16tog8( r, g, b );
	}
};

template <typename f_t, uint milligamma>
const f_t lsh_conv<f_t,milligamma>::sq2 = d_sqrt<f_t>( 2.0 );

template <typename f_t, uint milligamma>
const f_t lsh_conv<f_t,milligamma>::sq3 = d_sqrt<f_t>( 3.0 );


#endif // LSH_INCLUDED

