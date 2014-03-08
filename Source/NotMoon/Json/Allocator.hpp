#ifndef NOTMOON_JSON_ALLOCATOR_HPP
#define NOTMOON_JSON_ALLOCATOR_HPP
#include <NotMoon/Include.hpp>

#include <NotMoon/Json/Chunk.hpp>

namespace NotMoon
{
	namespace Json
	{
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
			pointer allocate( size_t num )
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
			void deallocate( pointer p, size_t num )
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
						delete[] this->cnk.chunks[ i ];
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
	}
}

#endif//NOTMOON_JSON_ALLOCATOR_HPP