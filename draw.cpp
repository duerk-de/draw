#ifdef DRAW_INCLUDING
#error  DRAW_INCLUDING
#endif //  DRAW_INCLUDING
#define DRAW_INCLUDING
#ifndef DRAW_INCLUDED
#define DRAW_INCLUDED

#include "ctrl.cpp"
#include "drawcalc.cpp"
#include "datareader.cpp"
#include "grid.cpp"
#include "graph.cpp"

struct draw_t
{
	drawcalc_t calc;
	datareader_t datareader;
	image_t image;
	layer_t layer;
	grid_t grid;
	pstream<graph_t> graphs;
	bool defaults;
	
	draw_t() :
		defaults( false )
	{
		graphs.add( new graph_t() );
	}
	
	void newgraph()
	{
		graphs.getlast()->setup( calc, datareader.data );
		graphs.add( new graph_t( *graphs.getlast() ) );
	}
	
	graph_t * getgraph()
	{
		return( graphs.getlast() );
	}
	
	void setfile( char const* name, uint len )
	{
		drawgraphs();
		datareader.setfile( name, len );
	}
	
	void setdefaults()
	{
		d_fprint( "setting default variables\n" );
		defaults = true;
		eval( "false = 0" );
		eval( "true = 1" );
		eval( "defaults = true" );
		eval( "border = 3" );
		calc.setdefaults();
		datareader.defaults( calc );
		image.defaults( calc );
		getgraph()->defaults( calc );
		eval( nullptr );
	}

	void eval( char const* str )
	{
		if( str )
			calc.eval( str );
		if( defaults )
		{
			layer.defaults( calc, false );
			layer.border( calc, false );
			grid.defaults( calc );
		}
	}
	
	void drawgraphs()
	{
		uint ngraphs = graphs.num();
		ENSURE( ngraphs, "no graph\n" );
		ngraphs--;
		
		if( !ngraphs )
			return;
		
		image.setup( calc );
		layer.setup( image, calc );
		datareader.setup( calc );
		
		d_fprint( "drawing %i graphs from %i file"
		          " (avg=%i, step=%i, backward=%b, bailout=%b, bailin=%b):\n",
		          ngraphs, datareader.filenames.num(),
		          datareader.avg, datareader.step,
		          datareader.backward, datareader.bailout, datareader.bailin );
		for( uint i = 0; i < datareader.filenames.num(); i++ )
			d_fprint( "%*s\n", datareader.filenames.get( i )->len,
			          datareader.filenames.get( i )->val );
		/*
		for( uint igraph = 0; igraph < ngraphs; igraph++ )
			graphs.get( igraph )->print();
		*/

		bool waswithinlayer = datareader.bailin ? false : true;
		while( datareader.readline() )
		{
			bool outoflayer = true;
			for( uint igraph = 0; igraph < ngraphs; igraph++ )
			{
				if( graphs.get( igraph )->draw( image, layer, true ) )
				{
					outoflayer = false;
					if( !waswithinlayer )
					{
						d_fprint( "bailin\n" );
						waswithinlayer = true;
					}
				}
			}
			if( datareader.bailout && outoflayer && waswithinlayer )
			{
				d_fprint( "bailout\n" );
				break;
			}
		}

		for( uint igraph = 0; igraph < ngraphs; igraph++ )
			while( graphs.get( igraph )->draw( image, layer, false ) );
		
		for( uint igraph = 0; igraph < ngraphs; igraph++ )
			graphs.del( 0 );
		
		datareader.teardown();
		datareader.unsetfiles();
		
		// exit( 1 );
	}
	
	void drawgrid()
	{
		image.setup( calc );
		layer.setup( image, calc );
		grid.setup( layer, calc );
		grid.draw( image, layer );
	}
	
	void save( char const* name, uint len )
	{
		drawgraphs();
		image.filename.set( name, len );
		image.write();
	}
};

#endif // DRAW_INCLUDED
#undef DRAW_INCLUDING

