
#include "../NotMoon/Json.hpp"
#include <fstream>
#include <iostream>
#include <string>

int main()
{
	NotMoon::Debug d;
	try
	{
		NOTMOON_THROW( NotMoon::Json::ParseErrorException, "test" );
	}
	catch( const NotMoon::Json::ParseErrorException& e )
	{
#if defined( NOTMOON_EXCEPTION_DETAIL )
		std::cout
			<< "File: " << e.file << std::endl
			<< "Line: " << e.line << std::endl
			<< "Func: " << e.func << std::endl
			<< "Message: " << e.message << std::endl;
#endif
	}
	using namespace NotMoon;
	Json::Parser parser;
	std::ifstream f{ "Sample/Sample.json" };
	std::vector<char> buffer;
	buffer.resize( static_cast<unsigned int>( f.seekg( 0, std::ios::end ).tellg() ) );
	f.seekg( 0, std::ios::beg ).read( &buffer[ 0 ], static_cast<std::streamsize>( buffer.size() ) );
	auto root = parser.parse( &buffer[ 0 ], &buffer[ 0 ] + buffer.size() );
	auto arr = *root.as<Json::Array>();
	auto obj = *arr[ 0 ].as<Json::Object>();
	auto str = obj[ "created_at" ].as<Json::String>();
	std::cout << str;
#ifndef _DEBUG
	std::cin >> std::string{};
#endif
}