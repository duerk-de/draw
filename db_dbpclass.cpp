__GIT_SYMLINKS__
target=../_inc/db_dbpclass.cpp
target_resolved=/dibom/src/c/_inc/db_dbpclass.cpp
kind=file
sha256=4bb422a1e68952a2dd62ea91ceb0d6ed4145c30cc5973156346179017907c7e5
size=42182
created=2026-11-15T11:20:55.351028
--END-HEADER--
#ifndef DB_DBPCLASS_INCLUDED
#define DB_DBPCLASS_INCLUDED

#include "db_defines.cpp"
#include "db_pointclass.cpp"
#include "db_rectclass.cpp"
#include "db_math.cpp"

#include "db_dbpclass_rgbproc.cpp"


/// image storing and processing class
/** color values from \"get\"-methods will be returned in r, g, b and n. r is also used for position in palette and monochrome value, n is normalization
	@see dbpnormalizeclass \n
	paint methods require an image format where pos( size.x+1 ) == img+bytesperpixel (position im memory ascends with ascending x)
*/
class dbpclass
{
	public:
		int p;					///< actual position in img
		int bi;					///< actual position of pixel in img[p]
		int pp;					///< actual position in palette
		int _r;					///< temporary
		pointclass size;		///< size of image
		int bytesperpixel;		///< bytes used per pixel, if can be expressed as integer
		int bitsperpixel;		///< bits used per pixel
		int pixelsperbyte;		///< pixels stores per byte, if can be expressed as integer
		int lgbitsperpixel;		///< log2 of bitsperpixel, if can be expressed as integer
		int lgpixelsperbyte;	///< log2 of pixelsperbyte, if can be expressed as integer
		int pixelsperbyte_1;	///< pixelsperbyte - 1
		int channels;			///< number of channels, 1 means monochome, 3 means rgb color, 2 means normalized monochrome, 4 means normalized rgb
		int mask;				///< maximum color value that can be stored
		int numcolors;			///< number of colors, available in palette in paletted
		int maxcolor;			///< maximum value of color
		bool paletted;			///< palette used
		int bytelinesize;		///< bytes per line (bitsperpixel*size.x), if can be expressed as integer 
		int palpos;				///< position of palette in img
		int datapos;			///< position of data in img
		int bytesize;			///< total size of img
		byte *img;				///< data contained here
		int *iparray;			///< array of size ipz*ipz, used for normalization of resampling
		int ipz;				///< size of iparray
		bool heaplocked;

		virtual ~dbpclass()														///< destructor, releases heap
		{
			if( !heaplocked )
			{
				delete [] iparray;
				delete [] img;
			}
		}
		virtual void create( __attribute__((__unused__)) pointclass _size )	{ _create(); }					///< creates new image with new _size (and bytesize and bytelinesize), other properties will be maintained
		inline void lockedcopy( dbpclass *dbp )
		{
			cpy( this, dbp, sizeof(*dbp) );
			heaplocked = true;
		}
	protected:
		inline dbpclass& operator= ( dbpclass const& dbp ) = delete;
		inline dbpclass( dbpclass const& dbp ) = delete;
		inline dbpclass()														///< default constructor, initializes stack
		{
			p = 0; bi = 0;
			size = 0;
			bytesperpixel = 0; bitsperpixel = 0; pixelsperbyte = 0; lgbitsperpixel = 0; lgpixelsperbyte = 0; pixelsperbyte_1 = 0;
			channels = 0; mask = 0;
			numcolors = 0; maxcolor = 0; paletted = false;
			bytelinesize = 0;
			palpos = 0; datapos = 0; bytesize = 0;
			img = null;
			iparray = null; ipz = 0;
			heaplocked = false;
		}
		/*
	public:
		dbpclass( pointclass _size, int _bitsperpixel = 24, int _channels = 3, bool _paletted = false, int _palpos = 0, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true )
		{
			iparray = null;
			ipz = 0;
			bits = 0;
			bytes = 0;
			p = 0;
			img = null;
			_create( _size, _bitsperpixel, _channels, _paletted, _palpos, _datapos, _bytelinesize, createbuffer );
		}
		*/
		// virtual void create( pointclass _size ) = 0;					///< creates new image with new _size (and bytesize and bytelinesize), other properties will be maintained
	protected:
		/// creates image, sets all stack data, releases old img
		inline void _create( pointclass _size = 0, int _bitsperpixel = 24, int _channels = 3, bool _paletted = false, int _palpos = 0, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true, int _bytesize = 0 )
		{
			int i, j;
			
			if( img && !heaplocked )
			{
				delete [] img;
				img = null;
			}
			p = 0; bi = 0;
			palpos = 0; datapos = 0; bytesize = 0;
			size = _size;
			if( size.x < 0 )
				size.x = 0;
			if( size.y < 0 )
				size.y = 0;
			
			channels = _channels;
			bitsperpixel = _bitsperpixel;
			paletted = _paletted;
			
			if( _channels < 1 || _channels > 4
				|| !( bitsperpixel == 1 || bitsperpixel == 2 || bitsperpixel == 4 || bitsperpixel == 8 || bitsperpixel%8 == 0 )
				|| ( bitsperpixel%channels != 0 && !paletted )
				|| ( paletted && bitsperpixel > 8 ) )
			{
				bytesperpixel = 0; pixelsperbyte = 0; lgbitsperpixel = 0; lgpixelsperbyte = 0; pixelsperbyte_1 = 0;
				mask = 0;
				numcolors = 0; maxcolor = 0;
				bytelinesize = 0;
			} else {
				if( bitsperpixel % 8 == 0 )
					bytesperpixel = bitsperpixel / 8;
				else
					bytesperpixel = 0;
				if( bitsperpixel <= 8 && 8 % bitsperpixel == 0 )
					pixelsperbyte = 8 / bitsperpixel;
				else
					pixelsperbyte = 0;
				pixelsperbyte_1 = pixelsperbyte - 1;
				
				for( lgbitsperpixel=0, i=bitsperpixel; i>1; lgbitsperpixel++, i>>=1 )
					if( i & 1 ) { lgbitsperpixel = 0; break; }
				for( lgpixelsperbyte=0, i=pixelsperbyte; i>1; lgpixelsperbyte++, i>>=1 )
					if( i & 1 ) { lgpixelsperbyte = 0; break; }
				
				j = bitsperpixel / channels;
				maxcolor =  ( 1 << j ) - 1;
				mask = 0;
				if( paletted || channels == 1 )
					for( i=0; i<bitsperpixel; mask = mask | (1<<i), i++ );
				else
					for( i=0; i<j; mask = mask | (1<<i), i++ );
				
				if( _bytelinesize == 0 )
					if( bytesperpixel > 0 )
						bytelinesize = bytesperpixel * size.x;
					else
						bytelinesize = 0; // bytelinesize = ( bitsperpixel * size.x + 7 ) / 8;
				else
					bytelinesize = _bytelinesize;
				
				palpos = _palpos;
				if( paletted )
				{
					if( _datapos == 0 )
					{
						numcolors = 1 << bitsperpixel;
						datapos = numcolors * 3 + palpos;
					} else {
						datapos = _datapos;
						numcolors = ( datapos - palpos ) / 3;
					}
				} else {
					datapos = _datapos;
				}
				if( _bytesize == 0 )
				{
					if( bytelinesize > 0 )
						bytesize = datapos + size.y * bytelinesize;
					else
						if( pixelsperbyte > 0 )
							bytesize = datapos + ( size.area() - 1 ) / pixelsperbyte + 1;
						else
							bytesize = datapos + ( size.area() * bitsperpixel + 7 ) / 8;
				} else {
					bytesize = _bytesize;
				}
				
				if( !heaplocked && createbuffer && bytesize > 0 )
					img = new byte[bytesize];
			}
		}
	public:
		virtual inline void getcol( __attribute__((__unused__)) int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b ) {}													///< gets color #r from palette
		virtual inline void setcol( __attribute__((__unused__)) const int &index, __attribute__((__unused__)) const int &r, __attribute__((__unused__)) const int &g, const __attribute__((__unused__)) int &b ) {}				///< sets color in palette
		virtual inline void setcol( __attribute__((__unused__)) const int &r, __attribute__((__unused__)) const int &g, const int __attribute__((__unused__)) &b ) {}								///< sets color in palette at previous position
		virtual inline int & poscol( const int &index )															///< gets position of color
		{	return( p = palpos + index + index + index );	}
		virtual inline void set24( const int __attribute__((__unused__)) &x, const int __attribute__((__unused__)) &y, const int __attribute__((__unused__)) &r, const int __attribute__((__unused__)) &g, const int __attribute__((__unused__)) &b ) {}	///< sets color in img, input color format is 24bit rgb
		virtual inline void set24( const int __attribute__((__unused__)) &r, const int __attribute__((__unused__)) &g, const int __attribute__((__unused__)) &b ) {}								///< sets color in img at previous position, input color format is 24bit rgb
		virtual inline void get24( __attribute__((__unused__)) const int &x, __attribute__((__unused__)) const int &y, __attribute__((__unused__)) int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )							///< gets color from img, output color format is 24bit rgb
		{	r = 0; g = 0; b = 0;	}
		virtual inline void get24( int &r, int &g, int &b )														///< gets color from img at previous position, output color format is 24bit rgb
		{	r = 0; g = 0; b = 0;	}
		virtual inline void set( __attribute__((__unused__)) const int &x, __attribute__((__unused__)) const int &y, __attribute__((__unused__)) const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b ) {}		///< sets color in img, color format is color format of class
		virtual inline void set( __attribute__((__unused__)) const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b ) {}									///< sets color in img at previous position, color format is color format of class
		virtual inline void get( __attribute__((__unused__)) const int &x, __attribute__((__unused__)) const int &y, int &r, int &g, int &b )							///< gets color from img, color format is color format of class
		{	r = 0; g = 0; b = 0;	}
		virtual inline void get( int &r, int &g, int &b )														///< gets color from img at previous position, color format is color format of class
		{	r = 0; g = 0; b = 0;	}
		virtual inline int & pos( const int &x, const int &y )													///< gets position of pixel
		{	return( p = datapos + y * bytelinesize + x * bytesperpixel );	}
		virtual inline void inrange24( int &r, int &g, int &b )													///< sets color to valid value in 24bit rgb space
		{	if( r > 0xff ) r = 0xff;
			else if( r < 0 ) r = 0;
			if( g > 0xff ) g = 0xff;
			else if( g < 0 ) g = 0;
			if( b > 0xff ) b = 0xff;
			else if( b < 0 ) b = 0;	}
		virtual inline void inrange( int &r, int &g, int &b )													///< sets color to valid value in color format of class space
		{	if( r > maxcolor ) r = maxcolor;
			else if( r < 0 ) r = 0;
			if( g > maxcolor ) g = maxcolor;
			else if( g < 0 ) g = 0;
			if( b > maxcolor ) b = maxcolor;
			else if( b < 0 ) b = 0;	}
		
		
	#include "db_dbpclass_paintbasics.cpp"	///< basic paint operations, like clearing image, converting between image types and b10/b14 algorithm
	#include "db_dbpclass_paint.cpp"		///< paint operations, like copying/moving areas, painting pixels and lines

};

/** pixel layout: r, g, b */
class dbprgb24class : public dbpclass																			///< 24bitpp rgb
{
	public:
		inline dbprgb24class( pointclass _size = 0, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true, int _bytesize = 0 ) : dbpclass()
		{	_create( _size, 24, 3, false, 0, _datapos, _bytelinesize, createbuffer, _bytesize );	}
		inline void create( pointclass _size )
		{	_create( _size, 24, 3, false, 0, datapos, 0, true, 0 );	}
		inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			*(img+p)   = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p+2) = (byte)b;	}
		inline void set24( const int &r, const int &g, const int &b )
		{	*(img+p)   = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p+2) = (byte)b;	}
		inline void set( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			*(img+p)   = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p+2) = (byte)b;	}
		inline void set( const int &r, const int &g, const int &b )
		{	*(img+p)   = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p+2) = (byte)b;	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(img+p);
			g = *(img+p+1);
			b = *(img+p+2);	}
		inline void get24( int &r, int &g, int &b )
		{	r = *(img+p);
			g = *(img+p+1);
			b = *(img+p+2);	}
		inline void get( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(img+p);
			g = *(img+p+1);
			b = *(img+p+2);	}
		inline void get( int &r, int &g, int &b )
		{	r = *(img+p);
			g = *(img+p+1);
			b = *(img+p+2);	}
};

/** pixel layout: b, g, r \n bytelinesize ist set to multiple of 32 bit \n pixel (0,0) has to be at the bytesize-bytelinesize */
class dbpbmp24class : public dbpclass																			///< 24bitpp rgb, windows compatible
{
	public:
		inline dbpbmp24class( pointclass _size = 0, bool filecompatible = false, bool createbuffer = true ) : dbpclass()
		{	if( filecompatible )
				datapos = 14+40;
			else
				datapos = 0;
			_create( _size, 24, 3, false, 0, datapos, ( ( _size.x * 3 - 1 ) / 4 + 1 ) * 4, createbuffer, 0 );	}
		inline void create( pointclass _size )
		{	_create( _size, 24, 3, false, 0, datapos, ( ( _size.x * 3 - 1 ) / 4 + 1 ) * 4, true, 0 );	}
		inline int & pos( const int &x, const int &y )
		{	return( p = bytesize - bytelinesize * ( y + 1 ) + ( x + x + x )  );	}
		inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	p = pos( x, y );
			*(img+p+2) = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p)   = (byte)b;	}
		inline void set24( const int &r, const int &g, const int &b )
		{	*(img+p+2) = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p)   = (byte)b;	}
		inline void set( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	p = pos( x, y );
			*(img+p+2) = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p)   = (byte)b;	}
		inline void set( const int &r, const int &g, const int &b )
		{	*(img+p+2) = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p)   = (byte)b;	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	p = pos( x, y );
			r = *(img+p+2);
			g = *(img+p+1);
			b = *(img+p);	}
		inline void get24( int &r, int &g, int &b )
		{	r = *(img+p+2);
			g = *(img+p+1);
			b = *(img+p);	}
		inline void get( const int &x, const int &y, int &r, int &g, int &b )
		{	p = pos( x, y );
			r = *(img+p+2);
			g = *(img+p+1);
			b = *(img+p);	}
		inline void get( int &r, int &g, int &b )
		{	r = *(img+p+2);
			g = *(img+p+1);
			b = *(img+p);	}

		bool parsefile( __attribute__((__unused__)) int imglen )
		{
			bool okay;
			okay = true;
			okay &= *(char*)( img + 0 ) == 'B';
			okay &= *(char*)( img + 1 ) == 'M';
			bytesize = *(int*)( img + 2 ); // size of file
			okay &= bytesize <= imglen;
			okay &= *(short*)( img + 6 ) == 0;
			okay &= *(short*)( img + 8 ) == 0;
			datapos = *(int*)( img + 10 );

			okay &= *(int*)( img + 14 ) >= 40; // sizeof bitmapinfoheader
			size.x = *(int*)( img + 18 );
			size.y = *(int*)( img + 22 );
			okay &= *(short*)( img + 26 ) == 1; // planes
			okay &= *(short*)( img + 28 ) == bitsperpixel; // bpp
			okay &= *(int*)( img + 30 ) == 0; // compression
			// *(int*)( img + 34 ) = bytesize - datapos; // size of image, unused without compression
			// *(int*)( img + 46 ) = 0; // number of color used
				
			bytelinesize = ( ( size.x * 3 - 1 ) / 4 + 1 ) * 4;
			
			return( okay );
		}
		bool makefile()
		{
			if( datapos >= 14+40 )
			{
				*(char*)( img + 0 ) = 'B';
				*(char*)( img + 1 ) = 'M';
				*(int*)( img + 2 ) = bytesize; // size of file
				*(short*)( img + 6 ) = 0;
				*(short*)( img + 8 ) = 0;
				*(int*)( img + 10 ) = datapos;

				*(int*)( img + 14 ) = 40; // sizeof bitmapinfoheader
				*(int*)( img + 18 ) = size.x;
				*(int*)( img + 22 ) = size.y;
				*(short*)( img + 26 ) = 1; // planes
				*(short*)( img + 28 ) = bitsperpixel; // bpp
				*(int*)( img + 30 ) = 0; // compression
				*(int*)( img + 34 ) = bytesize - datapos; // size of image, unused without compression
				*(int*)( img + 38 ) = 10000; // pixels per meter
				*(int*)( img + 42 ) = 10000; // pixels per meter
				*(int*)( img + 46 ) = 0; // number of color used
				*(int*)( img + 50 ) = 0; // number of color required for displaying 
				return( true );
			} else {
				return( false );
			}
		}

};

/** pixel layout: b, g, r, 0 \n pixel (0,0) has to be at the datapos */
class dbpbmp32class : public dbpclass																			///< 24/32bitpp rgb, symbian compatible
{
	public:
		inline dbpbmp32class( pointclass _size = 0, bool createbuffer = true ) : dbpclass()
		{	_create( _size, 32, 4, false, 0, 0, ( ( _size.x * 4 - 1 ) / 4 + 1 ) * 4, createbuffer, 0 );
			channels = 3;	}
		inline void create( pointclass _size )
		{	_create( _size, 32, 4, false, 0, datapos, ( ( _size.x * 4 - 1 ) / 4 + 1 ) * 4, true, 0 );
			channels = 3;	}
		inline int & pos( const int &x, const int &y )
		{	return( p = datapos + bytelinesize * y + ( x + x + x + x ) );	}
		inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			*(img+p+2) = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p)   = (byte)b;	}
		inline void set24( const int &r, const int &g, const int &b )
		{	*(img+p+2) = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p)   = (byte)b;	}
		inline void set( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			*(img+p+2) = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p)   = (byte)b;	}
		inline void set( const int &r, const int &g, const int &b )
		{	*(img+p+2) = (byte)r;
			*(img+p+1) = (byte)g;
			*(img+p)   = (byte)b;	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(img+p+2);
			g = *(img+p+1);
			b = *(img+p);	}
		inline void get24( int &r, int &g, int &b )
		{	r = *(img+p+2);
			g = *(img+p+1);
			b = *(img+p);	}
		inline void get( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(img+p+2);
			g = *(img+p+1);
			b = *(img+p);	}
		inline void get( int &r, int &g, int &b )
		{	r = *(img+p+2);
			g = *(img+p+1);
			b = *(img+p);	}
};

/** pixel layout: rr, gg, bb */
class dbprgb48class : public dbpclass																			///< 48bitpp rgb
{
	public:
		inline dbprgb48class( pointclass _size = 0, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true, int _bytesize = 0 ) : dbpclass()
		{	_create( _size, 48, 3, false, 0, _datapos, _bytelinesize, createbuffer, _bytesize );	}
		inline void create( pointclass _size )
		{	_create( _size, 48, 3, false, 0, datapos, 0, true, 0 );	}
		inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			*(ushort*)(img+p)   = (ushort)( (r<<8) | r );
			*(ushort*)(img+p+2) = (ushort)( (g<<8) | g );
			*(ushort*)(img+p+4) = (ushort)( (b<<8) | b );	}
		inline void set24( const int &r, const int &g, const int &b )
		{	*(ushort*)(img+p)   = (ushort)( (r<<8) | r );
			*(ushort*)(img+p+2) = (ushort)( (g<<8) | g );
			*(ushort*)(img+p+4) = (ushort)( (b<<8) | b );	}
		inline void set( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			*(ushort*)(img+p)   = (ushort)r;
			*(ushort*)(img+p+2) = (ushort)g;
			*(ushort*)(img+p+4) = (ushort)b;	}
		inline void set( const int &r, const int &g, const int &b )
		{	*(ushort*)(img+p)   = (ushort)r;
			*(ushort*)(img+p+2) = (ushort)g;
			*(ushort*)(img+p+4) = (ushort)b;	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(ushort*)(img+p  ) >> 8;
			g = *(ushort*)(img+p+2) >> 8;
			b = *(ushort*)(img+p+4) >> 8;	}
		inline void get24( int &r, int &g, int &b )
		{	r = *(ushort*)(img+p  ) >> 8;
			g = *(ushort*)(img+p+2) >> 8;
			b = *(ushort*)(img+p+4) >> 8;	}
		inline void get( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(ushort*)(img+p  );
			g = *(ushort*)(img+p+2);
			b = *(ushort*)(img+p+4);	}
		inline void get( int &r, int &g, int &b )
		{	r = *(ushort*)(img+p  );
			g = *(ushort*)(img+p+2);
			b = *(ushort*)(img+p+4);	}
};

class dbpbw8class : public dbpclass																				///< 8bitpp monochrome
{
	public:
		inline dbpbw8class( pointclass _size = 0, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true, int _bytesize = 0 ) : dbpclass()
		{	_create( _size, 8, 1, false, 0, _datapos, _bytelinesize, createbuffer, _bytesize );	}
		inline void create( pointclass _size )
		{	_create( _size, 8, 1, false, 0, datapos, 0, true, 0 );	}
		inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			*(img+p) = (byte)( (r+g+b)/3 );	}
		inline void set24( const int &r, const int &g, const int &b )
		{	*(img+p) = (byte)( (r+g+b)/3 );	}
		inline void set( const int &x, const int &y, const int &r, __attribute__((__unused__)) const int &g, const int __attribute__((__unused__)) &b )
		{	pos( x, y );
			*(img+p) = (byte)r;	}
		inline void set( const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	*(img+p) = (byte)r;	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(img+p);
			g = r; b = r;	}
		inline void get24( int &r, int &g, int &b )
		{	r = *(img+p);
			g = r; b = r;	}
		inline void get( const int &x, const int &y, int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	pos( x, y );
			r = *(img+p);	}
		inline void get( int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	r = *(img+p);	}
};

class dbppal8class : public dbpclass
{
	public:
		inline dbppal8class( pointclass _size = 0, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true, int _bytesize = 0 ) : dbpclass()
		{	if( _datapos == 0 )
				_datapos = ( 1 << 8 ) * 3;
			_create( _size, 8, 3, true, 0, _datapos, _bytelinesize, createbuffer, _bytesize );	}
		inline void create( pointclass _size )
		{	_create( _size, 8, 3, true, 0, datapos, 0, true, 0 );	}
		inline void getcol( int &r, int &g, int &b )
		{	pp = palpos + ( r + r + r );
			r = (int)*(img+pp);
			g = (int)*(img+pp+1);
			b = (int)*(img+pp+2);	}
		inline void setcol( const int &index, const int &r, const int &g, const int &b )
		{	pp = palpos + ( index + index + index );
			*(img+pp)   = r;
			*(img+pp+1) = g;
			*(img+pp+2) = b;	}
		inline void setcol( const int &r, const int &g, const int &b )
		{	*(img+pp)   = r;
			*(img+pp+1) = g;
			*(img+pp+2) = b;	}
		inline void set( const int &x, const int &y, const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	pos( x, y );
			*(img+p) = (byte)r;	}
		inline void set( const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	*(img+p) = (byte)r;	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(img+p);
			getcol( r, g, b );	}
		inline void get24( int &r, int &g, int &b )
		{	r = *(img+p);
			getcol( r, g, b );	}
		inline void get( const int &x, const int &y, int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	pos( x, y );
			r = *(img+p);	}
		inline void get( int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	r = *(img+p);	}
};

class dbpbw16class : public dbpclass																			///< 16bitpp monochrome
{
	public:
		inline dbpbw16class( pointclass _size = 0, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true, int _bytesize = 0 ) : dbpclass()
		{	_create( _size, 16, 1, false, 0, _datapos, _bytelinesize, createbuffer, _bytesize );	}
		inline void create( pointclass _size )
		{	_create( _size, 16, 1, false, 0, datapos, 0, true, 0 );	}
		inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			_r = (r+g+b)/3;
			*(ushort*)(img+p) = (ushort)( (_r<<8) | _r );	}
		inline void set24( const int &r, const int &g, const int &b )
		{	_r = (r+g+b)/3;
			*(ushort*)(img+p) = (ushort)( (_r<<8) | _r );	}
		inline void set( const int &x, const int &y, const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	pos( x, y );
			*(ushort*)(img+p) = (ushort)r;	}
		inline void set( const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	*(ushort*)(img+p) = (ushort)r;	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(ushort*)(img+p) >> 8;
			g = r; b = r;	}
		inline void get24( int &r, int &g, int &b )
		{	r = *(ushort*)(img+p) >> 8;
			g = r; b = r;	}
		inline void get( const int &x, const int &y, int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	pos( x, y );
			r = *(ushort*)(img+p);	}
		inline void get( int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	r = *(ushort*)(img+p);	}
};

class dbpbw32class : public dbpclass																			///< 32bitpp monochrome
{
	public:
		inline dbpbw32class( pointclass _size = 0, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true, int _bytesize = 0 ) : dbpclass()
		{	_create( _size, 32, 1, false, 0, _datapos, _bytelinesize, createbuffer, _bytesize );	}
		inline void create( pointclass _size )
		{	_create( _size, 32, 1, false, 0, datapos, 0, true, 0 );	}
		inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			_r = (r+g+b)/3;
			*(uint*)(img+p) = (uint)( (_r<<24) | (_r<<16) | (_r<<8) | _r );	}
		inline void set24( const int &r, const int &g, const int &b )
		{	_r = (r+g+b)/3;
			*(uint*)(img+p) = (uint)( (_r<<24) | (_r<<16) | (_r<<8) | _r );	}
		inline void set( const int &x, const int &y, const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	pos( x, y );
			*(uint*)(img+p) = (uint)r;	}
		inline void set( const int &r, __attribute__((__unused__)) const int &g, const __attribute__((__unused__)) int &b )
		{	*(uint*)(img+p) = (uint)r;	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(uint*)(img+p) >> 24;
			g = r; b = r;	}
		inline void get24( int &r, int &g, int &b )
		{	r = *(uint*)(img+p) >> 24;
			g = r; b = r;	}
		inline void get( const int &x, const int &y, int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	pos( x, y );
			r = *(uint*)(img+p);	}
		inline void get( int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	r = *(uint*)(img+p);	}
};

class dbpbwclass : public dbpclass																				///< variable bitpp (1,2,4,8) monochrome
{
	public:
		dbpbwclass( pointclass _size = 0, int _bits = 8 ) : dbpclass()
		{	_create( _size, _bits, 1, false, 0, 0, 0, true, 0 );	}
		inline void create( pointclass _size )
		{	_create( _size, bitsperpixel, 1, false, 0, 0, 0, true, 0 );	}
		inline int & pos( const int &x, const int &y )
		{	p = y * size.x + x;
			bi = ( p & pixelsperbyte_1 ) << lgbitsperpixel;
			return( p = datapos + ( p >> lgpixelsperbyte ) );	}
		inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			*(img+p) = (byte)( *(img+p) & ~( mask << bi ) ) | ( ( ((r+g+b)*mask/(0xff)*3) & mask ) << bi );	}
		inline void set24( const int &r, const int &g, const int &b )
		{	*(img+p) = (byte)( *(img+p) & ~( mask << bi ) ) | ( ( ((r+g+b)*mask/(0xff)*3) & mask ) << bi );	}
		inline void set( const int &x, const int &y, const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	pos( x, y );
			*(img+p) = (byte)( *(img+p) & ~( mask << bi ) ) | ( ( r & mask ) << bi );	}
		inline void set( const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	*(img+p) = (byte)( *(img+p) & ~( mask << bi ) ) | ( ( r & mask ) << bi );	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = ( *(img+p) >> bi ) & mask;
			r = r * 0xff / mask;
			g = r; b = r;	}
		inline void get24( int &r, int &g, int &b )
		{	r = ( *(img+p) >> bi ) & mask;
			r = r * 0xff / mask;
			g = r; b = r;	}
		inline void get( const int &x, const int &y, int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	pos( x, y );
			r = ( *(img+p) >> bi ) & mask;	}
		inline void get( int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	r = ( *(img+p) >> bi ) & mask;	}
};

class dbppalclass : public dbpclass
{
	public:
		inline dbppalclass( pointclass _size = 0, int _bits = 8, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true, int _bytesize = 0 ) : dbpclass()
		{	if( _datapos == 0 )
				_datapos = ( 1 << _bits ) * 3;
			_create( _size, _bits, 3, true, 0, _datapos, _bytelinesize, createbuffer, _bytesize );	}
		inline void create( pointclass _size )
		{	_create( _size, bitsperpixel, 3, true, 0, datapos, 0, true, 0 );	}
		inline void getcol( int &r, int &g, int &b )
		{	pp = palpos + ( r + r + r );
			r = (int)*(img+pp);
			g = (int)*(img+pp+1);
			b = (int)*(img+pp+2);	}
		inline void setcol( const int &index, const int &r, const int &g, const int &b )
		{	pp = palpos + ( index + index + index );
			*(img+pp) = r;
			*(img+pp+1) = g;
			*(img+pp+2) = b;	}
		inline void setcol( const int &r, const int &g, const int &b )
		{	*(img+pp) = r;
			*(img+pp+1) = g;
			*(img+pp+2) = b;	}
		inline int & pos( const int &x, const int &y )
		{	p = y * size.x + x;
			bi = ( p & pixelsperbyte_1 ) << lgbitsperpixel;
			return( p = datapos + ( p >> lgpixelsperbyte ) );	}
		inline void set( const int &x, const int &y, const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	pos( x, y );
			*(img+p) = (byte)( *(img+p) & ~( mask << bi ) ) | ( ( r & mask ) << bi );	}
		inline void set( const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	*(img+p) = (byte)( *(img+p) & ~( mask << bi ) ) | ( ( r & mask ) << bi );	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = ( *(img+p) >> bi ) & mask;
			getcol( r, g, b );	}
		inline void get24( int &r, int &g, int &b )
		{	r = ( *(img+p) >> bi ) & mask;
			getcol( r, g, b );	}
		inline void get( const int &x, const int &y, int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	pos( x, y );
			r = ( *(img+p) >> bi ) & mask;	}
		inline void get( int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	r = ( *(img+p) >> bi ) & mask;	}
};

class dbppal8pngclass : public dbpclass
{
	public:
		inline dbppal8pngclass( pointclass _size = 0, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true, int _bytesize = 0 ) : dbpclass()
		{	if( _datapos == 0 )
				_datapos = ( 1 << 8 ) * 3;
			if( _bytelinesize == 0 )
				_bytelinesize =  _size.x + 1;
			if( _bytesize == 0 )
				_bytesize = _datapos + _bytelinesize * _size.y + 1;
			_create( _size, 8, 3, true, 0, _datapos, _bytelinesize, createbuffer, _bytesize );	}
		inline void create( pointclass _size )
		{	bytelinesize = _size.x + 1;
			bytesize = datapos + bytelinesize * _size.y + 1;
			_create( _size, 8, 3, true, 0, datapos, 0, true, bytesize );	}
		inline void getcol( int &r, int &g, int &b )
		{	pp = palpos + ( r + r + r ) ;
			r = (int)*(img+pp);
			g = (int)*(img+pp+1);
			b = (int)*(img+pp+2);	}
		inline void setcol( const int &index, const int &r, const int &g, const int &b )
		{	pp = palpos + ( index + index + index );
			*(img+pp) = r;
			*(img+pp+1) = g;
			*(img+pp+2) = b;	}
		inline void setcol( const int &r, const int &g, const int &b )
		{	*(img+pp) = r;
			*(img+pp+1) = g;
			*(img+pp+2) = b;	}
		inline int & pos( const int &x, const int &y )
		{	return( p = datapos + y * bytelinesize + x );	}
		inline void set( const int &x, const int &y, const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	pos( x, y );
			*(img+p) = (byte)r;
		}
		inline void set( const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{
			*(img+p) = (byte)r;
		}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = *(img+p);
			getcol( r, g, b );	}
		inline void get24( int &r, int &g, int &b )
		{	r = *(img+p);
			getcol( r, g, b );	}
		inline void get( const int &x, const int &y, int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	pos( x, y );
			r = *(img+p);	}
		inline void get( int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	r = *(img+p);	}
};

class dbppalpngclass : public dbpclass
{
	public:
		inline dbppalpngclass( pointclass _size = 0, int _bits = 8, int _datapos = 0, int _bytelinesize = 0, bool createbuffer = true, int _bytesize = 0 ) : dbpclass()
		{	if( _datapos == 0 )
				_datapos = ( 1 << _bits ) * 3;
			if( _bytelinesize == 0 )
				_bytelinesize = ( _bits * _size.x + 7 ) / 8 + 1;
			if( _bytesize == 0 )
				_bytesize = _datapos + _bytelinesize * _size.y + 1;
			_create( _size, _bits, 3, true, 0, _datapos, _bytelinesize, createbuffer, _bytesize );	}
		inline void create( pointclass _size )
		{	bytelinesize = ( bitsperpixel * _size.x + 7 ) / 8 + 1;
			bytesize = datapos + bytelinesize * _size.y + 1;
			_create( _size, bitsperpixel, 3, true, 0, datapos, 0, true, bytesize );	}
		inline void getcol( int &r, int &g, int &b )
		{	pp = palpos + ( r + r + r );
			r = (int)*(img+pp);
			g = (int)*(img+pp+1);
			b = (int)*(img+pp+2);	}
		inline void setcol( const int &index, const int &r, const int &g, const int &b )
		{	pp = palpos + ( index + index + index );
			*(img+pp) = r;
			*(img+pp+1) = g;
			*(img+pp+2) = b;	}
		inline void setcol( const int &r, const int &g, const int &b )
		{	*(img+pp) = r;
			*(img+pp+1) = g;
			*(img+pp+2) = b;	}
		inline int & pos( const int &x, const int &y )
		{	bi = ( x & pixelsperbyte_1 ) << lgbitsperpixel;
			return( p = datapos + y * bytelinesize + ( x >> lgpixelsperbyte ) );	}
		inline void set( const int &x, const int &y, const int &r, __attribute__((__unused__)) const int &g, const __attribute__((__unused__)) int &b )
		{	pos( x, y );
			*(img+p) = (byte)( *(img+p) & ~( mask << bi ) ) | ( ( r & mask ) << bi );	}
		inline void set( const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	*(img+p) = (byte)( *(img+p) & ~( mask << bi ) ) | ( ( r & mask ) << bi );	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			r = ( (int)*(img+p) >> bi ) & mask;
			getcol( r, g, b );	}
		inline void get24( int &r, int &g, int &b )
		{	r = ( *(img+p) >> bi ) & mask;
			getcol( r, g, b );	}
		inline void get( const int &x, const int &y, int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	pos( x, y );
			r = ( *(img+p) >> bi ) & mask;	}
		inline void get( int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	r = ( *(img+p) >> bi ) & mask;	}
};

/** pixel layout: rrrgggbb */
class dbpdbp1class : public dbpclass																			///< 8bitpp rgb
{
	public:
	
		dbpdbp1class( pointclass _size = 0 ) : dbpclass()
		{	_create( _size, 8, 3, true, 0, 0, 0, true ); paletted = false;	}
		inline void create( pointclass _size )
		{	_create( _size, 8, 3, true, 0, 0, 0, true ); paletted = false;	}
		inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{	pos( x, y );
			*(img+p) = (byte)( ( r & 0xe0 ) | ( g & 0xe0 ) >> 3 | ( b & 0xc0 ) >> 6 );	}
		inline void set24( const int &r, const int &g, const int &b )
		{	*(img+p) = (byte)( ( r & 0xe0 ) | ( g & 0xe0 ) >> 3 | ( b & 0xc0 ) >> 6 );	}
		inline void set( const int &x, const int &y, const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	pos( x, y );
			*(img+p) = (byte)r;	}
		inline void set( const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b )
		{	*(img+p) = (byte)r;	}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{	pos( x, y );
			_r = *(img+p);
			r = ( _r & 0xe0 ) * 255/224;
			g = ( _r & 0x1c ) * 255/28;
			b = ( _r & 0x03 ) * 255/3;	}
		inline void get24( int &r, int &g, int &b )
		{	_r = *(img+p);
			r = ( _r & 0xe0 ) * 255/224;
			g = ( _r & 0x1c ) * 255/28;
			b = ( _r & 0x03 ) * 255/3;	}
		inline void get( const int &x, const int &y, int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	pos( x, y );
			r = *(img+p);	}
		inline void get( int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b )
		{	r = *(img+p);	}
};

/*
class dbpbwpngclass : public dbpclass
{
	public:

		dbpbwpngclass( pointclass _size = 0, int _bits = 8 ) : dbpclass( _size, _bits, 1, false, 0, 0, _bits * _size.x / 8, true ) {}
		inline void create( pointclass _size )
		{
			_create( _size, bitsperpixel, 1, false, 0, 0, bits * _size.x / 8 );
		}

		inline int pos( const int &x, const int &y )
		{
			p = ( x >> lgpixelsperbyte ) + 1;
			bytes = datapos + y * bytelinesize + p;
			bits = pixelsperbyte - 1 - ( p % pixelsperbyte );
			return( bytes );
		}
		inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{
			pos( x, y );
			p = ( ( r + g + b ) / 3 >> ( 8 - bitsperpixel ) );
			img[bytes] = (byte)( ( ( p & mask ) << ( bits << lgbitsperpixel ) ) | ( img[bytes] & ( ( mask << ( bits << lgbitsperpixel ) ) ^ 0xff ) ) );
		}
		inline void set( const int &x, const int &y, const int &r, const int &g, const int &b )
		{
			pos( x, y );
			img[bytes] = (byte)( ( ( r & mask ) << ( bits << lgbitsperpixel ) ) | ( img[bytes] & ( ( mask << ( bits << lgbitsperpixel ) ) ^ 0xff ) ) );
		}
		inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{
			pos( x, y );
			r = ( ( img[bytes] >> ( bits << lgbitsperpixel ) ) & mask ) * 0xff / mask;
			g = r;
			b = r;	
		}
		inline void get( const int &x, const int &y, int &r, int &g, int &b )
		{
			pos( x, y );
			r = ( ( img[bytes] >> ( bits << lgbitsperpixel ) ) & mask );
		}
};
*/

class dbpnormalizeclass : public dbpclass
{
	public:

		int norm;

	#include "db_dbpclass_dbpnormalizeclass_paintbasics.cpp"

		dbpnormalizeclass() : dbpclass()
		{
			_create();
		}
		virtual inline void set24( const int &x, const int &y, const int &r, const int &g, const int &b )
		{
			set( x, y, r, g, b, norm );
		}
		virtual inline void set( const int &x, const int &y, const int &r, const int &g, const int &b )
		{
			set( x, y, r, g, b, norm );
		}
		virtual inline void set( __attribute__((__unused__)) const int &x, __attribute__((__unused__)) const int &y, __attribute__((__unused__)) const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b, __attribute__((__unused__)) const int &n )
		{
		}
		virtual inline void get(__attribute__((__unused__))  const int &x, __attribute__((__unused__)) const int &y, __attribute__((__unused__)) int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b, __attribute__((__unused__)) int &n )
		{
		}
		virtual inline void get24( const int &x, const int &y, int &r, int &g, int &b )
		{
			get( x, y, r, g, b );
		}
		virtual inline void get( const int &x, const int &y, int &r, int &g, int &b )
		{
			int n;
			get( x, y, r, g, b, n );
			if( n > 0 )
			{
	            r = r * norm / n;
		        g = g * norm / n;
			    b = b * norm / n;
			} else {
				r = 0;
				g = 0;
				b = 0;
			}
		}
};


class dbpbwn16class : public dbpnormalizeclass
{
	public:

		dbpbwn16class( pointclass _size, int _norm = 0xff ) : dbpnormalizeclass()
		{
			_create( _size, 32, 2 );
			norm = _norm;
		}
		inline void set( const int &x, const int &y, const int &r, __attribute__((__unused__)) const int &g, __attribute__((__unused__)) const int &b, const int &n )
		{
			*( (unsigned long *)(img+pos( x, y )) ) = ( r << 16 ) | n;
		}
		inline void get( const int &x, const int &y, int &r, __attribute__((__unused__)) int &g, __attribute__((__unused__)) int &b, int &n )
		{
			r = *( (unsigned long *)(img+pos( x, y )) );
			n = r & 0xffff;
			r = ( r >> 16 ) & 0xffff;
		}
};

/*
class dbprgbn48class : public dbpnormalizeclass
{
	public:

		dbprgbn48class( pointclass _size, int _norm = 0xff )
		{
			_create( _size, 64, 4 );
			norm = _norm;
		}
		inline void set( const int &x, const int &y, const int &r, const int &g, const int &b, const int &n )
		{
			p = pos( x, y );
			*( (ushort *)(img+p) ) = r;
			*( (ushort *)(img+p)+1 ) = g;
			*( (ushort *)(img+p)+2 ) = b;
			*( (ushort *)(img+p)+3 ) = n;
		}
		inline void get( const int &x, const int &y, int &r, int &g, int &b, int &n )
		{
			p = pos( x, y );
			r = *( (ushort *)(img+p) );
			g = *( (ushort *)(img+p)+1 );
			b = *( (ushort *)(img+p)+2 );
			n = *( (ushort *)(img+p)+3 );
		}
};
*/

class dbpbwndoubleclass : public dbpnormalizeclass
{
	public:

	#include "db_dbpclass_dbpbwndoubleclass_paintbasics.cpp"

		dbpbwndoubleclass()
		{
			_create();
			norm = 0;
		}
		dbpbwndoubleclass( pointclass _size )
		{
			_create( _size, sizeof(double)*2*8, 2 );
			norm = 0;
		}
		inline void set( const int &x, const int &y, const double &r, __attribute__((__unused__)) const double &g, __attribute__((__unused__)) const double &b, const double &n )
		{
			int p;
			p = pos( x, y );
			*( (double *)(img+p) ) = r;
			*( (double *)(img+p+sizeof(double)) ) = n;
		}
		inline void get( const int &x, const int &y, double &r, __attribute__((__unused__)) double &g, __attribute__((__unused__)) double &b, double &n )
		{
			int p;
			p = pos( x, y );
			r = *( (double *)(img+p) );
			n = *( (double *)(img+p+sizeof(double)) );
		}
};


#endif // DB_DBPCLASS_INCLUDED
