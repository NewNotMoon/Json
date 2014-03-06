#ifndef NOTMOON_JSON_CHUNK_HPP
#define NOTMOON_JSON_CHUNK_HPP

#include <vector>
#include "../Debug/Debug.hpp"

namespace NotMoon
{
	namespace Json
	{
		// チャンク(メモリブロック)を保持する構造体
		struct Chunk
		{
			std::vector<char*> chunks;	// チャンクの配列
			size_t maxSize;			// 一番大きいチャンク(= chunks.back())のサイズ
			size_t offset;			// 未使用のメモリ開始位置

			Chunk( unsigned init_size = 0x8000 )
				: offset( 0 )
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
	}
}

#endif//NOTMOON_JSON_CHUNK_HPP