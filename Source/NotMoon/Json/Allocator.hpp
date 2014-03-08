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
			pointer allocate( size_t num )
			{
				if( this->cnk.offset + sizeof(T)* num >= this->cnk.maxSize )
				{
					// ������������Ȃ��ꍇ�́Achunk�𑝂₷
					this->cnk.maxSize *= 2;
					this->cnk.chunks.push_back( NOTMOON_NEW char[ this->cnk.maxSize ] );
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
			void deallocate( pointer p, size_t num )
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
	}
}

#endif//NOTMOON_JSON_ALLOCATOR_HPP