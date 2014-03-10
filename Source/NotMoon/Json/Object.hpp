#ifndef NOTMOON_JSON_OBJECT_HPP
#define NOTMOON_JSON_OBJECT_HPP
#include <NotMoon/Include.hpp>

#include <NotMoon/Json/Value.hpp>

namespace NotMoon
{
	namespace Json
	{
		class Object
			: public Map
		{
		public:
			Object( Allocator< Pair > c )
				: Map{ c }
			{
			}

			Value& operator[]( const String* key )
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

			const Value& operator[]( const String* key )
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
		Value::ResultType< Object > Value::as<Object>();
	}
}

#endif//NOTMOON_JSON_OBJECT_HPP