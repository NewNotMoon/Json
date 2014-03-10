//===============================================================================================================================
//!	
//!	@file	Sample/Json.hpp
//!	
//!		NotMoon::Json サンプルソース
//!	
//!		The author is a high school student of Japanese. Therefore is English is unskilled. 
//!		There may be a strange translation. When there is a mistranslation. Please you point out any mistakes to me.
//!	
//!	Copyright (C) 2014- 新々月 All Rights Reserved.
//===============================================================================================================================
#include <iostream>
#include <string>

void outputUtf8( std::string& str );

// NotMoon/Json.hppをインクルードしてください。
// Please Include the NotMoon/Json.hpp.
#include <NotMoon/Json.hpp>

int excuteSample( const std::vector<char>& buffer )
{
	using namespace NotMoon::Json;

	// 全ての操作にはまずParserクラスのインスタンスが必要です。
	// Instance of the Parser class is required in the first operation of all.
	Parser parser;

	// Parser::parse関数によりパース及びルート要素の取得。
	// Get the root element and parse by Parser::parse function.
	auto& root = parser.parse( &buffer[ 0 ], &buffer[ 0 ] + buffer.size() );

	// Value::getType関数により要素の種類を取得可能。値はValue::Type列挙型。
	// You can acquire by Value::getType function the kind of value. It is Value::Type enumerated type.
	if( root.getType() == Value::Type::Array )
	{
		// 要素はValue::as<T>関数により変換する。Value::as<T>関数は文字列型を除き参照を返す。
		// You can converted by the Value::as<T> function. Value::as<T> function returns a reference except for the string type.
		auto& arr = root.as<Array>();

		// 配列は添字アクセス可能。
		// Array is accessible by subscript.
		auto& obj = arr[ 0 ].as<Object>();

		// オブジェクトも添字アクセス可能。文字列のみNUL終端するポインタで返るため注意。
		// Object also accessible by subscript. Please note because the string only returns with the null-terminated string.
		auto str = obj[ "created_at" ].as<String*>();

		// \uXXXX形式の文字列もパース可能
		auto txt = obj[ "text" ].as<String*>();

		// 数値はdouble型。
		// Numbers is double precision real number type.
		auto& num = obj[ "number" ].as<Number>();

		std::cout << num << std::endl;
		outputUtf8( std::string{ txt } );
		outputUtf8( std::string{ str } );
	}
	//! 各要素の寿命はParserクラスのインスタンスと同じ。以後のアクセスは保証されない。
	//! Life of each element is the same as the instance of the Parser class. Future access is not guaranteed.
	return 0;
};

#include <fstream>

int main()
{
	std::locale::global( std::locale( "japanese" ) );
	NotMoon::Debug d;
	try
	{
		NOTMOON_THROW( NotMoon::Json::ParseErrorException, "test" );
	}
	catch( const NotMoon::Json::ParseErrorException& e )
	{
#if defined( NOTMOON_EXCEPTION_DETAIL )
		std::cout
			<< "\tFile: " << e.file << std::endl
			<< "\tLine: " << e.line << std::endl
			<< "\tFunc: " << e.func << std::endl
			<< "\tMessage: " << e.message << std::endl;
#endif
	}
	std::ifstream f{ "Sample/Sample.json" };
	if( f.fail() )
	{
		std::cout << "Can not open file.";
		std::cin >> std::string{};
		return -1;
	}
	std::vector<char> buffer;
	buffer.resize( static_cast<unsigned int>( f.seekg( 0, std::ios::end ).tellg() ) );
	f.seekg( 0, std::ios::beg ).read( &buffer[ 0 ], static_cast<std::streamsize>( buffer.size() ) );
	
	excuteSample( buffer );

	std::cin >> std::string{};
#ifndef _DEBUG
#endif
}

void outputUtf8( std::string& str )
{
	unsigned char lead;
	int char_size = 0;

	for( std::string::iterator it = str.begin(); it != str.end(); it += char_size )
	{
		lead = *it;

		if( lead < 0x80 )
		{
			char_size = 1;
			std::cout << str.substr( distance( str.begin(), it ), char_size );
		}
		else if( lead < 0xE0 )
		{
			char_size = 2;
			std::cout << str.substr( distance( str.begin(), it ), char_size );
		}
		else if( lead < 0xF0 )
		{
			char_size = 3;
			char* s = &*it;
			wchar_t u[ 2 ];
			u[ 0 ] = ( ( s[ 0 ] & 0x0f ) << 12 )
					| ( ( s[ 1 ] & 0x3f ) << 6 )
					| ( s[ 2 ] & 0x3f );
			u[ 1 ] = L'\0';
			std::wcout << u[ 0 ] << std::flush;
		}
		else
		{
			char_size = 4;
			std::cout << str.substr( distance( str.begin(), it ), char_size );
		}
	}
	std::cout << std::endl;
};