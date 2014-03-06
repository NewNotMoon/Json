#ifndef NOTMOON_META_META_HPP
#define NOTMOON_META_META_HPP

#include <type_traits>

namespace NotMoon
{
	namespace Meta
	{
		template < bool, typename True, typename False >
		struct If;

		template < typename True, typename False >
		struct If< true, True, False >
		{
			using Type = True;
		};

		template < typename True, typename False >
		struct If< false, True, False >
		{
			using Type = False;
		};

		template < typename T >
		using IfPointerThenConstPointerElseConstLvalueReferense =
			typename If
			<
				std::is_pointer< T >::value,
				typename std::add_const< T >::type,
				typename std::add_const
				<
					typename std::add_lvalue_reference< T >::type
				>::type
			>::Type;
	}
}

#endif//NOTMOON_META_META_HPP