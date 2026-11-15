__GIT_SYMLINKS__
target=../_inc/db_lz4i.cpp
target_resolved=/dibom/src/c/_inc/db_lz4i.cpp
kind=file
sha256=1f57ee1b92d55e72c3822a6ac3c883b3d88978a0eab1e5d52428dfee0ff5a552
size=5649
created=2026-11-15T11:20:55.378164
--END-HEADER--
#ifndef DB_LZ4_INCLUDED
#define DB_LZ4_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "db_str.cpp"

#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <stdint.h>

namespace lz4
{
	#define LZ4F_STATIC_LINKING_ONLY
	
	#include "lz4i/xxhash.c"
	#include "lz4i/lz4.c"
	#include "lz4i/lz4hc.c"
	#include "lz4i/lz4frame.c"
	
	constexpr int max_level = LZ4HC_CLEVEL_MAX;

	bool ensure_bufsize( byte** buf, int* bufsize, int required_size )
	{
		if( *bufsize < required_size )
		{
			*bufsize = required_size;
			delete [] *buf;
			*buf = new byte[*bufsize];
			if( !*buf )
			{
				*bufsize = 0;
				return( false );
			}
		}
		return( true );
	}

	int compress( byte const* in, int insize, byte** out, int* outsize, int level )
	{
		if( level > max_level )
			level = max_level;
		bool favorDecSpeed = true;
		LZ4F_preferences_t prefs = {
			{
				LZ4F_max4MB,
				LZ4F_blockLinked,
				LZ4F_contentChecksumEnabled,
				LZ4F_frame,
				(ulint)insize,
				0u,
				LZ4F_noBlockChecksum },
			level,
			0u,
			favorDecSpeed ? 1u : 0u,
			{ 0u, 0u, 0u }
		};
		
		int required_size = LZ4F_compressFrameBound( insize, &prefs );
		if( !ensure_bufsize( out, outsize, required_size ) )
			return( 0 );

		size_t out_size = LZ4F_compressFrame( *out, *outsize, in, insize, &prefs );
		if( LZ4F_isError( out_size ) )
			out_size = 0;
		return( out_size );
	}

	class decomp
	{
	protected:
		int error;
		LZ4F_dctx context;
		LZ4F_decompressOptions_t opt;
		int blocksize;
		int decompsize;
		
		decomp( decomp const& _t ) = delete;
		decomp& operator =( decomp const& _t ) = delete;
		
		char errmsg[256];

	public:
		enum : int { i_kb = 1024, i_mb = 1024 * 1024 };
		
		decomp()
		{
			error = 0;
			errmsg[0] = '\0';
			zro( &context, sizeof( context ) );
			opt = { false ? 1u : 0u, { 0u, 0u, 0u } };
			blocksize = 0;
			decompsize = 0;
		}
		~decomp()
		{
			delete [] context.tmpIn;
			delete [] context.tmpOutBuffer;
		}
		
		int init( byte const* in, int insize, int* inread )
		{
			int err = 0;
			errmsg[0] = '\0';
			
			LZ4F_frameInfo_t frame_info;
			zro( &frame_info, sizeof( frame_info ) );
			
			size_t read = insize;
			size_t expected_size = LZ4F_getFrameInfo( &context, &frame_info, in, &read );
			*inread = read;

			if( LZ4F_isError( expected_size ) )
			{
				err = 1;
				d_fstrn( errmsg, lenof(errmsg), "lz4 header error: %s", ( expected_size ) );
				blocksize = 0;
				decompsize = 0;
			} else {
				decompsize = frame_info.contentSize;
				switch( frame_info.blockSizeID )
				{
					case LZ4F_max64KB:  blocksize =  64 * i_kb; break; 
					case LZ4F_max256KB: blocksize = 256 * i_kb; break;
					case LZ4F_max1MB:   blocksize =   1 * i_mb; break;
					case LZ4F_max4MB:   blocksize =   4 * i_mb; break;
					default:
						err = 2;
						blocksize = 0;
						decompsize = 0;
						d_fstrn( errmsg, lenof(errmsg), "unknown block size %i", frame_info.blockSizeID );
				}
			}
			
			if( err )
				error = err;
			return( decompsize );
		}
		
		static inline constexpr int maxblocksize()
		{
			return( 4 * i_mb  );
		}
		
		inline int const& getblocksize() const
		{
			return( blocksize );
		}
		
		inline int const& getdecompsize() const
		{
			return( decompsize );
		}
		
		inline bool haserror() const
		{
			return( error != 0 );
		}
		
		inline char const* geterrmsg() const
		{
			return( errmsg );
		}
		
		void seterror( int num, char const* str )
		{
			error = num;
			str2strn( errmsg, lenof(errmsg), str );
		}
		
		bool process( byte const* in, int insize, int* inread, byte* out, int outsize, int* outwritten )
		{
			int err = 0;
			errmsg[0] = '\0';
			
			if( outsize == 0 )
			{
				d_fstrn( errmsg, lenof(errmsg), "output buffer full" );
				err = 3;
			}
			else if( insize == 0 )
			{
				d_fstrn( errmsg, lenof(errmsg), "input buffer empty" );
				err = 4;
			}
			
			if( err )
			{
				*outwritten = 0;
				*inread = 0;
			} else {
				size_t written = outsize;
				size_t read = insize;
				size_t expected_size = LZ4F_decompress( &context, out, &written, in, &read, &opt );
				*outwritten = written;
				*inread = read;
				
				if( LZ4F_isError( expected_size ) )
				{
					d_fstrn( errmsg, lenof(errmsg), "decompression error: %l %s", expected_size, LZ4F_getErrorName( expected_size ) );
					err = 5;
				}
				else if( read != (size_t)insize )
				{
					d_fstrn( errmsg, lenof(errmsg), "consumed only: %v of %i", read, insize );
					err = 6;
				}
				else if( expected_size == 0 )
				{
					return( false );
				}
			}
			if( err )
				error = err;

			return( err == 0 );
		}
	};

	int decompress( byte const* in, int insize, byte** rout, int* routsize, char* errmsg, uint errmsglen )
	{
		decomp dec;
		int read;
		int decompsize = dec.init( in, insize, &read );
		in += read;
		insize -= read;
		
		if( !ensure_bufsize( rout, routsize, decompsize ) )
		{
			if( errmsg )
				d_fstrn( errmsg, errmsglen, "unable to alloc %i bytes", decompsize );
			return( 0 );
		}
		
		byte* out = *rout;
		int outsize = decompsize;
		int written;
		bool dodecompress;
		{
			dodecompress = dec.process( in, insize, &read, out, outsize, &written );
			in += read;
			insize -= read;
			out += written;
			outsize -= written;
		} while( dodecompress );

		if( errmsg )
			d_fstrn( errmsg, errmsglen, "%s", dec.geterrmsg() );
		
		return( !outsize && !dec.haserror() ? decompsize : 0 );
	}
};


#endif // DB_LZ4_INCLUDED
