//===============================================================================================================================
//!	
//!	@file	Sample/Json.hpp
//!	
//!		NotMoon::Json �T���v���\�[�X
//!	
//!		The author is a high school student of Japanese. Therefore is English is unskilled. 
//!		There may be a strange translation. When there is a mistranslation. Please you point out any mistakes to me.
//!	
//!	Copyright (C) 2014- �V�X�� All Rights Reserved.
//===============================================================================================================================
#include <iostream>

// NotMoon/Json.hpp���C���N���[�h���Ă��������B
// Please Include the NotMoon/Json.hpp.
#include <NotMoon/Json.hpp>

void excuteSample( const std::vector<char>& buffer )
{
	using namespace NotMoon::Json;

	// �S�Ă̑���ɂ͂܂�Parser�N���X�̃C���X�^���X���K�v�ł��B
	// Instance of the Parser class is required in the first operation of all.
	Parser parser;

	// Parser::parse�֐��ɂ��p�[�X�y�у��[�g�v�f�̎擾�B
	// Get the root element and parse by Parser::parse function.
	auto& root = parser.parse( &buffer[ 0 ], &buffer[ 0 ] + buffer.size() );

	// Value::getType�֐��ɂ��v�f�̎�ނ��擾�\�B�l��Value::Type�񋓌^�B
	// You can acquire by Value::getType function the kind of value. It is Value::Type enumerated type.
	if( root.getType() == Value::Type::Array )
	{
		// �v�f��Value::as<T>�֐��ɂ��ϊ�����BValue::as<T>�֐��͕�����^�������Q�Ƃ�Ԃ��B
		// You can converted by the Value::as<T> function. Value::as<T> function returns a reference except for the string type.
		auto& arr = root.as<Array>();

		// �z��͓Y���A�N�Z�X�\�B
		// Array is accessible by subscript.
		auto& obj = arr[ 0 ].as<Object>();

		// �I�u�W�F�N�g���Y���A�N�Z�X�\�B������̂�NUL�I�[����|�C���^�ŕԂ邽�ߒ��ӁB
		// Object also accessible by subscript. Please note because the string only returns with the null-terminated string.
		auto str = obj[ "created_at" ].as<String>();

		// ���l��double�^�B
		// Numbers is double precision real number type.
		auto& num = obj[ "number" ].as<Number>();

		std::string txt{ obj[ "text" ].as<String>() };
		std::cout << str << std::endl;
		std::cout << num << std::endl;
		for( auto& i : txt )
		{
			std::cout << i << std::endl;
		}
	}
	//! �e�v�f�̎�����Parser�N���X�̃C���X�^���X�Ɠ����B�Ȍ�̃A�N�Z�X�͕ۏ؂���Ȃ��B
	//! Life of each element is the same as the instance of the Parser class. Future access is not guaranteed.
};

#include <fstream>
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

#ifndef _DEBUG
	std::cin >> std::string{};
#endif
}