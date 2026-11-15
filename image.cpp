#ifdef IMAGE_INCLUDING
#error  IMAGE_INCLUDING
#endif //  IMAGE_INCLUDING
#define IMAGE_INCLUDING
#ifndef IMAGE_INCLUDED
#define IMAGE_INCLUDED

#include "ctrl.cpp"
#include "math.cpp"
#include "drawcalc.cpp"
#include "userval.cpp"
#include "db_lsh.cpp"
#include "db_dbpclass.cpp"

struct image_t : public not_copyable_t
{
	uint x;
	uint y;
	userval_t filename;
	static constexpr uint milli_gamma = 2200;
	lsh_conv<float, milli_gamma> lsh;
	
	dbprgb48class *dbp;
	
	image_t() :
		x( 0 ),
		y( 0 ),
		lsh( "/tmp/draw-lsh-cnv.tmp" ),
		dbp( nullptr )
	{ }
	
	~image_t()
	{
		if( dbp )
			delete dbp;
	}
	
	void write()
	{
		ENSURE( dbp, "image not drawn\n" );
		dbpbmp24class dbpbmp( dbp->size, true );
		for( int y = 0; y < dbp->size.y; y++ )
		{
			for( int x = 0; x < dbp->size.x; x++ )
			{
				int r, g, b;
				dbp->get( x, y, r, g, b );
				lsh.gp()->n16tog8( r, g, b );
				dbpbmp.set( x, y, r, g, b );
			}
		}
		dbpbmp.makefile();
		ENSURE( filename.val, "image filename not set\n" );
		fileclass file;
		file.set( filename.val, 'w', filename.len );
		ENSURE( file.open(), "could not open file %s\n", filename.val );
		ENSURE( file.write( dbpbmp.img, dbpbmp.bytesize ), "could not write %yb to file %s\n", (double)dbpbmp.bytesize, filename.val );
		file.close();
		delete dbp;
		dbp = nullptr;
		d_fprint( "image (%ix%i) written to %s (%yb)\n",
		          dbpbmp.size.x, dbpbmp.size.y,
		          filename.val, (double)dbpbmp.bytesize );
		filename.isset = false;
	}
	
	void defaults( drawcalc_t & calc )
	{
		calc.eval( "imgx = 1024" );
		calc.eval( "imgy = 1024" );
	}
	
	void setup( drawcalc_t const& calc )
	{
		if( !dbp )
		{
			x = calc.getval( "imgx" );
			y = calc.getval( "imgy" );
			dbp = new dbprgb48class( pt_t( x, y ) );
			zro( dbp->img, dbp->bytesize );
			ENSURE( dbp, "oom\n" );
			ENSURE( dbp->img, "oom\n" );
		}
	}
	
	struct add16_t
	{
		int r;
		int g;
		int b;
		static inline void proc( int& r, int& g, int& b, void* rgb )
		{
			r += ((add16_t*)rgb)->r;
			g += ((add16_t*)rgb)->g;
			b += ((add16_t*)rgb)->b;
			gammapaintclass<milli_gamma>::tomax( r, g, b );
		}
	};

	void point( float x, float y, float u, float v,
	           float l, float s, float h )
	{
		add16_t rgb;
		lsh.lshton16( l, s, h, rgb.r, rgb.g, rgb.b );
		dbp->pxlb14( x, y, u, v, rgb.proc, (void*)&rgb );
	}
	
	void hline( float y, float v,
	            float l, float s, float h )
	{
		add16_t rgb;
		lsh.lshton16( l, s, h, rgb.r, rgb.g, rgb.b );
		dbp->hlineb14( y, v, 0, dbp->size.x, rgb.proc, (void*)&rgb );
	}
	
	void vline( float x, float u,
	            float l, float s, float h )
	{
		add16_t rgb;
		lsh.lshton16( l, s, h, rgb.r, rgb.g, rgb.b );
		dbp->vlineb14( x, u, 0, dbp->size.y, rgb.proc, (void*)&rgb );
	}
	
	void line( float x0, float y0, float u0, float v0,
	           float l0, float s0, float h0,
	           float x1, float y1, float u1, float v1,
	           float l1, float s1, float h1,
	           float step )
	{
		float dx = x1 - x0;
		float dy = y1 - y0;
		float du = u1 - u0;
		float dv = v1 - v0;
		float dl = l1 - l0;
		float ds = s1 - s0;
		float dh = h1 - h0;
		
		if( ( !dx && !dy ) || !u0 || !v0 || !u1 || !v1 )
			return;
		
		float d = math::sqrt_babylon( dx, dy );
		
		float id = 0;
		uint i = 0;
		while( id < d )
		{
			float p = id / d;
			float u = u0 + du * p;
			float v = v0 + dv * p;
			float dd = d / math::sqrt_babylon( dx / u, dy / v ) * step;
			float id0 = id;
			id += dd;
			if( id == id0 )
				break;
			i++;
		}
		uint n = i + 1;
		float dscale = d / id * step;
		id = 0;
		for( i = 0; i < n; i++ )
		{
			float p = id / d;
			float x = x0 + dx * p;
			float y = y0 + dy * p;
			float u = u0 + du * p;
			float v = v0 + dv * p;
			float l = l0 + dl * p;
			float s = s0 + ds * p;
			float h = h0 + dh * p;
			if( i )
				point( x, y, u, v, l, s, h );
			float dd = d / math::sqrt_babylon( dx / u, dy / v ) * dscale;
			id += dd;
		}
	}
	
	inline float spline_y( float y0, float dy0, float y1, float dy1, float t )
	{
		float sqt = d_sq( t );
		float t_1 = t - 1.f;
		float g1 = sqt * ( 3.f - 2.f * t );
		float g0 = 1 - g1;
		float d0 = t * d_sq( t_1 );
		float d1 = sqt * t_1;
		return( y0 * g0 + dy0 * d0 + y1 * g1 + dy1 * d1 );
	}
	
	inline float spline_dy( float y0, float dy0, float y1, float dy1, float t )
	{
		float t_1 = t - 1.f;
		float g0 = 6.f * t * t_1;
		float g1 = - g0;
		float d0 = t_1 * ( 3.f * t - 1.f );
		float d1 = t * ( 3.f * t - 2.f );
		return( y0 * g0 + dy0 * d0 + y1 * g1 + dy1 * d1 );
	}

	inline float spline_iy( float y0, float dy0, float y1, float dy1 )
	{
		float g0 = 0.5f;
		float g1 = g0;
		float d0 = 1.f / 12.f;
		float d1 = -d0;
		return( d_abs( y0 - y1 ) * g0 + d_abs( y0 - y1 ) * g1
		      + d_abs( dy0 * d0 ) * 1 + d_abs( dy1 * d1 ) * 1 );
	}
	
	void spline( float dy0, float x0, float y0, float u0, float v0,
	             float l0, float s0, float h0,
	             float dy1, float x1, float y1, float u1, float v1,
	             float l1, float s1, float h1,
	             float step )
	{
		float dx = x1 - x0;
		float du = u1 - u0;
		float dv = v1 - v0;
		float dl = l1 - l0;
		float ds = s1 - s0;
		float dh = h1 - h0;
		
		if( !dx || !u0 || !v0 || !u1 || !v1 )
			return;

		uint i = 0;
		float t = 0;
		while( t < 1.f )
		{
			float u = u0 + du * t;
			float v = v0 + dv * t;
			float sdy = spline_dy( y0, dy0, y1, dy1, t );
			float t0 = t;
			t += 1.f / math::sqrt_babylon( dx / u, sdy / v ) * step;
			if( t == t0 )
				break;
			i++;
		}
		uint n = i + 1;
		float dscale = 1.f / t * step;
		t = 0;
		for( i = 0; i < n; i++ )
		{
			float y = spline_y( y0, dy0, y1, dy1, t );
			float x = x0 + dx * t;
			float u = u0 + du * t;
			float v = v0 + dv * t;
			float l = l0 + dl * t;
			float s = s0 + ds * t;
			float h = h0 + dh * t;
			float sdy = spline_dy( y0, dy0, y1, dy1, t );
			if( i )
				point( x, y, u, v, l, s, h );
			t += 1.f / math::sqrt_babylon( dx / u, sdy / v ) * dscale;
		}
	}
	
};

#endif // IMAGE_INCLUDED
#undef IMAGE_INCLUDING

