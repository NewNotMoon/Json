#ifndef NOTMOON_JSON_VALUE_HPP
#define NOTMOON_JSON_VALUE_HPP
#include <NotMoon/Include.hpp>

#include <NotMoon/Json/Alias.hpp>
#include <NotMoon/Json/Allocator.hpp>
#include <NotMoon/Meta/Meta.hpp>

#include <vector>
#include <unordered_map>

namespace NotMoon
{
	namespace Json
	{
		struct EqualCString
		{
			bool operator()( const String* l, const String* r )
			{
				return ( std::strcmp( (char *)l, (char *)r ) == 0 );
			}
		};

		struct HashCString
		{
			size_t operator()( const String* l )
			{
				static std::hash<std::string> hash;
				return hash( std::string{ l } );
			}
		};

		class Value;
		using Pair			= std::pair< String*, Value >;
		using Map			= std::unordered_map< const String*, Value, HashCString, EqualCString, Allocator< Pair > >;
		using Vector		= std::vector< Value, Allocator< Value > >;
		class Array;
		class Object;

		class Value
		{
		public:
			enum class Type
				: short
			{
				Undefined,
				Null,
				Boolean,
				Number,
				String,
				Array,
				Object,
			};

		private:
			const void* p;
			Type  type;

		public:
			Value()
				: type{ Type::Undefined }
				, p{ nullptr }
			{
			}
			Value( Type t, const void* p )
				: type{ t }
				, p{ p }
			{
			}

			Type getType()
				const
			{
				return this->type;
			}

		public:
			template< typename T >
			using ResultType = Meta::IfPointerThenConstPointerElseConstLvalueReferense< T >;

			template< typename T >
			ResultType< T > as();
		};

		template<>
		Value::ResultType< Number > Value::as<Number>();

		template<>
		Value::ResultType< String* > Value::as<String*>();
	}
}

#endif//NOTMOON_JSON_VALUE_HPP