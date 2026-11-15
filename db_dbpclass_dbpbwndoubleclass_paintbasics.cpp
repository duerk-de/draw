__GIT_SYMLINKS__
target=../_inc/db_dbpclass_dbpbwndoubleclass_paintbasics.cpp
target_resolved=/dibom/src/c/_inc/db_dbpclass_dbpbwndoubleclass_paintbasics.cpp
kind=file
sha256=85f6696398eb195aab504a9b4f03085961d2d6d37b2c19c98a9510b81e35d890
size=1260
created=2026-11-15T11:20:55.353584
--END-HEADER--
#ifndef DB_DBPCLASS_DBPBWNDOUBLECLASS_PAINTBASICS_INCLUDED
#define DB_DBPCLASS_DBPBWNDOUBLECLASS_PAINTBASICS_INCLUDED


		inline void setb14( const double &x, const double &y, double dx, double dy, const double &r, const double &g, const double &b )
		{
			int xbeg, xend, ybeg, yend, ix, iy;
			double mdx, mdy, a, a1, a2, r1, g1, b1, n1;
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
					a1 = b14( d_abs(x-ix)/dx/SQR2H );
					a2 = b14( d_abs(y-iy)/dy/SQR2H );
					if( a1 > 0 && a2 > 0 )
					{
						a = ( a1 * a2 ) / ( dx * dy );
						if( a > 0 )
						{
							get( ix, iy, r1, g1, b1, n1 );
							if( channels == 2 )
								set( ix, iy, (a*r)+r1, g1, b1, a+n1 );
							else
								set( ix, iy, (a*r)+r1, (a*g)+g1, (a*b)+b1, a+n1 );
						}
					}
				}
			}
		}


#endif // DB_DBPCLASS_DBPBWNDOUBLECLASS_PAINTBASICS_INCLUDED
