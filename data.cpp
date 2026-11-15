#ifdef DATA_INCLUDING
#error  DATA_INCLUDING
#endif //  DATA_INCLUDING
#define DATA_INCLUDING
#ifndef DATA_INCLUDED
#define DATA_INCLUDED

#include "ctrl.cpp"
#include "row.cpp"

struct data_t
{
	stream <text_buffered_row_t*> rows;
	averaged_row_t next;
	averaged_row_t curr;
	averaged_row_t prev;
	
	data_t() :
		next( rows ),
		curr( rows ),
		prev( rows )
	{ }
	
	~data_t()
	{
		while( rows.num() )
		{
			delete *rows.get( rows.num() - 1 );
			rows.del( rows.num() - 1 );
		}
	}
	
	void setup( bool backward, uint iavg )
	{
		uint num;
		if( backward )
		{
			prev.setup(          0, iavg );
			curr.setup( prev.end(), iavg );
			next.setup( curr.end(), iavg );
			num = next.end();
		} else {
			next.setup(          0, iavg );
			curr.setup( next.end(), iavg );
			prev.setup( curr.end(), iavg );
			num = prev.end();
		}
		while( rows.num() < num )
			rows.add( new text_buffered_row_t() );
		while( rows.num() > num )
		{
			delete rows.getval( rows.num() - 1 );
			rows.del( rows.num() - 1 );
		}
	}
	
	void teardown()
	{
		for( uint i = 0; i < rows.num(); i++ )
			rows.getval( i )->valid = false;
		prev.setup( 0, 0 );
		curr.setup( 0, 0 );
		next.setup( 0, 0 );
	}
	
	text_buffered_row_t * getnextline( uint irow )
	{
		text_buffered_row_t * line = rows.getval( rows.num() - 1 );
		rows.del( rows.num() - 1 );
		line->reset();
		rows.ins( line, 0 );
		next.reset();
		curr.reset();
		prev.reset();
		line->irow = irow;
		return( line );
	}

	f_t geti()
	{
		averaged_row_t::val_t const* v = curr.getirow();
		return( v->valid ? v->val : 0 );
	}
	
	f_t getval( uint i )
	{
		averaged_row_t::val_t const* v = curr.getval( i );
		return( v->valid ? v->val : 0 );
	}
	
	f_t getmaxval( uint i )
	{
		averaged_row_t::val_t const* v = curr.getmaxval( i );
		return( v->valid ? v->val : 0 );
	}
	
	f_t getminval( uint i )
	{
		averaged_row_t::val_t const* v = curr.getminval( i );
		return( v->valid ? v->val : 0 );
	}
	
	f_t getsval( uint i )
	{
		averaged_row_t::val_t const* v = curr.getsval( i );
		return( v->valid ? v->val : 0 );
	}
	
	f_t getdi()
	{
		averaged_row_t::val_t const* n = next.getirow();
		averaged_row_t::val_t const* p = prev.getirow();
		if( !n->valid || !p->valid )
		{
			averaged_row_t::val_t const* c = curr.getirow();
			if( n->valid )
			{
				return( n->val - c->val );
			} else
			if( p->valid )
			{
				return( c->val - p->val );
			} else {
				return( 0 );
			}
		}
		return( ( n->val - p->val ) / 2 );
	}
	
	f_t getdval( uint i )
	{
		averaged_row_t::val_t const* n = next.getval( i );
		averaged_row_t::val_t const* p = prev.getval( i );
		if( !n->valid || !p->valid )
		{
			averaged_row_t::val_t const* c = curr.getval( i );
			if( n->valid )
			{
				return( n->val - c->val );
			} else
			if( p->valid )
			{
				return( c->val - p->val );
			} else {
				return( 0 );
			}
		}
		return( ( n->val - p->val ) / 2 );
	}
};

#endif // DATA_INCLUDED
#undef DATA_INCLUDING

