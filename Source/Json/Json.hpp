#ifndef NOTMOON_JSON_HPP
#define NOTMOON_JSON_HPP

#include <vector>
#include <unordered_map>
#include <boost/predef.h>

namespace NotMoon
{
	extern const char* nullString = "";
	struct Exception
		: public std::exception
	{
	public:
		using Line = decltype( __LINE__ );

	public:
		const char* file;
		const Line line;
		const char* func;
		const char* message;

	public:
		Exception()
			: file		{ nullString }
			, line		{ 0 }
			, func		{ nullString }
			, message	{ nullString }
		{
		}
		Exception
		(
			const char* file,
			const Line line,
			const char* func,
			const char* message
		)
			: file		{ file }
			, line		{ line }
			, func		{ func }
			, message	{ message }
		{
		}
		virtual ~Exception()
		{
		}

	public:
		virtual const char* what()
		{
			return this->message;
		}
	};
}

#if defined( _DEBUG )
#	if defined( BOOST_COMP_MSVC )
#		define NOTMOON_FUNCTION_NAME __FUNCTION__
#	elif defined( BOOST_COMP_GNUC )
#		define NOTMOON_FUNCTION_NAME __PRETTY_FUNCTION__
#	else
#		define NOTMOON_FUNCTION_NAME __func__
#	endif
#	if !defined( NOTMOON_EXCEPTION_DETAIL )
#		define NOTMOON_EXCEPTION_DETAIL
#	endif
#endif
#if defined( NOTMOON_EXCEPTION_DETAIL )
#	define NOTMOON_EXCEPTION_CONSTRUCTOR( name ) \
public: \
	name \
	( \
		const char* file, \
		const Line line, \
		const char* func, \
		const char* message \
	) \
		: Exception{ file, line, func, message } \
	{ \
	}
#	define NOTMOON_THROW( type, msg ) throw type{ __FILE__, __LINE__, NOTMOON_FUNCTION_NAME, msg }
#else
#	define NOTMOON_THROW( type, msg ) throw type{}
#	define NOTMOON_EXCEPTION_CONSTRUCTOR( name ) public: name() = default;
#endif

namespace NotMoon
{
	class ParseErrorException
		: public Exception
	{
		NOTMOON_EXCEPTION_CONSTRUCTOR( ParseErrorException );
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

		// チャンク(メモリブロック)を保持する構造体
		struct Chunk
		{
			std::vector<char*> chunks;	// チャンクの配列
			unsigned maxSize;			// 一番大きいチャンク(= chunks.back())のサイズ
			unsigned offset;			// 未使用のメモリ開始位置

			Chunk( unsigned init_size = 0x8000 )
				:offset( 0 )
				, maxSize( init_size )
			{
				chunks.push_back( NOTMOON_NEW char[ maxSize ] );
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

			// アロケータをU型にバインドする
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

			// STLのアロケータクラスに必要なメソッド群
			// メモリを割り当てる
			pointer allocate( unsigned num )
			{
				if( this->cnk.offset + sizeof(T)* num >= this->cnk.maxSize )
				{
					// メモリが足りない場合は、chunkを増やす
					this->cnk.maxSize *= 2;
					this->cnk.chunks.push_back( NOTMOON_NEW char[ this->cnk.maxSize ] );
					this->cnk.offset=0;

					return allocate( num ); // もう一度allocate
				}

				// offsetから、sizeof(T)*num分だけ、メモリを割り当てる
				// XXX: ここでコンストラクタを呼び出すのは間違っている気がする(2009/11/19)
				pointer ptr = new( this->cnk.chunks.back() + this->cnk.offset ) T[ num ];
				this->cnk.offset += sizeof(T)*num;
				return ptr;
			}

			// 割当て済みの領域を初期化する
			void construct( pointer p, const T& value )
			{
				new( (void*)p ) T( value );
			}

			// メモリを解放する
			// ※ 何も行わない
			void deallocate( pointer p, unsigned num )
			{
			}

			// 初期化済みの領域を削除する
			void destroy( pointer p )
			{
				p->~T();
			}


			// JSONに必要なメソッド群
			// 未使用メモリ開始位置(offset)を0にリセットする
			void reset()
			{
				this->cnk.offset = 0;
				if( this->cnk.chunks.size() > 1 )
				{
					// チャンクが複数ある場合は、一番最後のもの以外はdeleteする
					char *tmp = this->cnk.chunks.back();
					for( size_t i = 0; i < this->cnk.chunks.size() - 1; ++i )
					{
						delete[ ] this->cnk.chunks[ i ];
					}
					this->cnk.chunks.clear();
					this->cnk.chunks.push_back( tmp );
				}
			}

			void setTail( char* ptr )
			{
				this->cnk.offset = ptr - this->cnk.chunks.back();
			}

			// 引数のポインタ(ptr)が、あらかじめ確保しているメモリブロックの範囲内かどうかを判定する
			// 範囲外なら、trueを返す
			// ※ 上限のみをチェックしている
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

		struct EqualCString
		{
			bool operator()( const char* l, const char* r )
			{
				return ( std::strcmp( l, r ) == 0 );
			}
		};

		struct HashCString
		{
			size_t operator()( const char* l )
			{
				static std::hash<std::string> hash;
				return hash( std::string{ l } );
			}
		};

		class Value;
		using Null			= unsigned int;
		using Boolean		= bool;
		using Number		= double;
		using String		= char;
		using StringImpl	= char*;
		using Pair			= std::pair< char*, Value >;
		using Map			= std::unordered_map< const char*, Value, HashCString, EqualCString, Allocator< Pair > >;
		using Vector		= std::vector< Value, Allocator< Value > >;
		class Array;
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
			T* as()
			{
				return static_cast<const T*>( this->p );
			}
		};

		class Array
			: public Vector
		{
		public:
			Array( Allocator< Value > c )
				: Vector{ c }
			{
			}
		};


		class Object
			: public Map
		{
		public:
			Object( Allocator< Pair > c )
				: Map{ c }
			{
			}

			Value& operator[]( const std::string& key )
			{
				static Value undefined;
				auto it = this->find( key.c_str() );
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

				auto it = this->find( key.c_str() );
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
		Array* Value::as<Array>()
		{
			return static_cast<Array*>( const_cast<void*>( p ) );
		}
		template<>
		Object* Value::as<Object>()
		{
			return static_cast<Object*>( const_cast<void*>( p ) );
		}
		template<>
		Number* Value::as<Number>()
		{
			return static_cast<Number*>( const_cast<void*>( p ) );
		}
		template<>
		StringImpl Value::as<String>()
		{
			return static_cast<String*>( const_cast<void*>( p ) );
		}

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

#endif //NOTMOON_JSON_HPP