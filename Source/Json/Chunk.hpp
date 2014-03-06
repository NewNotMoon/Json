#ifndef NOTMOON_JSON_CHUNK_HPP
#define NOTMOON_JSON_CHUNK_HPP

#include <vector>
#include "../Debug/Debug.hpp"

namespace NotMoon
{
	namespace Json
	{
		// �`�����N(�������u���b�N)��ێ�����\����
		struct Chunk
		{
			std::vector<char*> chunks;	// �`�����N�̔z��
			size_t maxSize;			// ��ԑ傫���`�����N(= chunks.back())�̃T�C�Y
			size_t offset;			// ���g�p�̃������J�n�ʒu

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