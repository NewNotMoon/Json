#ifndef NOTMOON_JSON_EXCEPTION_HPP
#define NOTMOON_JSON_EXCEPTION_HPP
#include <NotMoon/Include.hpp>

#include <NotMoon/Exception/Exception.hpp>

namespace NotMoon
{
	namespace Json
	{
		class ParseErrorException
			: public Exception
		{
			NOTMOON_EXCEPTION_CONSTRUCTOR( ParseErrorException );
		};
	}
}

#endif//NOTMOON_JSON_EXCEPTION_HPP