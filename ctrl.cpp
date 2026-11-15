#ifdef CTRL_INCLUDING
#error  CTRL_INCLUDING
#endif //  CTRL_INCLUDING
#define CTRL_INCLUDING
#ifndef CTRL_INCLUDED
#define CTRL_INCLUDED

#include "db_consoleio.cpp"
#include "backtrace.cpp"
#include <stdlib.h>

#define ENSURE( expr, ... )                  \
	{                                        \
		if( !( expr ) )                      \
		{                                    \
			d_fprint_err( __VA_ARGS__ );     \
			bt::print( __FILE__, __LINE__ ); \
			exit( 1 );                       \
		}                                    \
	}

#endif // CTRL_INCLUDED
#undef CTRL_INCLUDING

