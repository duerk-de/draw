__GIT_SYMLINKS__
target=../_inc/db_dbpclass_paint.cpp
target_resolved=/dibom/src/c/_inc/db_dbpclass_paint.cpp
kind=file
sha256=1d5d2f2a90244fe45a291b6fc60d6870e6ff32cef9b8fe4c2dd96d1836f68740
size=36485
created=2026-11-15T11:20:55.358099
--END-HEADER--
#ifndef DB_DBPCLASS_PAINT_INCLUDED
#define DB_DBPCLASS_PAINT_INCLUDED


		const static int NONE = 0, ADD = 1, SUBTRACT = 2, AVERAGE = 3;

        void createiparray( int resize )
        {
        	int z, zoomin, x, y;
			if( resize < 0 )
			{
				resize = -resize;
				zoomin = 1 << resize;
				z = zoomin + 1;
				if( z != ipz && !heaplocked )
				{
					if( iparray )
						delete [] iparray;
					iparray = new int[z*z];
					ipz = z;
					for( y=0; y<z; y++ )
					{
						for( x=0; x<z; x++ )
						{
							iparray[x+y*z] =  (zoomin+1) * zoomin - x * x - y * y;
							if( iparray[x+y*z] < 0 )
								iparray[x+y*z] = 0;
						}
					}
				}
			}
        }
        
        // copies src bitmap to this bitmap at position pos
		// resize is log2 of 1/magnification (the zoomout value), rgbproc may process source bitmap colors
		// srcrgbproc may process destination bitmap colors but affects only if overlay is activated
		void copy24( dbpclass *src, pointclass pos, int resize, int overlay, void (*rgbproc)( int &r, int &g, int &b ), void (*srcrgbproc)( int &r, int &g, int &b ) = null )
		{
			rectclass range;
			int zoomout, zoomin;
			int x, y, u, v, r, g, b, r1, g1, b1;
			int _r[4], _g[4], _b[4], d[4], n;
			int i, j, k, l, z, _x, _y, z2;

			range.set( pointclass( 0, 0 ), src->size );
			if( range.width + range.x > src->size.x )
				range.width = src->size.x - range.x;
			if( range.height + range.y > src->size.y )
				range.height = src->size.y - range.y;
			if( range.x > src->size.x )
				range.x = src->size.x;
			if( range.y > src->size.y )
				range.y = src->size.y;
			
			zoomout = 0;
			zoomin = 0;
			if( resize > 0 ) {
				zoomout = 1 << resize;
				range = range / zoomout;
				z = resize << 1;
			} else if( resize < 0 ) {
				zoomin = 1 << (-resize);
				range = range * zoomin;
			}

			if( resize < 0 )
			{
				range = range / zoomin;

				if( pos.x + range.width*zoomin > size.x )
					range.width = ( size.x - pos.x ) / zoomin;
				if( pos.y + range.height*zoomin > size.y )
					range.height = ( size.y - pos.y ) / zoomin;
				if( pos.x < 0 )
					range.x = ( -pos.x + zoomin - 1 ) / zoomin;
				if( pos.y < 0 )
					range.y = ( -pos.y + zoomin - 1 ) / zoomin;

				if( range.width > range.x && range.height > range.y )
				{
					range.setresize( pointclass( -1, -1 ) );
					pos = pos + zoomin / 2;
					// range = ( ( range + zoomin ) << (2*zoomin+1) ) / zoomin;
					resize = -resize;
					z2 = zoomin / 2;
					z = zoomin + 1;
					if( z != ipz && !heaplocked )
					{
						if( iparray )
							delete [] iparray;
						iparray = new int[z*z];
						ipz = z;
						for( y=0; y<z; y++ )
						{
							for( x=0; x<z; x++ )
							{
								iparray[x+y*z] =  (zoomin+1) * zoomin - x * x - y * y;
								if( iparray[x+y*z] < 0 )
									iparray[x+y*z] = 0;
							}
						}
					}
					if( z == ipz )
					{
						x = range.x - 1;
						y = range.y - 1;
						// upper left corner
						src->get24( x+1, y+1, r, g, b );
						for( j=z2; j<zoomin; j++ )
						{
							for( i=z2; i<zoomin; i++ )
							{
								_x = ( x << resize ) + i + pos.x;
								_y = ( y << resize ) + j + pos.y;
								copyproc( src, _x, _y, r, g, b, overlay, rgbproc, srcrgbproc );
								set24( _x, _y, r, g, b );
							}
						}
						// upper border
						for( x=range.x; x<range.width; x++ )
						{
							src->get24( x, y+1, _r[2], _g[2], _b[2] );
							src->get24( x+1, y+1, _r[3], _g[3], _b[3] );
							for( j=z2; j<zoomin; j++ )
							{
								for( i=0; i<zoomin; i++ )
								{
									k = zoomin - i;
									l = zoomin - j;
									d[2] = iparray[i+z*l];
									d[3] = iparray[k+z*l];
									n = d[2] + d[3];
									r = ( _r[2] * d[2] + _r[3] * d[3] ) / n;
									g = ( _g[2] * d[2] + _g[3] * d[3] ) / n;
									b = ( _b[2] * d[2] + _b[3] * d[3] ) / n;
									_x = ( x << resize ) + i + pos.x;
									_y = ( y << resize ) + j + pos.y;
									copyproc( src, _x, _y, r, g, b, overlay, rgbproc, srcrgbproc );
									set24( _x, _y, r, g, b );
								}
							}
						}
						// upper right corner
						src->get24( x, y+1, r, g, b );
						for( j=z2; j<zoomin; j++ )
						{
							for( i=0; i<z2; i++ )
							{
								_x = ( x << resize ) + i + pos.x;
								_y = ( y << resize ) + j + pos.y;
								copyproc( src, _x, _y, r, g, b, overlay, rgbproc, srcrgbproc );
								set24( _x, _y, r, g, b );
							}
						}
						for( y=range.y; y<range.height; y++ )
						{
							x = range.x - 1;
							// left border
							src->get24( x+1, y, _r[1], _g[1], _b[1] );
							src->get24( x+1, y+1, _r[3], _g[3], _b[3] );
							for( j=0; j<zoomin; j++ )
							{
								for( i=z2; i<zoomin; i++ )
								{
									k = zoomin - i;
									l = zoomin - j;
									d[1] = iparray[k+z*j];
									d[3] = iparray[k+z*l];
									n = d[1] + d[3];
									r = ( _r[1] * d[1] + _r[3] * d[3] ) / n;
									g = ( _g[1] * d[1] + _g[3] * d[3] ) / n;
									b = ( _b[1] * d[1] + _b[3] * d[3] ) / n;
									_x = ( x << resize ) + i + pos.x;
									_y = ( y << resize ) + j + pos.y;
									copyproc( src, _x, _y, r, g, b, overlay, rgbproc, srcrgbproc );
									set24( _x, _y, r, g, b );
								}
							}
							// all the center
							for( x=range.x; x<range.width; x++ )
							{
								src->get24( x, y, _r[0], _g[0], _b[0] );
								src->get24( x+1, y, _r[1], _g[1], _b[1] );
								src->get24( x, y+1, _r[2], _g[2], _b[2] );
								src->get24( x+1, y+1, _r[3], _g[3], _b[3] );
								for( j=0; j<zoomin; j++ )
								{
									for( i=0; i<zoomin; i++ )
									{
										k = zoomin - i;
										l = zoomin - j;
										d[0] = iparray[i+z*j];
										d[1] = iparray[k+z*j];
										d[2] = iparray[i+z*l];
										d[3] = iparray[k+z*l];
										n = d[0] + d[1] + d[2] + d[3];
										r = ( _r[0] * d[0] + _r[1] * d[1] + _r[2] * d[2] + _r[3] * d[3] ) / n;
										g = ( _g[0] * d[0] + _g[1] * d[1] + _g[2] * d[2] + _g[3] * d[3] ) / n;
										b = ( _b[0] * d[0] + _b[1] * d[1] + _b[2] * d[2] + _b[3] * d[3] ) / n;
										_x = ( x << resize ) + i + pos.x;
										_y = ( y << resize ) + j + pos.y;
										copyproc( src, _x, _y, r, g, b, overlay, rgbproc, srcrgbproc );
										set24( _x, _y, r, g, b );
									}
								}
							}
							// right border
							src->get24( x, y, _r[0], _g[0], _b[0] );
							src->get24( x, y+1, _r[2], _g[2], _b[2] );
							for( j=0; j<zoomin; j++ )
							{
								for( i=0; i<z2; i++ )
								{
									l = zoomin - j;
									d[0] = iparray[i+z*j];
									d[2] = iparray[i+z*l];
									n = d[0] + d[2];
									r = ( _r[0] * d[0] + _r[2] * d[2] ) / n;
									g = ( _g[0] * d[0] + _g[2] * d[2] ) / n;
									b = ( _b[0] * d[0] + _b[2] * d[2] ) / n;
									_x = ( x << resize ) + i + pos.x;
									_y = ( y << resize ) + j + pos.y;
									copyproc( src, _x, _y, r, g, b, overlay, rgbproc, srcrgbproc );
									set24( _x, _y, r, g, b );
								}
							}
						}
						x = range.x - 1;
						// lower left corner
						src->get24( x+1, y, r, g, b );
						for( j=0; j<z2; j++ )
						{
							for( i=z2; i<zoomin; i++ )
							{
								_x = ( x << resize ) + i + pos.x;
								_y = ( y << resize ) + j + pos.y;
								copyproc( src, _x, _y, r, g, b, overlay, rgbproc, srcrgbproc );
								set24( _x, _y, r, g, b );
							}
						}
						// lower border
						for( x=range.x; x<range.width; x++ )
						{
							src->get24( x, y, _r[0], _g[0], _b[0] );
							src->get24( x+1, y, _r[1], _g[1], _b[1] );
							for( j=0; j<z2; j++ )
							{
								for( i=0; i<zoomin; i++ )
								{
									k = zoomin - i;
									d[0] = iparray[i+z*j];
									d[1] = iparray[k+z*j];
									n = d[0] + d[1];
									r = ( _r[0] * d[0] + _r[1] * d[1] ) / n;
									g = ( _g[0] * d[0] + _g[1] * d[1] ) / n;
									b = ( _b[0] * d[0] + _b[1] * d[1] ) / n;
									_x = ( x << resize ) + i + pos.x;
									_y = ( y << resize ) + j + pos.y;
									copyproc( src, _x, _y, r, g, b, overlay, rgbproc, srcrgbproc );
									set24( _x, _y, r, g, b );
								}
							}
						}
						// lower right corner
						src->get24( x, y, r, g, b );
						for( j=0; j<z2; j++ )
						{
							for( i=0; i<z2; i++ )
							{
								_x = ( x << resize ) + i + pos.x;
								_y = ( y << resize ) + j + pos.y;
								copyproc( src, _x, _y, r, g, b, overlay, rgbproc, srcrgbproc );
								set24( _x, _y, r, g, b );
							}
						}
					}
				}
			} else {
				if( pos.x + range.width > size.x )
					range.width = size.x - pos.x;
				if( pos.y + range.height > size.y )
					range.height = size.y - pos.y;
				if( pos.x < 0 )
					range.x = -pos.x;
				if( pos.y < 0 )
					range.y = -pos.y;
				for( y=range.y; y<range.height; y++ )
				{
					for( x=range.x; x<range.width; x++ )
					{
						if( zoomout )
						{
							r = 0; g = 0; b = 0;
							for( v=0; v<zoomout; v++ )
							{
								for( u=0; u<zoomout; u++ )
								{
									src->get24( x*zoomout+u, y*zoomout+v, r1, g1, b1 );
									r += r1; g += g1; b += b1;
								}
							}
							r >>= z; g >>= z; b >>= z;
						} else {
							src->get24( x, y, r, g, b );
						}
						_x = x + pos.x;
						_y = y + pos.y;
						copyproc( src, _x, _y, r, g, b, overlay, rgbproc, srcrgbproc );
						set24( _x, _y, r, g, b );
					}
				}
			}
		}
		
		void copy24( dbpclass *src, pointclass pos, int resize = 0, int overlay = NONE )
		{
			rectclass range;
			int zoomout, zoomin;
			int x, y, u, v, r, g, b, r1, g1, b1;
			int _r[4], _g[4], _b[4], d[4], n;
			int i, j, k, l, z, _x, _y;

			range.set( pointclass( 0, 0 ), src->size );
			if( range.width + range.x > src->size.x )
				range.width = src->size.x - range.x;
			if( range.height + range.y > src->size.y )
				range.height = src->size.y - range.y;
			if( range.x > src->size.x )
				range.x = src->size.x;
			if( range.y > src->size.y )
				range.y = src->size.y;

			zoomout = 0;
			zoomin = 0;
			if( resize > 0 ) {
				zoomout = 1 << resize;
				range = range / zoomout;
				z = resize << 1;
			} else if( resize < 0 ) {
				zoomin = 1 << (-resize);
				range = range * zoomin;
			}

			if( resize < 0 )
			{
				range = range / zoomin;

				if( pos.x + range.width*zoomin > size.x )
					range.width = ( size.x - pos.x - zoomin ) / zoomin;
				if( pos.y + range.height*zoomin > size.y )
					range.height = ( size.y - pos.y - zoomin ) / zoomin;
				if( pos.x < 0 )
					range.x = ( -pos.x + zoomin - 1 ) / zoomin;
				if( pos.y < 0 )
					range.y = ( -pos.y + zoomin - 1 ) / zoomin;

				if( range.width > range.x && range.height > range.y )
				{
					range.setresize( pointclass( -1, -1 ) );
					pos = pos + zoomin / 2;
					resize = -resize;
					z = zoomin + 1;
					if( z != ipz && !heaplocked )
					{
						if( iparray )
							delete [] iparray;
						iparray = new int[z*z];
						ipz = z;
						for( y=0; y<z; y++ )
						{
							for( x=0; x<z; x++ )
							{
								iparray[x+y*z] =  (zoomin+1) * zoomin - x * x - y * y;
								if( iparray[x+y*z] < 0 )
									iparray[x+y*z] = 0;
							}
						}
					}
					if( z == ipz )
					{
						x = range.x - 1;
						y = range.y - 1;
						for( y=range.y; y<range.height; y++ )
						{
							for( x=range.x; x<range.width; x++ )
							{
								src->get24( x, y, _r[0], _g[0], _b[0] );
								src->get24( x+1, y, _r[1], _g[1], _b[1] );
								src->get24( x, y+1, _r[2], _g[2], _b[2] );
								src->get24( x+1, y+1, _r[3], _g[3], _b[3] );
								for( j=0; j<zoomin; j++ )
								{
									for( i=0; i<zoomin; i++ )
									{
										k = zoomin - i;
										l = zoomin - j;
										d[0] = iparray[i+z*j];
										d[1] = iparray[k+z*j];
										d[2] = iparray[i+z*l];
										d[3] = iparray[k+z*l];
										n = d[0] + d[1] + d[2] + d[3];
										r = ( _r[0] * d[0] + _r[1] * d[1] + _r[2] * d[2] + _r[3] * d[3] ) / n;
										g = ( _g[0] * d[0] + _g[1] * d[1] + _g[2] * d[2] + _g[3] * d[3] ) / n;
										b = ( _b[0] * d[0] + _b[1] * d[1] + _b[2] * d[2] + _b[3] * d[3] ) / n;
										_x = ( x << resize ) + i + pos.x;
										_y = ( y << resize ) + j + pos.y;

										if( overlay )
										{
											get24( _x, _y, r1, g1, b1 );
											if( overlay == SUBTRACT )
											{
												r = r1 - 0xff + r;
												g = g1 - 0xff + g;
												b = b1 - 0xff + b;
											} else {
												if( overlay == ADD )
												{
													r = r1 + r;
													g = g1 + g;
													b = b1 + b;
												} else { // AVERAGE
													r = ( r1 + r ) >> 1;
													g = ( g1 + g ) >> 1;
													b = ( b1 + b ) >> 1;
												}
											}
											src->inrange24( r, g, b );
										}

										set24( _x, _y, r, g, b );
									}
								}
							}
						}
					}
				}
			} else {
				if( pos.x + range.width > size.x )
					range.width = size.x - pos.x;
				if( pos.y + range.height > size.y )
					range.height = size.y - pos.y;
				if( pos.x < 0 )
					range.x = -pos.x;
				if( pos.y < 0 )
					range.y = -pos.y;
				if( zoomout )
				{
					for( y=range.y; y<range.height; y++ )
					{
						for( x=range.x; x<range.width; x++ )
						{
							r = 0; g = 0; b = 0;
							for( v=0; v<zoomout; v++ )
							{
								for( u=0; u<zoomout; u++ )
								{
									src->get24( x*zoomout+u, y*zoomout+v, r1, g1, b1 );
									r += r1; g += g1; b += b1;
								}
							}
							r >>= z; g >>= z; b >>= z;
							_x = x + pos.x;
							_y = y + pos.y;
							if( overlay )
							{
								get24( _x, _y, r1, g1, b1 );
								if( overlay == SUBTRACT )
								{
									r = r1 - 0xff + r;
									g = g1 - 0xff + g;
									b = b1 - 0xff + b;
								} else {
									if( overlay == ADD )
									{
										r = r1 + r;
										g = g1 + g;
										b = b1 + b;
									} else { // AVERAGE
										r = ( r1 + r ) >> 1;
										g = ( g1 + g ) >> 1;
										b = ( b1 + b ) >> 1;
									}
								}
								src->inrange24( r, g, b );
							}
							set24( _x, _y, r, g, b );
						}
					}
				} else {
					if( overlay )
					{
						if( overlay == SUBTRACT )
						{
							for( y=range.y; y<range.height; y++ )
							{
								for( x=range.x; x<range.width; x++ )
								{
									src->get24( x, y, r, g, b );
									_x = x + pos.x;
									_y = y + pos.y;
									get24( _x, _y, r1, g1, b1 );
									r = r1 - 0xff + r;
									g = g1 - 0xff + g;
									b = b1 - 0xff + b;
									src->inrange24( r, g, b );
									set24( _x, _y, r, g, b );
								}
							}
						} else {
							if( overlay == ADD )
							{
								for( y=range.y; y<range.height; y++ )
								{
									for( x=range.x; x<range.width; x++ )
									{
										src->get24( x, y, r, g, b );
										_x = x + pos.x;
										_y = y + pos.y;
										get24( _x, _y, r1, g1, b1 );
										r = r1 + r;
										g = g1 + g;
										b = b1 + b;
										src->inrange24( r, g, b );
										set24( _x, _y, r, g, b );
									}
								}
							} else { // AVERAGE
								for( y=range.y; y<range.height; y++ )
								{
									for( x=range.x; x<range.width; x++ )
									{
										src->get24( x, y, r, g, b );
										_x = x + pos.x;
										_y = y + pos.y;
										get24( _x, _y, r1, g1, b1 );
										r = ( r1 + r ) >> 1;
										g = ( g1 + g ) >> 1;
										b = ( b1 + b ) >> 1;
										src->inrange24( r, g, b );
										set24( _x, _y, r, g, b );
									}
								}
							}
						}
					} else { // NONE
						for( y=range.y; y<range.height; y++ )
						{
							for( x=range.x; x<range.width; x++ )
							{
								src->get24( x, y, r, g, b );
								_x = x + pos.x;
								_y = y + pos.y;
								set24( _x, _y, r, g, b );
							}
						}
					}
				}
			}
		}
		
		
	protected:
		inline void copyproc( dbpclass *src, const int &x, const int &y, int &r, int &g, int &b, const int &overlay, void (*rgbproc)( int &r, int &g, int &b ), void (*srcrgbproc)( int &r, int &g, int &b ) )
		{
			int r1, g1, b1;
			if( rgbproc )
				(*rgbproc)( r, g, b );
			if( overlay )
			{
				get24( x, y, r1, g1, b1 );
				if( srcrgbproc )
					(*srcrgbproc)( r1, g1, b1 );
				if( overlay == SUBTRACT )
				{
					r = r1 - 0xff + r;
					g = g1 - 0xff + g;
					b = b1 - 0xff + b;
				} else {
					if( overlay == ADD )
					{
						r = r1 + r;
						g = g1 + g;
						b = b1 + b;
					} else { // AVERAGE
						r = ( r1 + r ) >> 1;
						g = ( g1 + g ) >> 1;
						b = ( b1 + b ) >> 1;
					}
				}
				src->inrange24( r, g, b );
			}
		}

	public:

		void copy( dbpclass *src, pointclass pos = pointclass( 0, 0 ), int lgzoom = 0, void (*rgbproc)( int &r, int &g, int &b ) = null )
		{
			rectclass range;
			range.set( pointclass( 0, 0 ), src->size );
			copy( src, pos, range, lgzoom, rgbproc );
		}

		void copy( dbpclass *src, pointclass pos, rectclass range, int lgzoom = 0, void (*rgbproc)( int &r, int &g, int &b ) = null )
		{
			int x, y, u, v, r1, g1, b1, r, g, b, zoomout, z;
			if( range.width + range.x > src->size.x )
				range.width = src->size.x - range.x;
			if( range.height + range.y > src->size.y )
				range.height = src->size.y - range.y;
			if( range.x > src->size.x )
				range.x = src->size.x;
			if( range.y > src->size.y )
				range.y = src->size.y;

			if( lgzoom > 0 ) {
				zoomout = 1 << lgzoom;
				range = range / zoomout;
				z = lgzoom << 1;
			} else {
				zoomout = 0;
			}

			if( pos.x + range.width > size.x )
				range.width = size.x - pos.x;
			if( pos.y + range.height > size.y )
				range.height = size.y - pos.y;
			if( pos.x < 0 )
				range.x = -pos.x;
			if( pos.y < 0 )
				range.y = -pos.y;
			
			if( zoomout )
			{
				for( y=range.y; y<range.height; y++ )
				{
					for( x=range.x; x<range.width; x++ )
					{
						r = 0; g = 0; b = 0;
						for( v=0; v<zoomout; v++ )
						{
							for( u=0; u<zoomout; u++ )
							{
								src->get( x*zoomout+u, y*zoomout+v, r1, g1, b1 );
								r += r1; g += g1; b += b1;
							}
						}
						r >>= z; g >>= z; b >>= z;
						if( rgbproc )
							(*rgbproc)( r, g, b );
						else
							convert( src, r, g, b );
						set( x+pos.x, y+pos.y, r, g, b );
					}
				}
			} else {
				for( y=range.y; y<range.height; y++ )
				{
					for( x=range.x; x<range.width; x++ )
					{
						src->get( x, y, r, g, b );
						if( rgbproc )
							(*rgbproc)( r, g, b );
						else
							convert( src, r, g, b );
						set( x+pos.x, y+pos.y, r, g, b );
					}
				}
			}
		}

		void copyident( dbpclass *src, pointclass pos, int lgzoom = 0 )
		{
			int x, y, u, v, r1, g1, b1, r, g, b, zoomout, z;
			rectclass range;
			range.set( pointclass( 0, 0 ), src->size );
			if( range.width + range.x > src->size.x )
				range.width = src->size.x - range.x;
			if( range.height + range.y > src->size.y )
				range.height = src->size.y - range.y;
			if( range.x > src->size.x )
				range.x = src->size.x;
			if( range.y > src->size.y )
				range.y = src->size.y;

			if( lgzoom > 0 ) {
				zoomout = 1 << lgzoom;
				range = range / zoomout;
				z = lgzoom << 1;
			} else {
				zoomout = 0;
			}

			if( pos.x + range.width > size.x )
				range.width = size.x - pos.x;
			if( pos.y + range.height > size.y )
				range.height = size.y - pos.y;
			if( pos.x < 0 )
				range.x = -pos.x;
			if( pos.y < 0 )
				range.y = -pos.y;
			
			if( zoomout )
			{
				for( y=range.y; y<range.height; y++ )
				{
					for( x=range.x; x<range.width; x++ )
					{
						r = 0; g = 0; b = 0;
						for( v=0; v<zoomout; v++ )
						{
							for( u=0; u<zoomout; u++ )
							{
								src->get( x*zoomout+u, y*zoomout+v, r1, g1, b1 );
								r += r1; g += g1; b += b1;
							}
						}
						r >>= z; g >>= z; b >>= z;
						set( x+pos.x, y+pos.y, r, g, b );
					}
				}
			} else {
				for( y=range.y; y<range.height; y++ )
				{
					for( x=range.x; x<range.width; x++ )
					{
						src->get( x, y, r, g, b );
						set( x+pos.x, y+pos.y, r, g, b );
					}
				}
			}
		}

		void move( dbpclass *src, pointclass dstpos, rectclass srcrange )
		{
			int x, y, r, g, b;
			pointclass srcb, srce;
			
			srcb = srcrange.origin();
			srce = srcrange.lowerright();
			
			if( dstpos.x + srce.x > size.x )
				srce.x = size.x - dstpos.x;
			if( dstpos.y + srce.y > size.y )
				srce.y = size.y - dstpos.y;
			if( dstpos.x < 0 )
				srcb.x = -dstpos.x;
			if( dstpos.y < 0 )
				srcb.y = -dstpos.y;
			
			if( dstpos.x + srcb.x < 0 )
				srcb.x = -dstpos.x;
			if( dstpos.y + srcb.y < 0 )
				srcb.y = -dstpos.y;
			
			if( dstpos.y < 0 )
			{
				if( dstpos.x < 0 )
				{
					for( y=srcb.y; y<srce.y; y++ )
					{
						for( x=srcb.x; x<srce.x; x++ )
						{
							src->get( x, y, r, g, b );
							set( x+dstpos.x, y+dstpos.y, r, g, b );
						}
					}
				} else {
					srce.x--;
					srcb.x--;
					for( y=srcb.y; y<srce.y; y++ )
					{
						for( x=srce.x; x>srcb.x; x-- )
						{
							src->get( x, y, r, g, b );
							set( x+dstpos.x, y+dstpos.y, r, g, b );
						}
					}
				}
			} else {
				srce.y--;
				srcb.y--;
				if( dstpos.x < 0 )
				{
					for( y=srce.y; y>srcb.y; y-- )
					{
						for( x=srcb.x; x<srce.x; x++ )
						{
							src->get( x, y, r, g, b );
							set( x+dstpos.x, y+dstpos.y, r, g, b );
						}
					}
				} else {
					srce.x--;
					srcb.x--;
					for( y=srce.y; y>srcb.y; y-- )
					{
						for( x=srce.x; x>srcb.x; x-- )
						{
							src->get( x, y, r, g, b );
							set( x+dstpos.x, y+dstpos.y, r, g, b );
						}
					}
				}
			}
		}

		void moveident( dbpclass *src, pointclass dstpos, rectclass srcrange )
		{
			int y, srclinebeg, srclinelength;
			pointclass srcb, srce;
			
			srcb = srcrange.origin();
			srce = srcrange.lowerright();
			
			if( dstpos.x + srce.x > size.x )
				srce.x = size.x - dstpos.x;
			if( dstpos.y + srce.y > size.y )
				srce.y = size.y - dstpos.y;
			if( dstpos.x < 0 )
				srcb.x = -dstpos.x;
			if( dstpos.y < 0 )
				srcb.y = -dstpos.y;
			
			if( dstpos.x + srcb.x < 0 )
				srcb.x = -dstpos.x;
			if( dstpos.y + srcb.y < 0 )
				srcb.y = -dstpos.y;
			
			if( dstpos.y < 0 )
			{
				if( dstpos.x < 0 )
				{
					y = srcb.y;
					if( y < srce.y )
					{
						srclinebeg = src->pos( srcb.x, y );
						srclinelength = src->pos( srce.x, y ) + bytesperpixel - srclinebeg;
						if( srclinelength > 0 )
						{
							cpy( img + pos( srcb.x+dstpos.x, y+dstpos.y ), src->img + srclinebeg, srclinelength );
							for( y++; y<srce.y; y++ )
							{
								srclinebeg = src->pos( srcb.x, y );
								cpy( img + pos( srcb.x+dstpos.x, y+dstpos.y ), src->img + srclinebeg, srclinelength );
							}
						}
					}
				} else {
					srce.x--;
					srcb.x--;
					y = srcb.y;
					if( y < srce.y )
					{
						srclinebeg = src->pos( srcb.x, y );
						srclinelength = src->pos( srce.x, y ) + bytesperpixel - srclinebeg;
						cpyr( img + pos( srcb.x+dstpos.x, y+dstpos.y ), src->img + srclinebeg, srclinelength );
						if( srclinelength > 0 )
						{
							for( y++; y<srce.y; y++ )
							{
								srclinebeg = src->pos( srcb.x, y );
								cpyr( img + pos( srcb.x+dstpos.x, y+dstpos.y ), src->img + srclinebeg, srclinelength );
							}
						}
					}
				}
			} else {
				srce.y--;
				srcb.y--;
				if( dstpos.x < 0 )
				{
					y = srce.y;
					if( y > srcb.y )
					{
						srclinebeg = src->pos( srcb.x, y );
						srclinelength = src->pos( srce.x, y ) + bytesperpixel - srclinebeg;
						if( srclinelength > 0 )
						{
							cpy( img + pos( srcb.x+dstpos.x, y+dstpos.y ), src->img + srclinebeg, srclinelength );
							for( y--; y>srcb.y; y-- )
							{
								srclinebeg = src->pos( srcb.x, y );
								cpy( img + pos( srcb.x+dstpos.x, y+dstpos.y ), src->img + srclinebeg, srclinelength );
							}
						}
					}
				} else {
					srce.x--;
					srcb.x--;
					y = srce.y;
					if( y > srcb.y )
					{
						srclinebeg = src->pos( srcb.x, y );
						srclinelength = src->pos( srce.x, y ) + bytesperpixel - srclinebeg;
						if( srclinelength > 0 )
						{
							cpy( img + pos( srcb.x+dstpos.x, y+dstpos.y ), src->img + srclinebeg, srclinelength );
							for( y--; y>srcb.y; y-- )
							{
								srclinebeg = src->pos( srcb.x, y );
								cpyr( img + pos( srcb.x+dstpos.x, y+dstpos.y ), src->img + srclinebeg, srclinelength );
							}
						}
					}
				}
			}
		}

		// simple painting methods
		
		inline void line( rectclass coords, void (*rgbproc)( int &r, int &g, int &b ) = null )
		{
			int x, y, dir, r, g, b;

			if( coords.width*coords.width > coords.height*coords.height )
			{
				if( coords.width < 0 )
					dir = -1;
				else
					dir = 1;
				for( x=0; x*dir<coords.width*dir; x+=dir )
				{
					y = x*coords.height/coords.width;
					if( rectclass( pointclass(0,0), size ) == pointclass( x+coords.x, y+coords.y ) )
					{
						get( x+coords.x, y+coords.y, r, g, b );
						if( rgbproc ) {
							(*rgbproc)( r, g, b );
						} else {
							r = 0; g = 0; b = 0;
						}
						set( x+coords.x, y+coords.y, r, g, b );
					}
				}
			} else {
				if( coords.height < 0 )
					dir = -1;
				else
					dir = 1;
				for( y=0; y*dir<coords.height*dir; y+=dir )
				{
					x = y*coords.width/coords.height;
					if( rectclass( pointclass(0,0), size ) == pointclass( x+coords.x, y+coords.y ) )
					{
						get( x+coords.x, y+coords.y, r, g, b );
						if( rgbproc ) {
							(*rgbproc)( r, g, b );
						} else {
							r = 0; g = 0; b = 0;
						}
						set( x+coords.x, y+coords.y, r, g, b );
					}
				}
			}
		}

		template <typename f_t = double>
		inline void pxlb14( f_t x, f_t y, f_t dx, f_t dy,
		                    void (*rgbproc)( int &r, int &g, int &b, void * ptr ),
		                    void * ptr = nullptr )
		{
			static constexpr f_t mind = 0.0;
			if( dx < mind )
				dx = mind;
			if( dy < mind )
				dy = mind;
			f_t dxs2 = dx * (f_t)SQR2H;
			f_t dys2 = dy * (f_t)SQR2H;
			f_t mdx = b14_max( dxs2 );
			f_t mdy = b14_max( dys2 );
			int xbeg = (int)( x - mdx );
			int xend = (int)( x + mdx ) + 1;
			int ybeg = (int)( y - mdy );
			int yend = (int)( y + mdy ) + 1;
			if( xbeg < 0 )
				xbeg = 0;
			if( ybeg < 0 )
				ybeg = 0;
			if( xend > size.x )
				xend = size.x;
			if( yend > size.y )
				yend = size.y;
			for( int iy = ybeg; iy < yend; iy++ )
			{
				f_t a_2 = b14<f_t>( d_abs<f_t>( y - (f_t)iy ) / dys2 );
				for( int ix = xbeg; ix < xend; ix++ )
				{
					f_t a_1 = b14<f_t>( d_abs<f_t>( x - (f_t)ix ) / dxs2 );
					if( a_1 > (f_t)0 && a_2 > (f_t)0 )
					{
						f_t a = a_1 * a_2;
						f_t a1 = (f_t )1 - a;
						int r, g, b;
						get( ix, iy, r, g, b );
						int r1 = r;
						int g1 = g;
						int b1 = b;
						(*rgbproc)( r1, g1, b1, ptr );
						r = (int)( a * r1 + a1 * r );
						g = (int)( a * g1 + a1 * g );
						b = (int)( a * b1 + a1 * b );
						set( ix, iy, r, g, b );
					}
				}
			}
		}
		
		template <typename f_t = double>
		inline void hlineb14( f_t y, f_t dy, int xbeg, int xend,
		                      void (*rgbproc)( int &r, int &g, int &b, void * ptr ),
		                      void * ptr = nullptr )
		{
			static constexpr f_t mind = 0.0;
			if( dy < mind )
				dy = mind;
			f_t dys2 = dy * (f_t)SQR2H;
			f_t mdy = b14_max( dys2 );
			int ybeg = (int)( y - mdy );
			int yend = (int)( y + mdy ) + 1;
			if( xbeg < 0 )
				xbeg = 0;
			if( ybeg < 0 )
				ybeg = 0;
			if( xend > size.x )
				xend = size.x;
			if( yend > size.y )
				yend = size.y;
			for( int iy = ybeg; iy < yend; iy++ )
			{
				f_t a = b14<f_t>( d_abs<f_t>( y - (f_t)iy ) / dys2 );
				if( a > (f_t)0 )
				{
					f_t a1 = (f_t )1 - a;
					for( int ix = xbeg; ix < xend; ix++ )
					{
						int r, g, b;
						get( ix, iy, r, g, b );
						int r1 = r;
						int g1 = g;
						int b1 = b;
						(*rgbproc)( r1, g1, b1, ptr );
						r = (int)( a * r1 + a1 * r );
						g = (int)( a * g1 + a1 * g );
						b = (int)( a * b1 + a1 * b );
						set( ix, iy, r, g, b );
					}
				}
			}
		}
		
		template <typename f_t = double>
		inline void vlineb14( f_t x, f_t dx, int ybeg, int yend,
		                      void (*rgbproc)( int &r, int &g, int &b, void * ptr ),
		                      void * ptr = nullptr )
		{
			static constexpr f_t mind = 0.0;
			if( dx < mind )
				dx = mind;
			f_t dxs2 = dx * (f_t)SQR2H;
			f_t mdx = b14_max( dxs2 );
			int xbeg = (int)( x - mdx );
			int xend = (int)( x + mdx ) + 1;
			if( xbeg < 0 )
				xbeg = 0;
			if( ybeg < 0 )
				ybeg = 0;
			if( xend > size.x )
				xend = size.x;
			if( yend > size.y )
				yend = size.y;
			for( int iy = ybeg; iy < yend; iy++ )
			{
				for( int ix = xbeg; ix < xend; ix++ )
				{
					f_t a = b14<f_t>( d_abs<f_t>( x - (f_t)ix ) / dxs2 );
					if( a > (f_t)0 )
					{
						f_t a1 = (f_t )1 - a;
						int r, g, b;
						get( ix, iy, r, g, b );
						int r1 = r;
						int g1 = g;
						int b1 = b;
						(*rgbproc)( r1, g1, b1, ptr );
						r = (int)( a * r1 + a1 * r );
						g = (int)( a * g1 + a1 * g );
						b = (int)( a * b1 + a1 * b );
						set( ix, iy, r, g, b );
					}
				}
			}
		}
		
		/*
		template <typename f_t = double>
		inline void pxlb14( f_t x, f_t y, f_t dx, f_t dy,
		                    void (*rgbproc)( int &r, int &g, int &b ) = null )
		{
			int xbeg, xend, ybeg, yend, ix, iy, r, g, b, r1, g1, b1;
			double mdx, mdy, a, a1, a_1, a_2;
			if( dx < (f_t)0.5 )
				dx = (f_t)0.5;
			if( dy < (f_t)0.5 )
				dy = (f_t)0.5;
			mdx = dx * 3.5 + 0.55;
			mdy = dy * 3.5 + 0.55;
			xbeg = (int)(x-mdx); xend = (int)(x+mdx) + 1;
			ybeg = (int)(y-mdy); yend = (int)(y+mdy) + 1;
			if( xbeg < 0 ) xbeg = 0;
			if( ybeg < 0 ) ybeg = 0;
			if( xend > size.x ) xend = size.x;
			if( yend > size.y ) yend = size.y;
			for( iy=ybeg; iy<yend; iy++ )
			{
				for( ix=xbeg; ix<xend; ix++ )
				{
					a_1 = b14( d_abs(x-ix)/dx/SQR2H );
					a_2 = b14( d_abs(y-iy)/dy/SQR2H );
					if( a_1 > 0 && a_2 > 0 )
					{
						a = a_1 * a_2;
						a1 = 1-a;
						get( ix, iy, r, g, b );
						r1 = r;
						g1 = g;
						b1 = b;
						if( rgbproc ) {
							(*rgbproc)( r1, g1, b1 );
						} else {
							r1 = 0; g1 = 0; b1 = 0;
						}
						r = (int)( a*r1 + a1*r );
						g = (int)( a*g1 + a1*g );
						b = (int)( a*b1 + a1*b );
						set( ix, iy, r, g, b );
					}
				}
			}
		}
		*/
		/*
		inline void pxlb14( const double &x, const double &y, double dx, double dy,
		                    void (*rgbproc)( int &r, int &g, int &b ),
		                    const rectclass &excluderect )
		{
			int xbeg, xend, ybeg, yend, ix, iy, r, g, b, r1, g1, b1;
			double mdx, mdy, a, a1, a_1, a_2;
			if( dx < 0.5 )
				dx = 0.5;
			if( dy < 0.5 )
				dy = 0.5;
			mdx = dx * 3.5 + 0.55;
			mdy = dy * 3.5 + 0.55;
			xbeg = (int)(x-mdx); xend = (int)(x+mdx) + 1;
			ybeg = (int)(y-mdy); yend = (int)(y+mdy) + 1;
			if( xbeg < 0 ) xbeg = 0;
			if( ybeg < 0 ) ybeg = 0;
			if( xend > size.x ) xend = size.x;
			if( yend > size.y ) yend = size.y;
			for( iy=ybeg; iy<yend; iy++ )
			{
				for( ix=xbeg; ix<xend; ix++ )
				{
					if( excluderect != pointclass( ix, iy ) )
					{
						a_1 = b14( d_abs(x-ix)/dx/SQR2H );
						a_2 = b14( d_abs(y-iy)/dy/SQR2H );
						if( a_1 > 0 && a_2 > 0 )
						{
							a = a_1 * a_2;
							a1 = 1-a;
							get( ix, iy, r, g, b );
							r1 = r;
							g1 = g;
							b1 = b;
							(*rgbproc)( r1, g1, b1 );
							r = (int)( a*r1 + a1*r );
							g = (int)( a*g1 + a1*g );
							b = (int)( a*b1 + a1*b );
							set( r, g, b );
						}
					}
				}
			}
		}
		*/

/*		
		template <typename f_t = double>
		inline void lineb14_1d( int xbeg, int xend, double y, double dy, void (*rgbproc)( int &r, int &g, int &b ) = null )
		{
			int ybeg, yend, ix, iy, r, g, b, r1, g1, b1;
			double mdy, a, a1, a_1;
			if( dy < 0.5 )
				dy = 0.5;
			mdy = dy * 3.5 + 0.55;
			ybeg = (int)(y-mdy); yend = (int)(y+mdy) + 1;
			if( ybeg < 0 ) ybeg = 0;
			if( yend > size.y ) yend = size.y;
			for( iy=ybeg; iy<yend; iy++ )
			{
				a = b14( d_abs(y-iy)/dy/SQR2H );
				if( a > 0 )
				{
					a1 = 1-a;
					for( ix=xbeg; ix<xend; ix++ )
					{
						get( ix, iy, r, g, b );
						r1 = r;
						g1 = g;
						b1 = b;
						if( rgbproc ) {
							(*rgbproc)( r1, g1, b1 );
						} else {
							r1 = 0; g1 = 0; b1 = 0;
						}
						r = (int)( a*r1 + a1*r );
						g = (int)( a*g1 + a1*g );
						b = (int)( a*b1 + a1*b );
						set( ix, iy, r, g, b );
					}
				}
			}
		}
		template <typename f_t = double>
		inline void lineb14_1d( double x, double dx, int ybeg, int yend, void (*rgbproc)( int &r, int &g, int &b ) = null )
		{
			int xbeg, xend, ix, iy, r, g, b, r1, g1, b1;
			double mdx, a, a1, a_1;
			if( dx < 0.5 )
				dx = 0.5;
			mdx = dx * 3.5 + 0.55;
			xbeg = (int)(x-mdx); xend = (int)(x+mdx) + 1;
			if( xbeg < 0 ) xbeg = 0;
			if( xend > size.x ) xend = size.x;
			for( ix=xbeg; ix<xend; ix++ )
			{
				a = b14( d_abs(x-ix)/dx/SQR2H );
				if( a > 0 )
				{
					a1 = 1-a;
					for( iy=ybeg; iy<yend; iy++ )
					{
						get( ix, iy, r, g, b );
						r1 = r;
						g1 = g;
						b1 = b;
						if( rgbproc ) {
							(*rgbproc)( r1, g1, b1 );
						} else {
							r1 = 0; g1 = 0; b1 = 0;
						}
						r = (int)( a*r1 + a1*r );
						g = (int)( a*g1 + a1*g );
						b = (int)( a*b1 + a1*b );
						set( ix, iy, r, g, b );
					}
				}
			}
		}
		
		inline void lineb14( rectclass coords, double dx, double dy, void (*rgbproc)( int &r, int &g, int &b, int &a ) )
		{
			double length, x, y, width, height, opacity, steps;
			int  i, iopacity, isteps;
			int a;
			pointclass p;
			
			x = coords.x;
			y = coords.y;
			width = coords.width;
			height = coords.height;
			
			pxlb14( x, y, dx, dy, rgbproc, 0xff );
			
			if( d_abs( coords.width ) > 0 || d_abs( coords.height ) > 0 )
			{
				length = sqrt15( width, height );
				if( length > 1.0 )
				{
					if( length > 2.0 )
					{
						
						steps = length - 1.0;
						isteps = (int)steps;
						if( steps > isteps )
							isteps++;
						opacity = steps / isteps;
						iopacity = (int)(opacity*255.0);
						
						for( i=0; i<isteps; i++ )
						{
							pxlb14( 
									x + ( i + 0.5 * ( 1.0 + opacity ) ) / length * opacity * width, 
									y + ( i + 0.5 * ( 1.0 + opacity ) ) / length * opacity * height, 
								dx, dy, rgbproc, iopacity );
						}
					} else {
						a = (int)(length*0xff);
						pxlb14( x+width/2, y+height/2, dx, dy, rgbproc, a );
					}
				}
			}
		}
*/
		
		template <typename f_t = double>
		inline void lineb14( const doublerectclass &coords, double &length, const double &delta, void (*rgbproc)( int &r, int &g, int &b ), const rectclass &excluderect )
		{
			f_t x, y, width, height, opacity, steps, stepwidth;
			int  i, isteps;
			
			x = coords.x;
			y = coords.y;
			width = coords.width;
			height = coords.height;
			
			pxlb14( x, y, delta, delta, rgbproc, excluderect );
			if( !length )
				length = sqrt15( width, height );
			if( length > delta )
			{
				stepwidth = delta;
				steps = ( length - (f_t)1.0 ) / stepwidth;
				isteps = (int)steps;
				if( steps > isteps )
					isteps++;
				opacity = steps / isteps;
				for( i=0; i<isteps; i++ )
				{
					pxlb14( x + ( i + (f_t)0.5 * ( (f_t)1.0 + opacity ) ) / length * stepwidth * opacity * width, 
							y + ( i + (f_t)0.5 * ( (f_t)1.0 + opacity ) ) / length * stepwidth * opacity * height, 
							delta, delta, rgbproc, excluderect );
				}
			} else {
				pxlb14( x+width*0.5, y+height*0.5, delta, delta, rgbproc, excluderect );
			}
		}


#endif // DB_DBPCLASS_PAINT_INCLUDED
