#ifndef NOTMOON_EXCEPTION_EXCEPTION_HPP
#define NOTMOON_EXCEPTION_EXCEPTION_HPP
#include <NotMoon/Include.hpp>

#include <exception>

namespace NotMoon
{
	static const char* nullString = "";
	struct Exception
		: public std::exception
	{
	public:
		using Line = decltype( __LINE__ );

	public:
		const char* file;
		const Line line;
		const char* func;
		const char* message;

	public:
		Exception()
			: file		{ nullString }
			, line		{ 0 }
			, func		{ nullString }
			, message	{ nullString }
		{
		}
		Exception
		(
			const char* file,
			const Line line,
			const char* func,
			const char* message
		)
			: file		{ file }
			, line		{ line }
			, func		{ func }
			, message	{ message }
		{
		}
		virtual ~Exception()
		{
		}

	public:
		virtual const char* what()
		{
			return this->message;
		}
	};
}

#if defined( _DEBUG )
#	if defined( BOOST_COMP_MSVC )
#		define NOTMOON_FUNCTION_NAME __FUNCTION__
#	elif defined( BOOST_COMP_GNUC )
#		define NOTMOON_FUNCTION_NAME __PRETTY_FUNCTION__
#	else
#		define NOTMOON_FUNCTION_NAME __func__
#	endif
#	if !defined( NOTMOON_EXCEPTION_DETAIL )
#		define NOTMOON_EXCEPTION_DETAIL
#	endif
#endif
#if defined( NOTMOON_EXCEPTION_DETAIL )
#	define NOTMOON_EXCEPTION_CONSTRUCTOR( name ) \
public: \
	name \
	( \
		const char* file, \
		const Line line, \
		const char* func, \
		const char* message \
	) \
		: Exception{ file, line, func, message } \
	{ \
	}
#	define NOTMOON_THROW( type, msg ) throw type{ __FILE__, __LINE__, NOTMOON_FUNCTION_NAME, msg }
#else
#	define NOTMOON_THROW( type, msg ) throw type{}
#	define NOTMOON_EXCEPTION_CONSTRUCTOR( name ) public: name() = default;
#endif

#endif//NOTMOON_EXCEPTION_EXCEPTION_HPP