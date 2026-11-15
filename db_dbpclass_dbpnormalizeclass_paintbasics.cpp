__GIT_SYMLINKS__
target=../_inc/db_dbpclass_dbpnormalizeclass_paintbasics.cpp
target_resolved=/dibom/src/c/_inc/db_dbpclass_dbpnormalizeclass_paintbasics.cpp
kind=file
sha256=5ef572dedb6ab5de2f9a00d4b0bc81fdfc697ebe936a4b8a394fc7df6cc84626
size=4090
created=2026-11-15T11:20:55.355943
--END-HEADER--
#ifndef DB_DBPCLASS_DBPNORMALIZECLASS_PAINTBASICS_INCLUDED
#define DB_DBPCLASS_DBPNORMALIZECLASS_PAINTBASICS_INCLUDED


		inline void setexp( const double &x, const double &y, double dx, double dy, const int &r, const int &g, const int &b, double corr = 2.55 )
		{
			int xbeg, xend, ybeg, yend, ix, iy, r1, g1, b1, n1;
			double mdx, mdy, a, ax, ay;
			const static double prec = 1e-4;
			if( dx < 0.5 )
				dx = 0.5;
			if( dy < 0.5 )
				dy = 0.5;
			mdx = dx * 3.5 + 0.55;
			mdy = dy * 3.5 + 0.55;
			xbeg = (int)(x-mdx); xend = (int)(x+mdx) + 1;
			ybeg = (int)(y-mdy); yend = (int)(y+mdy) + 1;
			if( xbeg < 0) xbeg = 0;
			if( ybeg < 0) ybeg = 0;
			if( xend > size.x ) xend = size.x;
			if( yend > size.y ) yend = size.y;
			for( iy=ybeg; iy<yend; iy++ )
			{
				for( ix=xbeg; ix<xend; ix++ )
				{
					ax = (x-ix)/(dx*SQR2H);
					ax = d_exp( -ax*ax, prec );
					ay = (y-iy)/(dx*SQR2H);
					ay = d_exp( -ay*ay, prec );
					a = ( ax * ay ) / ( dx * dy ) * corr;
					if( a > 0 )
					{
						get( ix, iy, r1, g1, b1, n1 );
						if( channels == 2 )
							set( ix, iy, (int)(a*r)+r1, g1, b1, (int)(a*norm)+n1 );
						else
							set( ix, iy, (int)(a*r)+r1, (int)(a*g)+g1, (int)(a*b)+b1, (int)(a*norm)+n1 );
					}
				}
			}
		}
		
		inline void setb14( const double &x, const double &y, double dx, double dy, const int &r, const int &g, const int &b, double corr = 2.55 )
		{
			int xbeg, xend, ybeg, yend, ix, iy, r1, g1, b1, n1;
			double mdx, mdy, a, a1, a2;
			// const static double prec = 1e-4;
			if( dx < 0.5 )
				dx = 0.5;
			if( dy < 0.5 )
				dy = 0.5;
			mdx = dx * 3.5 + 0.55;
			mdy = dy * 3.5 + 0.55;
			xbeg = (int)(x-mdx); xend = (int)(x+mdx) + 1;
			ybeg = (int)(y-mdy); yend = (int)(y+mdy) + 1;
			if( xbeg < 0) xbeg = 0;
			if( ybeg < 0) ybeg = 0;
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
						a = ( a1 * a2 ) / ( dx * dy ) * corr;
						if( a > 0 )
						{
							get( ix, iy, r1, g1, b1, n1 );
							if( channels == 2 )
								set( ix, iy, (int)(a*r)+r1, g1, b1, (int)(a*norm)+n1 );
							else
								set( ix, iy, (int)(a*r)+r1, (int)(a*g)+g1, (int)(a*b)+b1, (int)(a*norm)+n1 );
						}
					}
				}
			}
		}
		
		inline void setb10( const double &x, const double &y, double dx, double dy, const int &r, const int &g, const int &b, double corr = 2.55 )
		{
			int xbeg, xend, ybeg, yend, ix, iy, r1, g1, b1, n1;
			double mdx, mdy, a, a1, a2;
			if( dx < 0.5 )
				dx = 0.5;
			if( dy < 0.5 )
				dy = 0.5;
			mdx = dx * 2.6171504655 + 0.55;
			mdy = dy * 2.6171504655 + 0.55;
			xbeg = (int)(x-mdx); xend = (int)(x+mdx) + 1;
			ybeg = (int)(y-mdy); yend = (int)(y+mdy) + 1;
			if( xbeg < 0) xbeg = 0;
			if( ybeg < 0) ybeg = 0;
			if( xend > size.x ) xend = size.x;
			if( yend > size.y ) yend = size.y;
			for( iy=ybeg; iy<yend; iy++ )
			{
				for( ix=xbeg; ix<xend; ix++ )
				{
					a1 = b10( d_abs(x-ix)/dx/SQR2H );
					a2 = b10( d_abs(y-iy)/dy/SQR2H );
					if( a1 > 0 && a2 > 0 )
					{
						a = ( a1 * a2 ) / ( dx * dy ) * corr;
						if( a > 0 )
						{
							get( ix, iy, r1, g1, b1, n1 );
							if( channels == 2 )
								set( ix, iy, (int)(a*r)+r1, g1, b1, (int)(a*norm)+n1 );
							else
								set( ix, iy, (int)(a*r)+r1, (int)(a*g)+g1, (int)(a*b)+b1, (int)(a*norm)+n1 );
						}
					}
				}
			}
		}

		void copyb10( dbpclass *src, doublerectclass pos )
		{
			int x, y, r, g, b;
			double dx, dy;
			dx = pos.width/src->size.x * 0.7;
			dy = pos.height/src->size.y * 0.7;
			for( y=0; y<src->size.y; y++ )
			{
				for( x=0; x<src->size.x; x++ )
				{
					src->get24( x, y, r, g, b );
					setb10( x*pos.width/src->size.x+pos.x, y*pos.height/src->size.y+pos.y, dx, dy, r, g, b );
				}
			}
		}


#endif // DB_DBPCLASS_DBPNORMALIZECLASS_PAINTBASICS_INCLUDED
