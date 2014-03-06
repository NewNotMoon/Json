#ifndef NOTMOON_JSON_ARRAY_HPP
#define NOTMOON_JSON_ARRAY_HPP

#include "Value.hpp"

namespace NotMoon
{
	namespace Json
	{
		class Array
			: public Vector
		{
		public:
			Array( Allocator< Value > c )
				: Vector{ c }
			{
			}
		};

		template<>
		Meta::IfPointerThenConstPointerElseConstLvalueReferense< Array > Value::as<Array>()
		{
			return *static_cast<Array*>( const_cast<void*>( p ) );
		}
	}
}

#endif//NOTMOON_JSON_ARRAY_HPP