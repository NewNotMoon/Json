
#include "../Json/Json.hpp"
#include <fstream>
#include <iostream>
#include <string>

int main()
{
	using namespace NotMoon;
	Json::Parser parser;
	std::ifstream f{ "Test/test.json" };
	std::vector<char> buffer;
	buffer.resize( f.seekg( 0, std::ios::end ).tellg() );
	f.seekg( 0, std::ios::beg ).read( &buffer[ 0 ], static_cast<std::streamsize>( buffer.size() ) );
	auto root = parser.parse( &buffer[ 0 ], &buffer[ 0 ] + buffer.size() );
	auto arr = root.as<Json::Array>();
	auto obj = arr[ 0 ].as<Json::Object>();
	auto str = obj[ "created_at" ].as<Json::String>();
	std::cout << str;
	std::cin >> std::string{};
}