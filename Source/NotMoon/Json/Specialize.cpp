#include <NotMoon/Json/Value.hpp>
#include <NotMoon/Json/Array.hpp>
#include <NotMoon/Json/Object.hpp>

using namespace NotMoon::Json;

template<>
Value::ResultType< Number > Value::as<Number>()
{
	return *static_cast<Number*>( const_cast<void*>( p ) );
}

template<>
Value::ResultType< String > Value::as<String>()
{
	return static_cast<String>( const_cast<void*>( p ) );
}

template<>
Value::ResultType< Object > Value::as<Object>()
{
	return *static_cast<Object*>( const_cast<void*>( p ) );
}

template<>
Value::ResultType< Array > Value::as<Array>()
{
	return *static_cast<Array*>( const_cast<void*>( p ) );
}