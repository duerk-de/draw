__GIT_SYMLINKS__
target=../_inc/db_dbpclass_paintbasics.cpp
target_resolved=/dibom/src/c/_inc/db_dbpclass_paintbasics.cpp
kind=file
sha256=a7c41032f99bfd14c0407c600f53224f28a6b8dd2b222fea89225600cf8f9f0b
size=4407
created=2026-11-15T11:20:55.360478
--END-HEADER--
#ifndef DB_DBPCLASS_PAINTBASICS_INCLUDED
#define DB_DBPCLASS_PAINTBASICS_INCLUDED


		// ----------------------------------
		void clear24( int r = 0, int g = 0, int b = 0 )
		{
			int x, y;
			for( y=0; y<size.y; y++ )
				for( x=0; x<size.x; x++ )
					set24( x, y, r, g, b );
		}

		void clear( int r, int g = 0, int b = 0 )
		{
			int x, y;
			for( y=0; y<size.y; y++ )
				for( x=0; x<size.x; x++ )
					set( x, y, r, g, b );
		}

		void clear()
		{
			int i, j, l;
			unsigned int *idst;
			l = bytesize - datapos;
			j = l / sizeof(int);
			idst = (unsigned int*)( img + datapos );
			for( i=0; i<j; i++ )
				*( idst + i ) = 0x00000000;
			for( i*=sizeof(int); i<l; i++ )
				*( img + datapos + i ) = 0x00;
		}

		void clearwhite()
		{
			int i, j, l;
			unsigned int *idst;
			l = bytesize - datapos;
			j = l / sizeof(int);
			idst = (unsigned int*)( img + datapos );
			for( i=0; i<j; i++ )
				*( idst + i ) = 0xffffffff;
			for( i*=sizeof(int); i<l; i++ )
				*( img + datapos + i ) = 0xff;
		}

		inline void convert( dbpclass *inputdbp, int &r, int &g, int &b )
		{
			if( !( mask == inputdbp->mask && channels == inputdbp->channels ) )
			{
				if( inputdbp->paletted )
					inputdbp->getcol( r, g, b );
				r = r * mask / inputdbp->mask;
				if( inputdbp->channels < 3 )
				{
					if( channels > 2 )
					{
						g = r;
						b = r;
					}
				} else {
					g = g * mask / inputdbp->mask;
					b = b * mask / inputdbp->mask;
					if( channels < 3 )
						r = ( r + g + b ) / 3;
				}
			}
		}
	
		template <typename f_t = double>
		static f_t b10_max( f_t chi )
		{
			return( (f_t)2.6171504655 * chi );
		}
		template <typename f_t = double>
		static f_t b10( f_t chi )
		{
			f_t b10;
			if( chi > (f_t)2.6171504655 ) {
				b10 = (f_t)0.0;
			} else {
				if( chi > (f_t)1.4326566565 ) {
					b10 = (f_t)0.01831563889 * ( (f_t)141 + chi/(f_t)3 * ( (f_t)-640 + chi * ( (f_t)369 + chi * ( (f_t)-96 + (f_t)9.5*chi ) ) ) );
				} else {
					if( chi > (f_t)0.55818262732 ) {
						b10 = (f_t)0.3678794412 * ( (f_t)2.5 + chi/(f_t)3 * ( (f_t)4 + chi * ( (f_t)-18 + chi * ( (f_t)12 - (f_t)2.5*chi ) ) ) );
					} else {
						chi *= chi;
						b10 = (f_t)1 - chi * ( (f_t)1 - chi * ( (f_t)0.5 - chi/(f_t)6 ) );
					}
				}
			}
			return( b10 );
		}

		template <typename f_t = double>
		static constexpr f_t b14_max( f_t chi )
		{
			return( (f_t)3.167 * chi );
		}
		template <typename f_t = double>
		static f_t b14( f_t chi )
		{
			f_t b14;
			if( chi > (f_t)3.167 )
			{
				b14 = (f_t)0.0;
			} else {
				if( chi > (f_t)2.6238 ) {
				    b14 = (f_t)1.2340980408667956e-4 * ( (f_t)19 - (f_t)6 * chi );
				} else {
					if( chi > (f_t)1.4326566565 ) {
						b14 = (f_t)0.01831563889 * ( (f_t)141 + chi/(f_t)3 * ( (f_t)-640 + chi * ( (f_t)369 + chi * ( (f_t)-96 + (f_t)9.5*chi ) ) ) );
					} else {
						if( chi > (f_t)0.55818262732 ) {
							b14 = (f_t)0.3678794412 * ( (f_t)2.5 + chi/(f_t)3 * ( (f_t)4 + chi * ( (f_t)-18 + chi * ( (f_t)12 - (f_t)2.5*chi ) ) ) );
						} else {
							chi *= chi;
							b14 = (f_t)1 - chi * ( (f_t)1 - chi * ( (f_t)0.5 - chi/(f_t)6 ) );
						}
					}
				}
			}
			return( b14 );
		}
		

		template <typename f_t = double>
		static f_t sqrt15( f_t arg )
		{
			f_t a, a2;
			if( arg < (f_t)0.0 )
			{
				return( (f_t)0.0 );
			} else {
				if( arg > (f_t)4.0 )
				{
					return( (f_t)1.0 );
				} else {
					a = arg - (f_t)1.5;
					a2 = a*a;
					return( (f_t)1.2247448719315894 + (f_t)0.4082482904638629*a - (f_t)0.06804138174397717*a2 + (f_t)0.02268046058132572*a2*a - (f_t)0.009450191908885717*a2*a2 );
				}
			}
		}
		
		template <typename f_t = double>
		static f_t sqrt15( f_t swidth, f_t sheight )
		{
			f_t swidth2, sheight2, length;
			swidth2 = swidth * swidth;
			sheight2 = sheight * sheight;
			if( swidth2 || sheight2 )
			{
				if( swidth2 > sheight2 )
					if( swidth2 )
						length = sqrt15( ( swidth2 + sheight2 ) / swidth2 ) * d_abs( swidth );
					else
						length = d_abs( sheight );
				else
					if( sheight2 )
						length = sqrt15( ( swidth2 + sheight2 ) / sheight2 ) * d_abs( sheight );
					else
						length = d_abs( swidth );
			} else {
				length = (f_t)0.0;
			}
			return( length );
		}


#endif // DB_DBPCLASS_PAINTBASICS_INCLUDED
