#ifndef NOTMOON_JSON_STRINGBUFFER_HPP
#define NOTMOON_JSON_STRINGBUFFER_HPP

#include "Allocator.hpp"

namespace NotMoon
{
	namespace Json
	{
		class StringBuffer
		{
		private:
			char*				buffer;
			size_t				length;
			size_t				max;
			Allocator<char>&	allocator;

		public:
			StringBuffer( Allocator<char>& allocator, size_t s = 0 )
				: allocator{ allocator }
			{
				this->initialize( s );
			}

			void initialize( size_t size = 0 )
			{
				this->buffer = this->allocator.allocate( size );
				this->length = 0;
				this->max = size;
			}

			void append( char c )
			{
				if( this->max < 1 + this->length )
				{
					this->resize( 1 );
				}
				this->buffer[ this->length++ ] = c;
			}

			void append( const char* begin, const char* end )
			{
				size_t s = end - begin;
				if( this->max < s + this->length )
				{
					this->resize( s );
				}
				copy( this->buffer + this->length, begin, s );
				this->length += s;
			}

			void operator+=( char c )
			{
				this->append( c );
			}

			char* getEnd()
				const
			{
				return this->buffer + this->length;
			}

			char* getBuffer()
				const
			{
				return this->buffer;
			}

			char* convertCString()
			{
				this->append( '\0' );
				return this->buffer;
			}

			void resize( size_t num )
			{
				if( this->allocator.isRange( this->buffer + this->length + num ) )
				{
					char* t = this->allocator.allocate( num + this->length );
					copy( t, this->buffer, this->length );
					this->buffer = t;
				}
			}

		public:
			static inline void copy( char* target, const char* source, size_t n )
			{
				for( size_t i = 0; i < n; ++i )
				{
					target[ i ] = source[ i ];
				}
			}
		};
	}
}

#endif//NOTMOON_JSON_STRINGBUFFER_HPP