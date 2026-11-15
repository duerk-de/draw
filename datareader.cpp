#ifdef DATAREADER_INCLUDING
#error DATAREADER_INCLUDING
#endif // DATAREADER_INCLUDING
#define DATAREADER_INCLUDING
#ifndef DATAREADER_INCLUDED
#define DATAREADER_INCLUDED

#include "ctrl.cpp"
#include "drawcalc.cpp"
#include "userval.cpp"
#include "data.cpp"
#include "reader.cpp"

struct datareader_t
{
	stream<userval_t> filenames;
	uint backward;
	uint bailout;
	uint bailin;

	uint avg;
	uint step;
	
	uint ifile;
	uint irow;
	uint nrow;
	reader_t * reader;
	
	data_t data;
	
	datareader_t() :
		backward( false ),
		bailout( false ),
		bailin( false ),
		avg( 1 ),
		step( 1 ),
		irow( 0 ),
		nrow( 1 ),
		reader( nullptr )
	{ }
	
	~datareader_t()
	{
		if( reader )
			delete reader;
	}
	
	void setfile( char const* name, int len )
	{
		filenames.add( userval_t( name, len ) );
		irow = 0;
		nrow = 1;
	}
	
	void unsetfiles()
	{
		filenames.drop();
	}

	void defaults( drawcalc_t & calc )
	{
		calc.eval( "avg = 1" );
		calc.eval( "step = 1" );
		calc.eval( "backward = 0" );
		calc.eval( "bailout = 0" );
		calc.eval( "bailin = 0" );
	}
	
	void setup( drawcalc_t const& calc )
	{
		ENSURE( !reader, "reader setup\n" );
		ENSURE( filenames.num(), "data not set up\n" );
		avg = calc.getintval( "avg" );
		if( avg < 1 )
			avg = 1;
		ENSURE( avg >= 1, "avg out of range (%i)\n", avg );
		step = calc.getintval( "step" );
		if( step < 1 )
			step = 1;
		ENSURE( step >= 1, "step out of range (%i)\n", step );
		backward = calc.getintval( "backward" );
		bailout = calc.getintval( "bailout" );
		bailin = calc.getintval( "bailin" );
		data.setup( backward, avg );
		ifile = 0;
		userval_t * filename = filenames.get( ifile );
		if( backward )
			reader = new reader_backward_t( filename->val, filename->len );
		else
			reader = new reader_forward_t( filename->val, filename->len );
		ENSURE( reader, "oom\n" );
	}
	
	void teardown()
	{
		delete reader;
		reader = nullptr;
		data.teardown();
		filenames.drop();
	}
	
	bool readline()
	{
		for( ;; )
		{
			text_buffered_row_t * line = data.getnextline( ++irow );
			while( !reader->parsenextline( *line ) && ifile + 1 < filenames.num() )
			{
				delete reader;
				reader = nullptr;
				ifile++;
				userval_t * filename = filenames.get( ifile );
				if( backward )
					reader = new reader_backward_t( filename->val, filename->len );
				else
					reader = new reader_forward_t( filename->val, filename->len );
				ENSURE( reader, "oom\n" );
			}
			if( data.curr.isfull() )
			{
				// d_fprint( "full\n" );
				if( irow >= nrow )
				{
					// d_fprint( "okay\n" );
					// d_fprint( "%d  %d\n", data.getval( 0 ), data.getval( 1 ) );
					nrow = irow + step;
					return( true );
				}
			} else {
				if( !line->valid  )
					return( false );
			}
		}
	}
};

#endif // DATAREADER_INCLUDED
#undef DATAREADER_INCLUDING

