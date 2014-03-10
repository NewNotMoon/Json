#ifndef NOTMOON_JSON_PARSER_HPP
#define NOTMOON_JSON_PARSER_HPP
#include <NotMoon/Include.hpp>

#include <NotMoon/Json/Reader.hpp>
#include <NotMoon/Json/Array.hpp>
#include <NotMoon/Json/Object.hpp>
#include <NotMoon/Json/StringBuffer.hpp>
#include <NotMoon/Json/Exception.hpp>

namespace NotMoon
{
	template<typename T>
	static inline void encodeUcs2ToUtf8( char fst, char snd, T &to )
	{
		if( static_cast<unsigned char>( fst ) >= 0x08 )
		{
			to += ( 0xE0 | ( 0xF0 & fst ) >> 4 );
			to += ( 0x80 | ( 0x0F & fst ) << 2 | ( 0xC0 & snd ) >> 6 );
			to += ( 0x80 | 0x3F & snd );
		}
		else if( fst != 0 || static_cast<unsigned char>( snd ) >= 0x80 )
		{
			to += ( 0xC0 | ( fst & 0x7 ) << 2 | ( snd & 0xC0 ) >> 6 );
			to += ( 0x80 | 0x3F & snd );
		}
		else
		{
			to += ( snd );
		}
	}

	template<typename T>
	static inline void encodeSurrogateUcs2ToUtf8( char fst, char snd, char thd, char fth, T &to )
	{
		to += 0xF0 | 0x03 & fst;
		to += 0x80 | ( ( 0xFC & snd ) >> 2 ) + 0x10;
		to += 0x80 | ( 0x03 & snd ) << 4 | ( 0x03 & thd ) << 2 | ( 0xC0 & fth ) >> 6;
		to += 0x80 | 0x3F & fth;
	}

	static inline bool isSurrogate( char c )
	{
		unsigned uc = static_cast<unsigned char>( c );
		return uc >= 0xD8 && uc <= 0xDF;
	}

	namespace Json
	{
		class Parser
		{
		private:
			Chunk memory;
			Allocator<char>	allocator;
			StringBuffer buffer;
			Reader reader;
			static const bool constantTrue = true;
			static const bool constantFalse = false;
			static const unsigned int constantNull = 0;

			struct FunctionTable
			{
			private:
				using Function = Value( Parser::* )();
				Function table[256];

			public:
				FunctionTable()
				{
					for( auto& i : this->table )
					{
						i = &Parser::parseNumber;
					}

					this->table[ '{' ]	= &Parser::parseObject;
					this->table[ '[' ]	= &Parser::parseArray;
					this->table[ '"' ]	= &Parser::parseString;
					this->table[ 't' ]	= &Parser::parseTrue;
					this->table[ 'f' ]	= &Parser::parseFalse;
					this->table[ 'n' ]	= &Parser::parseNull;
				}
				Function operator[]( const char c )
					const
				{
					return this->table[ c ];
				}
			};

			static const unsigned char escapeNumber = 31;
			static const unsigned char escapeNumberValue = 254;
			static const unsigned char escapeString = 5;

			struct ConvertTable
			{
			private:
				using Function = void( Parser::* )();

			public:
				enum class Tag
					: char
				{
					Undefined,
					Hex,
					Escape,
					Number,
				};
				template < class T >
				struct Value
				{
					Tag tag;
					T value;
				};

			private:
				union Union
				{ 
					Value< Function > escape;
					Value< unsigned char > hex;
					Value< unsigned char > number;
				} table[ 256 ];

			public:
				ConvertTable()
				{
					for( auto& i : this->table )
					{
						i.escape ={ Tag::Undefined, nullptr };
					}
					this->table[ '0' ].hex = { Tag::Hex, 0 };
					this->table[ '1' ].hex = { Tag::Hex, 1 };
					this->table[ '2' ].hex = { Tag::Hex, 2 };
					this->table[ '3' ].hex = { Tag::Hex, 3 };
					this->table[ '4' ].hex = { Tag::Hex, 4 };
					this->table[ '5' ].hex = { Tag::Hex, 5 };
					this->table[ '6' ].hex = { Tag::Hex, 6 };
					this->table[ '7' ].hex = { Tag::Hex, 7 };
					this->table[ '8' ].hex = { Tag::Hex, 8 };
					this->table[ '9' ].hex = { Tag::Hex, 9 };
					this->table[ 'a' ].hex = { Tag::Hex, 10 };
					this->table[ 'A' ].hex = { Tag::Hex, 10 };
					this->table[ 'b' ].hex = { Tag::Hex, 11 };
					this->table[ 'B' ].hex = { Tag::Hex, 11 };
					this->table[ 'c' ].hex = { Tag::Hex, 12 };
					this->table[ 'C' ].hex = { Tag::Hex, 12 };
					this->table[ 'd' ].hex = { Tag::Hex, 13 };
					this->table[ 'D' ].hex = { Tag::Hex, 13 };
					this->table[ 'e' ].hex = { Tag::Hex, 14 };
					this->table[ 'E' ].hex = { Tag::Hex, 14 };
					this->table[ 'f' ].hex = { Tag::Hex, 15 };
					this->table[ 'F' ].hex = { Tag::Hex, 15 };

					this->table[ 'b' + escapeString ].escape ={ Tag::Escape, &Parser::appendBufferB };
					this->table[ 'f' + escapeString ].escape ={ Tag::Escape, &Parser::appendBufferF };
					this->table[ 't' + escapeString ].escape ={ Tag::Escape, &Parser::appendBufferT };
					this->table[ 'r' + escapeString ].escape ={ Tag::Escape, &Parser::appendBufferR };
					this->table[ 'n' + escapeString ].escape ={ Tag::Escape, &Parser::appendBufferN };
					this->table[ 'u' + escapeString ].escape ={ Tag::Escape, &Parser::parseUtf16 };


					this->table[ '0' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '1' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '2' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '3' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '4' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '5' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '6' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '7' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '8' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '9' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '+' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '-' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ 'e' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ 'E' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
					this->table[ '.' - escapeNumber ].hex = { Tag::Number, escapeNumberValue };
				}
				Union operator[]( const char c )
					const
				{
					return this->table[ c ];
				}
			};

			static const FunctionTable& getFunctionTable()
			{
				static const FunctionTable table;
				return table;
			}

			static const ConvertTable& getConvertTable()
			{
				static const ConvertTable table;
				return table;
			}

			static Value& getUndefined()
			{
				static Value undefined;
				return undefined;
			}

		public:
			Parser()
				: allocator{ this->memory }
				, buffer{ this->allocator }
			{
			}

			Value parse( const char* begin, const char* end )
			{
				this->allocator.reset();
				this->reader.initialize( begin, end );
				return this->parseValue();
			}

		private:
			Value parseValue()
			{
				return ( this->*getFunctionTable()[ this->reader.skipSpace() ] )( );
			}
			Value parseNumber()
			{
				this->buffer.initialize();
				const char* begin = this->reader.getCurrent() - 1;

				for( char c = this->reader.getPrevious();; c = this->reader.read() )
				{
					if( this->getConvertTable()[ c - escapeNumber ].number.tag == ConvertTable::Tag::Number )
					{
						if( this->reader.isEof() )
						{
							goto end;
						}
					}
					else
					{
						this->reader.goBack();
						goto end;
					}
				}
			end:
				buffer.append( begin, this->reader.getCurrent() );
				buffer.convertCString();
				String* end;
				double *d = reinterpret_cast<double*>( this->allocator.allocate( sizeof( double ) ) );
				*d = std::strtod( buffer.convertCString(), &end );
				return Value( Value::Type::Number, d );
			}
			Value parseObject()
			{
				Object* o = new( this->allocator.allocate( sizeof( Object ) ) ) Object{ this->memory };

				if( this->reader.skipSpace() != '}' )
				{
					this->reader.goBack();
					do
					{
						if( this->reader.skipSpace() == '"' )
						{
							Value key = parseString();
							if( this->reader.skipSpace() == ':' )
							{
								auto k = key.as<String*>();
								o->insert( Pair{ k, parseValue() } );
							}
						}
					}
					while( this->reader.skipSpace() == ',' );
				}
				return Value( Value::Type::Object, o );
			}
			Value parseArray()
			{
				Array *a = new( this->allocator.allocate( sizeof( Array ) ) ) Array{ this->memory };

				if( this->reader.skipSpace() != ']' )
				{
					this->reader.goBack();
					do
					{
						a->push_back( parseValue() );
					}
					while( this->reader.skipSpace() == ',' );
				}
				return Value( Value::Type::Array, a );
			}
			// 文字を16進数で解釈して、対応する数値を返す
			char convertCharToHex()
			{
				auto t = this->getConvertTable()[ this->reader.read() ].hex.value;
				return t;
			};
			// 16進数表記の文字を二つ読み込み、数値(8bit)に変換する
			char readHex()
			{
				auto t = this->convertCharToHex() << 4 | this->convertCharToHex();
				return t;
			};
			// "\uXXXX"形式の文字列を読み込む(UTF-8に変換する)
			void parseUtf16()
			{
				//// \uXXXX\0
				//char src[] =
				//{
				//	*( this->reader.getCurrent() - 2 ),
				//	*( this->reader.getCurrent() - 1 ),
				//	*( this->reader.getCurrent() ),
				//	*( this->reader.getCurrent() + 1 ),
				//	*( this->reader.getCurrent() + 2 ),
				//	*( this->reader.getCurrent() + 3 ),
				//	'\0'
				//};
				//this->reader.seek( 4 );
				//this->buffer.append( '\0' );
				//auto i = u_unescape( src, this->buffer.getEnd() - 1, 1 );
				char fst = this->readHex();
				char snd = this->readHex();

				if( isSurrogate( fst ) )
				{
					if( this->reader.read() == '\\' && this->reader.read() == 'u' )
					{
						auto thd = this->readHex();
						auto fth = this->readHex();
						encodeSurrogateUcs2ToUtf8( fst, snd, thd, fth, this->buffer );
					}
				}
				else
				{
					encodeUcs2ToUtf8( fst, snd, this->buffer );
				}
			};
			void appendBufferB()
			{
				this->buffer += '\b';
			}
			void appendBufferF()
			{
				this->buffer += '\f';
			}
			void appendBufferT()
			{
				this->buffer += '\t';
			}
			void appendBufferR()
			{
				this->buffer += '\r';
			}
			void appendBufferN()
			{
				this->buffer += '\n';
			}
			Value parseString()
			{
				this->buffer.initialize();
				const char* begin = this->reader.getCurrent();
				unsigned char i;
				for( char c = this->reader.read();; c = this->reader.read() )
				{
					switch( c )
					{
					case '\\':
						this->buffer.append( begin, this->reader.getCurrent() - 1 );
						i = this->reader.read() + this->escapeString;
						if( this->getConvertTable()[ i ].escape.tag == ConvertTable::Tag::Escape )
						{
							( this->*getConvertTable()[ i ].escape.value )();
						}
						else
						{
							this->buffer += this->reader.getPrevious();
						}
						begin = this->reader.getCurrent();
						break;
					case '"':
						this->buffer.append( begin, this->reader.getCurrent() - 1 );
						this->allocator.setTail( this->buffer.getEnd() + 1 );
						return Value{ Value::Type::String, this->buffer.convertCString() };
					}
				}
				return this->getUndefined();
			}
			Value parseTrue()
			{
				if( this->reader.read() == 'r' && this->reader.read() == 'u' && this->reader.read() == 'e' && this->reader.getEnd() )
				{
					return Value{ Value::Type::Boolean, &this->constantTrue };
				}
				return this->getUndefined();
			}
			Value parseFalse()
			{
				if( this->reader.read() == 'a' && this->reader.read() == 'l' && this->reader.read() == 's' && this->reader.read() == 'e' && this->reader.getEnd() )
				{
					return Value{ Value::Type::Boolean, &this->constantFalse };
				}
				return this->getUndefined();
			}
			Value parseNull()
			{
				if( this->reader.read() == 'u' && this->reader.read() == 'l' && this->reader.read() == 'l' && this->reader.getEnd() )
				{
					return Value{ Value::Type::Null, &this->constantNull };
				}
				return this->getUndefined();
			}
		};
	}
}

#endif//NOTMOON_JSON_PARSER_HPP