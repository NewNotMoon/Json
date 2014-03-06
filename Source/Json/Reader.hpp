#ifndef NOTMOON_JSON_READER_HPP
#define NOTMOON_JSON_READER_HPP

#include "../Debug/Debug.hpp"

namespace NotMoon
{
	namespace Json
	{
		enum class Charactor
			: unsigned int
		{
			Undefined,
			Space,
			End,
		};

		struct CharactorTable
		{
			Charactor table[ 256 ];
			CharactorTable()
			{
				for( auto& it : table )
				{
					it = Charactor::Undefined;
				}
				table[ ' ' ]	= Charactor::Space;
				table[ '\t' ]	= Charactor::Space;
				table[ '\r' ]	= Charactor::Space;
				table[ '\n' ]	= Charactor::Space;
				table[ '}' ]	= Charactor::End;
				table[ ']' ]	= Charactor::End;
				table[ ',' ]	= Charactor::End;
			}
			Charactor operator[]( const char c )
			{
				return this->table[ c ];
			}
		};

		static inline CharactorTable getTable()
		{
			static CharactorTable table;
			return table;
		}

		class Reader
		{
		private:
			const char*	begin;
			const char*	current;
			const char* end;

		public:
			Reader( const char* begin, const char* end )
				: begin{ begin }
				, current{ begin }
				, end{ end }
			{
			}
			Reader( const char* begin, size_t size )
				: begin{ begin }
				, current{ begin }
				, end{ begin + size }
			{
			}
			char read()
			{
				return *this->current++;
			}
			void goBack()
			{
				--this->current;
			}
			char getPrevious()
				const
			{
				return this->current[ -1 ];
			}
			bool isEof()
				const
			{
				return !( this->current < this->end );
			}
			const char*	getCurrent()
				const
			{
				return this->current;
			}
			char skipSpace()
			{
				while( true )
				{
					if( getTable()[ *this->current ] != Charactor::Space )
					{
						return *this->current++;
					}
					++this->current;
				}
			}
			bool getEnd()
				const
			{
				if( isEof() )
				{
					return true;
				}
				switch( *this->current )
				{
				case ' ': case '\t': case '\r': case '\n': case '}': case ']': case ',': return true;
				}
				return false;
			}
		};
	}
}

#endif//NOTMOON_JSON_READER_HPP