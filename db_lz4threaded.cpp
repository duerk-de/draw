__GIT_SYMLINKS__
target=../_inc/db_lz4threaded.cpp
target_resolved=/dibom/src/c/_inc/db_lz4threaded.cpp
kind=file
sha256=4acd5df4f8eb2e0654d04f611cdf0cbdb596f6e9d68afc50fe12dad0b11f341e
size=5576
created=2026-11-15T11:20:55.380366
--END-HEADER--
#ifndef DB_LZ4THREADED_INCLUDED
#define DB_LZ4THREADED_INCLUDED


#include "db_defines.cpp"
#include "db_lz4i.cpp"
#include "db_readthread.cpp"


namespace lz4
{
	class decomp_block
	{
	protected:
		enum e_state : int { state_notinitialized,
							state_idle,
							state_finished,
							state_error };
		int state;
		lz4::decomp dec;
		byte* out;
		byte* out0;
		int outsize;
		
		e_state parse_header( byte const* buf, int size_to_read, int* read )
		{
			e_state istate = state_idle;
			if( dec.init( buf, size_to_read, read ) == 0 )
				istate = state_error;
			if( dec.haserror() )
				istate = state_error;
			return( istate );
		}
		
		e_state decompress( byte const* buf, int size_to_read )
		{
			e_state istate = state_idle;
			if( size_to_read )
			{
				int read, written;
				if( !dec.process( buf, size_to_read, &read, out, outsize, &written ) )
					istate = state_finished;
				out += written;
				outsize -= written;
				if( dec.haserror() )
					istate = state_error;
				else if( read != size_to_read )
				{
					dec.seterror( 7, "did not process all data" );
					istate = state_error;
				}
			}
			return( istate );
		}

	public:
		decomp_block()
		{
			out0 = nullptr;
			out = nullptr;
			outsize = 0;
		}
		bool init( byte const* buf, int size_to_read, int* size_read )
		{
			state = parse_header( buf, size_to_read, size_read );
			return( state == state_idle || state == state_finished );
		}
		void set_output( byte* _out, int _outsize )
		{
			out0 = _out;
			out = out0;
			outsize = _outsize;
		}
		bool decompress_inline( byte const* buf, int size_to_read )
		{
			if( state == state_idle )
				state = decompress( buf, size_to_read );
			else if( state != state_finished && state != state_error )
			{
				dec.seterror( 8, "not ready to process" );
				state = state_error;
			}
			return( state == state_idle || state == state_finished );
		}
		inline int const& getdecompsize() const
		{
			return( dec.getdecompsize() );
		}
		inline int const& getblocksize() const
		{
			return( dec.getblocksize() );
		}
		static inline constexpr int maxblocksize()
		{
			return( lz4::decomp::maxblocksize() );
		}
		inline int getlength() const
		{
			return( out - out0 );
		}
		inline bool ready() const
		{
			return( state == state_idle );
		}
		inline bool finished()
		{
			return( state == state_finished );
		}
		inline char const* geterrmsg() const
		{
			return( dec.geterrmsg() );
		}
	};

	bool decompress_threaded( char const* filename,
							  byte** out, int* outsize, int *outlength,
							  byte** in, int* insize, int* inlength, 
							  char* errmsg, uint errmsglen )
	{
		readthread reader;
		lz4::decomp_block decompressor;
		static constexpr int max_supported_blocksize = 256 * lz4::decomp::i_kb;
		static_assert( LZ4F_HEADER_SIZE_MAX <= max_supported_blocksize );
		int wrlen;
		int rdlen;
		byte* rd;
		byte* wr;
		constexpr bool single_threaded = true;
		
		if( errmsg && errmsglen )
			*errmsg = '\0';
		
		do {
			
			if( !reader.init( filename ) )
				break;
			
			if( !lz4::ensure_bufsize( in, insize, max_supported_blocksize * 2 ) )
			{
				if( errmsg && !*errmsg )
					d_fstrn( errmsg, errmsglen, "could not alloc input buffer of %i bytes", max_supported_blocksize * 2 );
				break;
			}
			rd = *in;
			
			if( !reader.read_inline( rd, LZ4F_HEADER_SIZE_MAX, &wrlen ) )
				break;
			if( wrlen != LZ4F_HEADER_SIZE_MAX )
			{
				if( errmsg && !*errmsg )
					d_fstrn( errmsg, errmsglen, "error reading header" );
				break;
			}
			if( !decompressor.init( rd, wrlen, &rdlen ) )
				break;
			
			if( decompressor.getblocksize() > max_supported_blocksize )
			{
				if( errmsg && !*errmsg )
					d_fstrn( errmsg, errmsglen, "blocksize of %i not supported. limit is %i", decompressor.getblocksize(), max_supported_blocksize );
				break;
			}
			wr = *in + decompressor.getblocksize();

			if( !lz4::ensure_bufsize( out, outsize, decompressor.getdecompsize() ) )
			{
				if( errmsg && !*errmsg )
					d_fstrn( errmsg, errmsglen, "could not alloc output buffer of %i bytes", decompressor.getdecompsize() );
				break;
			}
			decompressor.set_output( *out, *outsize );
			
			int notrdlen = wrlen - rdlen;
			cpy( rd, rd + rdlen, notrdlen );
			if( reader.read_inline( rd + notrdlen, decompressor.getblocksize() - notrdlen, &rdlen ) )
			{
				rdlen += notrdlen;
				if( !single_threaded )
					reader.start( false, 0 );
				while( reader.ready() && decompressor.ready() )
				{
					// FIXME: multithreading does not work
					if( !single_threaded )
						reader.read_threaded( wr, decompressor.getblocksize() );
					else
						reader.read_inline( wr, decompressor.getblocksize(), &wrlen );
					decompressor.decompress_inline( rd, rdlen );
					if( !single_threaded )
						rdlen = reader.wait_for_reading();
					else
						rdlen = wrlen;
					d_flip( rd, wr );
				}
				if( !single_threaded )
					reader.cancel_loop();
			}

			decompressor.decompress_inline( rd, rdlen );
			
		} while( false );

		if( inlength != nullptr )
			*inlength = reader.gettotalreadsize();
		if( outlength != nullptr )
			*outlength = decompressor.getlength();
		
		if( reader.file_not_openable() )
			return( true );

		if( errmsg && errmsglen )
		{
			if( !reader.finished() && !*errmsg )
				d_fstrn( errmsg, errmsglen, "did not read complete file: %s", reader.geterrmsg() );
			if( !decompressor.finished() && !*errmsg )
				d_fstrn( errmsg, errmsglen, "did not decompress all data: %s", decompressor.geterrmsg() );
		}
		
		return( reader.finished() && decompressor.finished() );
	}
};

#endif // DB_LZ4THREADED_INCLUDED

