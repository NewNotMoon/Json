#ifndef NOTMOON_JSON_ARRAY_HPP
#define NOTMOON_JSON_ARRAY_HPP
#include <NotMoon/Include.hpp>

#include <NotMoon/Json/Value.hpp>

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
		Value::ResultType< Array > Value::as<Array>();
	}
}

#endif//NOTMOON_JSON_ARRAY_HPP