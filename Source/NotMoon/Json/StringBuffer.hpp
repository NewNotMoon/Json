#ifndef NOTMOON_JSON_STRINGBUFFER_HPP
#define NOTMOON_JSON_STRINGBUFFER_HPP
#include <NotMoon/Include.hpp>

#include <NotMoon/Json/Allocator.hpp>

namespace NotMoon
{
	namespace Json
	{
		class StringBuffer
		{
		private:
			String*				buffer;
			size_t				length;
			size_t				max;
			Allocator<String>	allocator;

		public:
			StringBuffer( Allocator<String> allocator, size_t s = 0 )
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

			void append( String c )
			{
				if( this->max < 1 + this->length )
				{
					this->resize( 1 );
				}
				this->buffer[ this->length++ ] = c;
			}

			void append( const String* begin, const String* end )
			{
				size_t s = end - begin;
				if( this->max < s + this->length )
				{
					this->resize( s );
				}
				copy( this->buffer + this->length, begin, s );
				this->length += s;
			}
			
			void operator+=( String c )
			{
				this->append( c );
			}

			String* getEnd()
				const
			{
				return this->buffer + this->length;
			}

			String* getBuffer()
				const
			{
				return this->buffer;
			}

			String* convertCString()
			{
				this->append( '\0' );
				return this->buffer;
			}

			void resize( size_t num )
			{
				if( this->allocator.isRange( this->buffer + this->length + num ) )
				{
					String* t = this->allocator.allocate( num + this->length );
					copy( t, this->buffer, this->length );
					this->buffer = t;
				}
			}

		public:
			template < typename T >
			static inline void copy( T* target, const T* source, size_t n )
			{
				for( size_t i = 0; i < n; ++i )
				{
					target[ i ] = source[ i ];
				}
			}
			template < typename T, typename U >
			static inline void copy( T* target, const U* source, size_t n )
			{
				for( size_t i = 0; i < n; ++i )
				{
					target[ i ] = static_cast< T >( source[ i ] );
				}
			}
		};
	}
}

#endif//NOTMOON_JSON_STRINGBUFFER_HPP