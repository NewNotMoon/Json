#ifndef NOTMOON_JSON_HPP
#define NOTMOON_JSON_HPP

#include <vector>
#include <unordered_map>

namespace NotMoon
{
	class ParseErrorException
	{
	};

	namespace Json
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

		// �`�����N(�������u���b�N)��ێ�����\����
		struct Chunk
		{
			std::vector<char*> chunks;	// �`�����N�̔z��
			unsigned maxSize;			// ��ԑ傫���`�����N(= chunks.back())�̃T�C�Y
			unsigned offset;			// ���g�p�̃������J�n�ʒu

			Chunk( unsigned init_size = 0x8000 )
				:offset( 0 )
				, maxSize( init_size )
			{
				chunks.push_back( new char[ maxSize ] );
			}
			~Chunk()
			{
				for( char* i : chunks )
				{
					delete[ ] i;
				}
			}
		};

		template<class T>
		class Allocator
		{
		private:
			Chunk& cnk;

		public:
			typedef			T*	pointer;
			typedef const	T*	const_pointer;
			typedef			T&	reference;
			typedef const	T&	const_reference;
			typedef			T	value_type;

			// �A���P�[�^��U�^�Ƀo�C���h����
			template<class U>
			struct rebind
			{
				typedef Allocator<U> other;
			};

			Allocator( Chunk& cnk )
				:cnk( cnk )
			{
			}

			template <class U> Allocator( const Allocator<U>& src )
				: cnk( const_cast<Chunk&>( src.getChunk() ) )
			{
			}

			// STL�̃A���P�[�^�N���X�ɕK�v�ȃ��\�b�h�Q
			// �����������蓖�Ă�
			pointer allocate( unsigned num )
			{
				if( this->cnk.offset + sizeof(T)* num >= this->cnk.maxSize )
				{
					// ������������Ȃ��ꍇ�́Achunk�𑝂₷
					this->cnk.maxSize *= 2;
					this->cnk.chunks.push_back( new char[ this->cnk.maxSize ] );
					this->cnk.offset=0;

					return allocate( num ); // ������xallocate
				}

				// offset����Asizeof(T)*num�������A�����������蓖�Ă�
				// XXX: �����ŃR���X�g���N�^���Ăяo���̂͊Ԉ���Ă���C������(2009/11/19)
				pointer ptr = new( this->cnk.chunks.back() + this->cnk.offset ) T[ num ];
				this->cnk.offset += sizeof(T)*num;
				return ptr;
			}

			// �����čς݂̗̈������������
			void construct( pointer p, const T& value )
			{
				new( (void*)p ) T( value );
			}

			// ���������������
			// �� �����s��Ȃ�
			void deallocate( pointer p, unsigned num )
			{
			}

			// �������ς݂̗̈���폜����
			void destroy( pointer p )
			{
				p->~T();
			}


			// JSON�ɕK�v�ȃ��\�b�h�Q
			// ���g�p�������J�n�ʒu(offset)��0�Ƀ��Z�b�g����
			void reset()
			{
				this->cnk.offset = 0;
				if( this->cnk.chunks.size() > 1 )
				{
					// �`�����N����������ꍇ�́A��ԍŌ�̂��̈ȊO��delete����
					char *tmp = this->cnk.chunks.back();
					for( size_t i = 0; i < this->cnk.chunks.size() - 1; ++i )
					{
						delete[ ] this->cnk.chunks[ i ];
					}
					this->cnk.chunks.clear();
					this->cnk.chunks.push_back( tmp );
				}
			}

			// chunk�̖��g�p�������J�n�ʒu(offset)��ݒ肷��
			// JSON��parse_string�̂��߂ɁA���ʂɗp��
			void setTail( char* ptr )
			{
				this->cnk.offset = ptr - this->cnk.chunks.back();
			}

			// �����̃|�C���^(ptr)���A���炩���ߊm�ۂ��Ă��郁�����u���b�N�͈͓̔����ǂ����𔻒肷��
			// �͈͊O�Ȃ�Atrue��Ԃ�
			// �� ����݂̂��`�F�b�N���Ă���
			bool isRange( char* ptr )
			{
				return this->cnk.chunks.back() + this->cnk.maxSize <= ptr;
			}
			const Chunk& getChunk() const
			{
				return this->cnk;
			}
		};

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

		class Value;

		using Null			= unsigned int;
		using Boolean		= bool;
		using Number		= double;
		using String		= std::string;
		using Array			= std::vector< Value >;
		using Pair			= std::pair< std::string, Value >;
		class Object;

		class Value
		{
		public:
			enum class Type
				: short
			{
				Undefined,
				Null,
				Boolean,
				Number,
				String,
				Array,
				Object,
			};

		private:
			const void* p;
			Type  type;

		public:
			Value()
				: type{ Type::Undefined }
				, p{ nullptr }
			{
			}
			Value( Type t, const void* p )
				: type{ t }
				, p{ p }
			{
			}

			Type getType()
				const
			{
				return this->type;
			}

		public:
			template<class T>
			T& as()
			{
				return static_cast<const T*>( this->p );
			}
		};

		using Map = std::unordered_map< std::string, Value >;

		class Object
			: public Map
		{
		public:
			Object()
			{
			}

			Value& operator[]( const std::string& key )
			{
				static Value undefined;

				auto it = this->find( key );
				if( it != this->end() )
				{
					return it->second;
				}
				else
				{
					return undefined;
				}
			}

			const Value& operator[]( const std::string& key )
				const
			{
				static const Value undefined;

				auto it = this->find( key );
				if( it != this->end() )
				{
					return it->second;
				}
				else
				{
					return undefined;
				}
			}
		};


		template<>
		Array& Value::as<Array>()
		{
			return *( static_cast<Array*>( const_cast<void*>( p ) ) );
		}
		template<>
		Object& Value::as<Object>()
		{
			return *( static_cast<Object*>( const_cast<void*>( p ) ) );
		}
		template<>
		Number& Value::as<Number>()
		{
			return *( static_cast<Number*>( const_cast<void*>( p ) ) );
		}
		template<>
		String& Value::as<String>()
		{
			return *( static_cast<String*>( const_cast<void*>( p ) ) );
		}

		class Temporary
			: public std::vector<char>
		{
		public:
			Temporary()
			{
			}

			void append( char c )
			{
				this->push_back( c );
			}
			void append( const char* begin, const char* end )
			{
				this->insert( this->end(), begin, end );
			}

			void operator+=( char c )
			{
				this->push_back( c );
			}

			char* c_str()
			{
				this->push_back( '\0' );
				return this->data();
			}
		};

		class Parser
		{
		private:
			using Function = Value( Parser::* )( Reader& );
			using FunctionArray = Value( Parser::*[ 256 ] )( Reader& );
			Chunk memory;
			Allocator<char>	alloc;
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
				: alloc{ this->memory }
			{
				
			}

			Value parse( const char* begin, const char* end )
			{
				this->alloc.reset();
				Reader in{ begin, end };
				return this->parseValue( in );
			}

		private:
			Value parseNumber( Reader& in )
			{
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
				Temporary buffer;
				buffer.append( begin, in.getCurrent() );
				buffer.c_str();
				char* end;
				double *d = reinterpret_cast<double*>( this->alloc.allocate( sizeof( double ) ) );
				*d = std::strtod( buffer.c_str(), &end );
				return Value( Value::Type::Number, d );
			}
			Value parseObject( Reader& in )
			{
				Object* o = new( this->alloc.allocate( sizeof( Object ) ) ) Object();

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
								auto& k = key.as<String>();
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
				Array *a = new( this->alloc.allocate( sizeof( Array ) ) ) Array( 0, Value() );

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
				// "\uXXXX"�`���̕������ǂݍ���(UTF-8�ɕϊ�����)
				static auto parseUtf16 = []( Reader& in, Temporary& s )
				{
					// 16�i���\�L�̕������ǂݍ��݁A���l(8bit)�ɕϊ�����
					static auto readHex = []( Reader& in )
						-> char
					{
						// ������16�i���ŉ��߂��āA�Ή����鐔�l��Ԃ�
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

				Temporary buffer;
				const char* begin = in.getCurrent();
				for( char c = in.read();; c = in.read() )
				{
					switch( c )
					{
					case '\\':
						buffer.append( begin, in.getCurrent() - 1 );
						switch( in.read() )
						{
						case 'b': buffer += '\b'; break;
						case 'f': buffer += '\f'; break;
						case 't': buffer += '\t'; break;
						case 'r': buffer += '\r'; break;
						case 'n': buffer += '\n'; break;
						case 'u': parseUtf16( in, buffer ); break;
						default: buffer += in.getPrevious();
						}
						begin = in.getCurrent();
						break;
					case '"':
						buffer.append( begin, in.getCurrent() - 1 );
						std::string* s = new( this->alloc.allocate( sizeof( std::string ) ) ) std::string{ buffer.c_str() };
						return Value{ Value::Type::String, s };
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

#endif //NOTMOON_JSON_HPP