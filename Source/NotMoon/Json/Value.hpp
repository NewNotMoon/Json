#ifndef NOTMOON_JSON_VALUE_HPP
#define NOTMOON_JSON_VALUE_HPP

#include <vector>
#include <unordered_map>
#include "../Debug/Debug.hpp"
#include "Allocator.hpp"
#include "../Meta/Meta.hpp"

namespace NotMoon
{
	namespace Json
	{
		struct EqualCString
		{
			bool operator()( const char* l, const char* r )
			{
				return ( std::strcmp( l, r ) == 0 );
			}
		};

		struct HashCString
		{
			size_t operator()( const char* l )
			{
				static std::hash<std::string> hash;
				return hash( std::string{ l } );
			}
		};

		class Value;
		using Null			= unsigned int;
		using Boolean		= bool;
		using Number		= double;
		using String		= char*;
		using Pair			= std::pair< char*, Value >;
		using Map			= std::unordered_map< const char*, Value, HashCString, EqualCString, Allocator< Pair > >;
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
			Meta::IfPointerThenConstPointerElseConstLvalueReferense< T > as();
		};

		template<>
		Meta::IfPointerThenConstPointerElseConstLvalueReferense< Number > Value::as<Number>()
		{
			return *static_cast<Number*>( const_cast<void*>( p ) );
		}

		template<>
		Meta::IfPointerThenConstPointerElseConstLvalueReferense< String > Value::as<String>()
		{
			return static_cast<String>( const_cast<void*>( p ) );
		}
	}
}

#endif//NOTMOON_JSON_VALUE_HPP