__GIT_SYMLINKS__
target=../_inc/db_gammapaint.cpp
target_resolved=/dibom/src/c/_inc/db_gammapaint.cpp
kind=file
sha256=2c941d6a431dffac0e6e4ca24e438c82d9d8f60b795fe2cd522df0d18571123c
size=2495
created=2026-11-15T11:20:55.373923
--END-HEADER--
#ifndef GAMMAPAINT_INCLUDED
#define GAMMAPAINT_INCLUDED

#include "db_defines.cpp"
#include "db_math.cpp"
#include "db_dbpclass_rgbproc.cpp"


template <uint milli_gamma>
class gammapaintclass 
{
public:
	static constexpr uint bits = 16u;
	static constexpr uint num = 1u << bits;
	static constexpr int max = num - 1u;
	
	static constexpr uint gbits = 8u;
	static constexpr uint gnum = 1u << gbits;
	static constexpr int gmax = gnum - 1u;
	
protected:
	double gamma, gamma_1;
	int t_n16_g8[num];
	int t_g8_n16[num];
	
	gammapaintclass( const gammapaintclass &_gammapaint ) = delete;
	gammapaintclass operator =( const gammapaintclass &_gammapaint ) = delete;
	
public:
	gammapaintclass()
	{
		gamma = (double)milli_gamma / 1000.0;
		gamma_1 = 1.0 / gamma;
		for( uint i=0; i<num; i++ )
			t_n16_g8[i] = (int)( d_hoch( ( (double)i / (double)max ), gamma_1, 1e-6 ) * (double)gmax + 0.5 );
		for( uint i=0; i<gnum; i++ )
			t_g8_n16[i] = (int)( d_hoch( ( (double)i / (double)gmax ), gamma, 1e-6 ) * (double)max + 0.5 );
	}
	/*
	static inline constexpr uint mapsize()
	{
		return( sizeof( *t_n16_g8 ) * num );
	}
	inline int *map()
	{
		return( t_n16_g8 );
	}
	*/
	inline void g8ton16( int& r, int& g, int& b ) const
	{
		r = t_g8_n16[r];
		g = t_g8_n16[g];
		b = t_g8_n16[b];
	}
	inline void n16tog8( int& r, int& g, int& b ) const
	{
		r = t_n16_g8[r];
		g = t_n16_g8[g];
		b = t_n16_g8[b];
	}
	static inline void tominmax( int& n16r, int& n16g, int& n16b )
	{
		n16r = d_min( max, d_max( 0, n16r ) );
		n16g = d_min( max, d_max( 0, n16g ) );
		n16b = d_min( max, d_max( 0, n16b ) );
	}
	static inline void tomax( int& n16r, int& n16g, int& n16b )
	{
		n16r = d_min( max, n16r );
		n16g = d_min( max, n16g );
		n16b = d_min( max, n16b );
	}
	// deprecated
	static inline void n16tolsh( int const& n16r, int const& n16g, int const& n16b,
	                             double& l, double& s, double& h )
	{
		double dr = (double)n16r / (double)max * 2.0 - 1.0;
		double dg = (double)n16g / (double)max * 2.0 - 1.0;
		double db = (double)n16b / (double)max * 2.0 - 1.0;
		rgb_lsh( dr, dg, db, l, s, h );
	}
	// deprecated
	static inline void lshton16( double const& l, double const& s, double const& h,
	                             int& n16r, int& n16g, int& n16b )
	{
		double dr, dg, db;
		lsh_rgb( l, s, h, dr, dg, db );
		n16r = (int)( (dr+1.0)*0.5 * (double)max );
		n16g = (int)( (dg+1.0)*0.5 * (double)max );
		n16b = (int)( (db+1.0)*0.5 * (double)max );
	}
};


#endif // GAMMAPAINT_INCLUDED

