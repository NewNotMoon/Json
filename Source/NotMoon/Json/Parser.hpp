#ifndef NOTMOON_JSON_PARSER_HPP
#define NOTMOON_JSON_PARSER_HPP

#include "Reader.hpp"
#include "Array.hpp"
#include "Object.hpp"
#include "StringBuffer.hpp"
#include "Exception.hpp"

namespace NotMoon
{
	template<typename T>
	static inline void encodeUcs2ToUtf8( char fst, char snd, T &to )
	{
		if( static_cast<unsigned char>( fst ) >= 0x08 )
		{
			to += 0xE0 | ( 0xF0 & fst ) >> 4;
			to += 0x80 | ( 0x0F & fst ) << 2 | ( 0xC0 & snd ) >> 6;
			to += 0x80 | 0x3F & snd;
		}
		else if( fst != 0 || static_cast<unsigned char>( snd ) >= 0x80 )
		{
			to += 0xC0 | ( fst & 0x7 ) << 2 | ( snd & 0xC0 ) >> 6;
			to += 0x80 | 0x3F & snd;
		}
		else
		{
			to += snd;
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
			using Function = Value( Parser::* )( Reader& );
			using FunctionArray = Value( Parser::*[ 256 ] )( Reader& );
			Chunk memory;
			Allocator<char>	allocator;
			StringBuffer buffer;
			static const bool constantTrue = true;
			static const bool constantFalse = false;
			static const unsigned int constantNull = 0;
			struct FunctionTable
			{
			private:
				FunctionArray table;

			public:
				FunctionTable()
				{
					for( auto& i : this->table )
					{
						i = &Parser::parseNumber;
					}
					this->table[ '{' ] = &Parser::parseObject;
					this->table[ '[' ] = &Parser::parseArray;
					this->table[ '"' ] = &Parser::parseString;
					this->table[ 't' ] = &Parser::parseTrue;
					this->table[ 'f' ] = &Parser::parseFalse;
					this->table[ 'n' ] = &Parser::parseNull;
				}
				Function operator[]( const char c )
				{
					return this->table[ c ];
				}
			};

			static FunctionTable& getParseTable()
			{
				static FunctionTable table;
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
				Reader in{ begin, end };
				return this->parseValue( in );
			}

		private:
			Value parseNumber( Reader& in )
			{
				this->buffer.initialize();
				const char* begin = in.getCurrent() - 1;

				for( char c = in.getPrevious();; c = in.read() )
				{
					switch( c )
					{
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
					case '+': case '-': case '.': case 'e': case 'E':
						if( in.isEof() )
						{
							goto end;
						}
						break;

					default:
						in.goBack();
						goto end;
					}
				}
			end:
				buffer.append( begin, in.getCurrent() );
				buffer.convertCString();
				char* end;
				double *d = reinterpret_cast<double*>( this->allocator.allocate( sizeof( double ) ) );
				*d = std::strtod( buffer.convertCString(), &end );
				return Value( Value::Type::Number, d );
			}
			Value parseObject( Reader& in )
			{
				Object* o = new( this->allocator.allocate( sizeof( Object ) ) ) Object{ this->memory };

				if( in.skipSpace() != '}' )
				{
					in.goBack();
					do
					{
						if( in.skipSpace() == '"' )
						{
							Value key = parseString( in );
							if( in.skipSpace() == ':' )
							{
								auto k = key.as<String>();
								o->insert( Pair{ k, parseValue( in ) } );
							}
						}
					}
					while( in.skipSpace() == ',' );
				}
				return Value( Value::Type::Object, o );
			}
			Value parseArray( Reader& in )
			{
				Array *a = new( this->allocator.allocate( sizeof( Array ) ) ) Array{ this->memory };

				if( in.skipSpace() != ']' )
				{
					in.goBack();
					do
					{
						a->push_back( parseValue( in ) );
					}
					while( in.skipSpace() == ',' );
				}
				return Value( Value::Type::Array, a );
			}
			Value	parseString( Reader& in )
			{
				// "\uXXXX"形式の文字列を読み込む(UTF-8に変換する)
				static auto parseUtf16 = []( Reader& in, StringBuffer& s )
				{
					// 16進数表記の文字を二つ読み込み、数値(8bit)に変換する
					static auto readHex = []( Reader& in )
						-> char
					{
						// 文字を16進数で解釈して、対応する数値を返す
						static auto convertCharToHex = []( Reader& in )
						{
							switch( in.read() )
							{
#define MAP(c,n)		case c: return n;
#define MAP2(c1,c2,n)	case c1: case c2: return n;
								MAP( '0', 0 ); MAP( '1', 1 ); MAP( '2', 2 ); MAP( '3', 3 ); MAP( '4', 4 );	MAP( '5', 5 ); MAP( '6', 6 ); MAP( '7', 7 ); MAP( '8', 8 ); MAP( '9', 9 );
								MAP2( 'a', 'A', 10 ); MAP2( 'b', 'B', 11 ); MAP2( 'c', 'C', 12 ); MAP2( 'd', 'D', 13 ); MAP2( 'e', 'E', 14 ); MAP2( 'f', 'F', 15 );
#undef MAP
#undef MAP2
							default:
								_ASSERT( false );
							}
							return -1;
						};
						return convertCharToHex( in ) << 4 | convertCharToHex( in );
					};
					char fst = readHex( in );
					char snd = readHex( in );

					if( isSurrogate( fst ) )
					{
						if( in.read() == '\\' && in.read() == 'u' )
						{
							encodeSurrogateUcs2ToUtf8( fst, snd, readHex( in ), readHex( in ), s );
						}
					}
					else
					{
						encodeUcs2ToUtf8( fst, snd, s );
					}
				};

				this->buffer.initialize();
				const char* begin = in.getCurrent();
				for( char c = in.read();; c = in.read() )
				{
					switch( c )
					{
					case '\\':
						this->buffer.append( begin, in.getCurrent() - 1 );
						switch( in.read() )
						{
						case 'b': this->buffer += '\b'; break;
						case 'f': this->buffer += '\f'; break;
						case 't': this->buffer += '\t'; break;
						case 'r': this->buffer += '\r'; break;
						case 'n': this->buffer += '\n'; break;
						case 'u': parseUtf16( in, this->buffer ); break;
						default: this->buffer += in.getPrevious();
						}
						begin = in.getCurrent();
						break;
					case '"':
						this->buffer.append( begin, in.getCurrent() - 1 );
						this->allocator.setTail( this->buffer.getEnd() + 1 );
						return Value{ Value::Type::String, this->buffer.convertCString() };
					}
				}
				return this->getUndefined();
			}
			Value parseTrue( Reader& in )
			{
				if( in.read() == 'r' && in.read() == 'u' && in.read() == 'e' && in.getEnd() )
				{
					return Value{ Value::Type::Boolean, &this->constantTrue };
				}
				return this->getUndefined();
			}
			Value parseFalse( Reader& in )
			{
				if( in.read() == 'a' && in.read() == 'l' && in.read() == 's' && in.read() == 'e' && in.getEnd() )
				{
					return Value{ Value::Type::Boolean, &this->constantFalse };
				}
				return this->getUndefined();
			}
			Value parseNull( Reader& in )
			{
				if( in.read() == 'u' && in.read() == 'l' && in.read() == 'l' && in.getEnd() )
				{
					return Value{ Value::Type::Null, &this->constantNull };
				}
				return this->getUndefined();
			}
			Value parseValue( Reader& in )
			{
				return (this->*getParseTable()[ in.skipSpace() ])( in );
			}
		};
	}
}

#endif//NOTMOON_JSON_PARSER_HPP