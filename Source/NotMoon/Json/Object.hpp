#ifndef NOTMOON_JSON_OBJECT_HPP
#define NOTMOON_JSON_OBJECT_HPP

#include "Value.hpp"

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

			Value& operator[]( const char* key )
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

			const Value& operator[]( const char* key )
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
		Meta::IfPointerThenConstPointerElseConstLvalueReferense< Object > Value::as<Object>()
		{
			return *static_cast<Object*>( const_cast<void*>( p ) );
		}
	}
}

#endif//NOTMOON_JSON_OBJECT_HPP