#ifdef ROW_INCLUDING
#error  ROW_INCLUDING
#endif //  ROW_INCLUDING
#define ROW_INCLUDING
#ifndef ROW_INCLUDED
#define ROW_INCLUDED

#include "ctrl.cpp"
#include "math.cpp"
#include "reader.cpp"

typedef double f_t;

struct text_buffered_row_t : public not_copyable_t, reader_line_t
{
	struct cell_t
	{
		bool parsed;
		bool valid;
		uint b;
		uint e;
		f_t val;
		
		cell_t() = default;
		
		cell_t( uint _b, uint _e ) :
			parsed( false ),
			valid( false ),
			b( _b ),
			e( _e )
		{ }
		
		void parse( char const* ptr )
		{
			uint numparsed;
			val = str2doublel( ptr, b, e, numparsed );
			parsed = true;
			valid = numparsed == e - b;
		}
	};

	char * buf;
	uint cap;
	stream<cell_t> cells;
	uint irow;
	bool valid;
	
	text_buffered_row_t() :
		buf( nullptr ),
		cap( 0 ),
		irow( 0 ),
		valid( false )
	{ }
	
	virtual ~text_buffered_row_t()
	{
		if( buf )
			delete [] buf;
	}
	
	virtual void set( char const* _ptr, uint _len )
	{
		cells.drop();
		if( !_ptr || !_len )
		{
			valid = false;
			return;
		}
		if( _len > cap )
		{
			if( buf )
				delete [] buf;
			buf = new char[_len];
			cap = _len;
		}
		cpy( buf, _ptr, _len );
		valid = true;
	}
	
	virtual void add( uint b, uint e )
	{
		cells.add( cell_t( b, e ) );
	}
	
	void reset()
	{
		set( nullptr, 0 );
	}
	
	cell_t const* getcell( uint icol )
	{
		if( icol >= cells.num() )
			return( nullptr );
		cell_t * cell = cells.get( icol );
		if( !cell->parsed )
			cell->parse( buf );
		return( cell );
	}
};

struct averaged_row_t
{
	uint pos;
	uint len;
	stream <text_buffered_row_t*> & buffered_rows;
	
	struct val_t
	{
		f_t val;
		bool valid;
		bool isset;
		
		val_t() :
			valid( false ),
			isset( false )
		{ }
		
		inline void reset()
		{
			valid = false;
			isset = false;
		}
		
		inline void set( f_t const& _val )
		{
			val = _val;
			valid = true;
			isset = true;
		}
	};
	stream <val_t> cells;
	stream <val_t> maxcells;
	stream <val_t> mincells;
	stream <val_t> scells;
	val_t irow;
	
	averaged_row_t( stream <text_buffered_row_t*> & _buffered_rows ) :
		pos( 0 ),
		len( 0 ),
		buffered_rows( _buffered_rows )
	{ }
	
	void setup( uint _pos, uint _len )
	{
		pos = _pos;
		len = _len;
	}
	
	uint end() const
	{
		return( pos + len );
	}
	
	void reset()
	{
		irow.reset();
		cells.drop();
		maxcells.drop();
		mincells.drop();
		scells.drop();
	}
	
	uint numvalid() const
	{
		uint num = 0;
		for( uint i = 0; i < len; i++ )
			if( buffered_rows.getval( i + pos )->valid )
				num++;
		return( num );
	}
	
	bool isfull() const
	{
		return( numvalid() == len );
	}

	val_t const* getirow()
	{
		if( !irow.valid )
		{
			uint sum = 0;
			uint num = 0;
			for( uint i = 0; i < len; i++ )
			{
				text_buffered_row_t * row = buffered_rows.getval( i + pos );
				if( row->valid )
				{
					sum += row->irow;
					num++;
				}
			}
			if( num )
				irow.set( (f_t)sum / (f_t)num );
			else
				irow.isset = true;
		}
		return( &irow );
	}
	
	val_t const* getval( uint icol )
	{
		while( cells.num() <= icol )
			cells.add( val_t() );
		val_t * cell = cells.get( icol );
		if( !cell->isset )
		{
			f_t sum = 0;
			uint num = 0;
			for( uint i = 0; i < len; i++ )
			{
				text_buffered_row_t * row = buffered_rows.getval( i + pos );
				text_buffered_row_t::cell_t const* rowcell = row->getcell( icol );
				if( rowcell && rowcell->valid )
				{
					sum += rowcell->val;
					num++;
				}
			}
			if( num )
				cell->set( sum / (f_t)num );
			else
				cell->isset = true;
		}
		return( cell );
	}
	
	val_t const* getmaxval( uint icol )
	{
		while( maxcells.num() <= icol )
			maxcells.add( val_t() );
		val_t * cell = maxcells.get( icol );
		if( !cell->isset )
		{
			f_t max = 0;
			bool maxvalid = false;
			for( uint i = 0; i < len; i++ )
			{
				text_buffered_row_t * row = buffered_rows.getval( i + pos );
				text_buffered_row_t::cell_t const* rowcell = row->getcell( icol );
				if( rowcell && rowcell->valid )
				{
					if( !maxvalid || rowcell->val > max )
					{
						max = rowcell->val;
						maxvalid = true;
					}
				}
			}
			if( maxvalid )
				cell->set( max );
			else
				cell->isset = true;
		}
		return( cell );
	}
	
	val_t const* getminval( uint icol )
	{
		while( mincells.num() <= icol )
			mincells.add( val_t() );
		val_t * cell = mincells.get( icol );
		if( !cell->isset )
		{
			f_t min = 0;
			bool minvalid = false;
			for( uint i = 0; i < len; i++ )
			{
				text_buffered_row_t * row = buffered_rows.getval( i + pos );
				text_buffered_row_t::cell_t const* rowcell = row->getcell( icol );
				if( rowcell && rowcell->valid )
				{
					if( !minvalid || rowcell->val < min )
					{
						min = rowcell->val;
						minvalid = true;
					}
				}
			}
			if( minvalid )
				cell->set( min );
			else
				cell->isset = true;
		}
		return( cell );
	}
	
	val_t const* getsval( uint icol )
	{
		val_t const* val = getval( icol );
		while( scells.num() <= icol )
			scells.add( val_t() );
		val_t * scell = scells.get( icol );
		if( !scell->isset )
		{
			f_t sum = 0;
			uint num = 0;
			for( uint i = 0; i < len; i++ )
			{
				text_buffered_row_t * row = buffered_rows.getval( i + pos );
				text_buffered_row_t::cell_t const* rowcell = row->getcell( icol );
				if( rowcell && rowcell->valid )
				{
					sum += d_sq( rowcell->val - val->val );
					num++;
				}
			}
			if( num >= 2 )
			{
				f_t sq = sum * 2.f / ( 2.f * (f_t)num - 3.f );
				scell->set( math::sqrt_heron( sq, sq ) );
			} else {
				scell->isset = true;
			}
		}
		return( scell );
	}
};
	
#endif // ROW_INCLUDED
#undef ROW_INCLUDING

