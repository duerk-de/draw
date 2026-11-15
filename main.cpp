#ifdef MAIN_INCLUDING
#error  MAIN_INCLUDING
#endif //  MAIN_INCLUDING
#define MAIN_INCLUDING
#ifndef MAIN_INCLUDED
#define MAIN_INCLUDED

#include "ctrl.cpp"
#include "cmdarg.cpp"
#include "draw.cpp"

struct main_t
{
	int argc;
	char const* const* argv;
	draw_t draw;
	
	main_t( int _argc, char** _argv ) :
		argc( _argc ),
		argv( _argv )
	{ }
	
	void run()
	{
		{
			char buf[1024];
			buildheader( buf, sizeof( buf ), "draw", "2025", true );
			out( buf );
		}
		for( int argi = 1; argi < argc; argi++ )
		{
			cmdarg_t arg( argv[argi] );
			
			if( argi == 1 && !( arg.is_var( "nodefaults" ) && arg.is_cmd() ) )
				draw.setdefaults();

			if( arg.is_empty() )
				continue;

			if( argi == 1 && ( arg.is_var( "nodefaults" ) && arg.is_cmd() ) )
				d_fprint( "not setting default variables\n" );
			else
			if( arg.is_var( "printvar" ) && !arg.is_cmd() )
				draw.calc.printvar( arg.val, arg.vallen );
			else
			if( arg.is_var( "printvars" ) && arg.is_cmd() )
			{
				d_fprint( "list of variables with value in line %i:\n", argi );
				draw.calc.printvars();
			} else
			if( arg.is_var( "data" ) && !arg.is_cmd() )
				draw.setfile( arg.val, arg.vallen );
			else
			if( arg.is_var( "grid" ) && arg.is_cmd() )
				draw.drawgrid();
			else
			if( arg.is_var( "plot" ) && arg.is_cmd() )
				draw.newgraph();
			else
			if( arg.is_var( "plotx" ) && !arg.is_cmd() )
				ENSURE( draw.getgraph()->sx.set( arg.val, arg.vallen ), "%*s already set in line %i\n", arg.varlen, arg.var, argi )
			else
			if( arg.is_var( "ploty" ) && !arg.is_cmd() )
				ENSURE( draw.getgraph()->sy.set( arg.val, arg.vallen ), "%*s already set in line %i\n", arg.varlen, arg.var, argi )
			else
			if( arg.is_var( "lum" ) && !arg.is_cmd() )
				ENSURE( draw.getgraph()->l.set( arg.val, arg.vallen ), "%*s already set in line %i\n", arg.varlen, arg.var, argi )
			else
			if( arg.is_var( "sat" ) && !arg.is_cmd() )
				ENSURE( draw.getgraph()->s.set( arg.val, arg.vallen ), "%*s already set in line %i\n", arg.varlen, arg.var, argi )
			else
			if( arg.is_var( "hue" ) && !arg.is_cmd() )
				ENSURE( draw.getgraph()->h.set( arg.val, arg.vallen ), "%*s already set in line %i\n", arg.varlen, arg.var, argi )
			else
			if( arg.is_var( "datax" ) && !arg.is_cmd() )
				ENSURE( draw.getgraph()->x.set( arg.val, arg.vallen ), "%*s already set in line %i\n", arg.varlen, arg.var, argi )
			else
			if( arg.is_var( "datay" ) && !arg.is_cmd() )
				ENSURE( draw.getgraph()->y.set( arg.val, arg.vallen ), "%*s already set in line %i\n", arg.varlen, arg.var, argi )
			else
			if( arg.is_var( "datacond" ) && !arg.is_cmd() )
				ENSURE( draw.getgraph()->cond.set( arg.val, arg.vallen ), "%*s already set in line %i\n", arg.varlen, arg.var, argi )
			else
			if( arg.is_var( "out" ) && !arg.is_cmd() )
				draw.save( arg.val, arg.vallen );
			else 
				draw.eval( argv[argi] );
		}
		d_fprint( "\n" );
	}
};

int main( int argc, char** argv )
{
	static main_t m( argc, argv );
	double t0 = getintseconds();
	m.run();
	double t1 = getintseconds();
	d_fprint( "done in %t\n", t1 - t0 );
	return( 0 );
}

#endif // MAIN_INCLUDED
#undef MAIN_INCLUDING

