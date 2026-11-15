#ifdef MATH_INCLUDING
#error  MATH_INCLUDING
#endif //  MATH_INCLUDING
#define MATH_INCLUDING
#ifndef MATH_INCLUDED
#define MATH_INCLUDED

#include "ctrl.cpp"

namespace math
{
	template <typename t>
	static t sqrt_heron( t value, t estimated )
	{
		if( value == (t)0 )
			return( (t)0 );
		t check = estimated;
		estimated = ( estimated + check ) / (t)2;
		check = value / estimated;
		t last;
		do {
			last = estimated;
			estimated = ( estimated + check ) / (t)2;
			check = value / estimated;
		} while( estimated != last );
		return( estimated );
	}
	
	template <typename t>
	static t sqrt_babylon( t a, t b )
	{
		a = d_abs( a );
		b = d_abs( b );
		if( a < b )
			d_flip( a, b );
		t a2 = a * a;
		t b2 = b * b;
		t n = a2 + b2;
		t x = a + b2 / ( (t)2 * a );
		return( sqrt_heron( n, x ) );
	}
	
};

#endif // MATH_INCLUDED
#undef MATH_INCLUDING

