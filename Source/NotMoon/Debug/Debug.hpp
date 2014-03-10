#ifndef NOTMOON_DEBUG_DEBUG_HPP
#define NOTMOON_DEBUG_DEBUG_HPP
#include <NotMoon/Include.hpp>

#include <iostream>
#if	defined( BOOST_COMP_MSVC )
#	include <Windows.h>
#	include <tchar.h>
#	if defined( _DEBUG )
#		define _CRTDBG_MAP_ALLOC
#		include <crtdbg.h>
#		define NOTMOON_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#		define NOTMOON_DEBUG
#		define NOTMOON_DEBUG_MESSAGE( mes ) OutputDebugString( _T(mes) ); OutputDebugString( _T("\n") );
#	else
#		define NOTMOON_NEW new
#		define NOTMOON_DEBUG_MESSAGE( mes )
#	endif
#endif

namespace NotMoon
{
#if defined( NOTMOON_DEBUG )
	class DebugStreambuf
		: public std::streambuf
	{
		char buffer[ 1024 ];
	public:
		DebugStreambuf()
		{
			setp( buffer, buffer+1024 );
			setg( buffer, buffer, buffer+1024 );
		}
		virtual std::streampos seekoff(
			std::streamoff off,
			std::ios::seek_dir dir,
			std::ios::openmode nMode = std::ios::in | std::ios::out )
		{
			return EOF;
		}
		virtual std::streampos seekpos(
			std::streampos pos,
			std::ios::openmode nMode = std::ios::in | std::ios::out )
			override
		{
			return EOF;
		}
		virtual int sync()
			override
		{
			*pptr() = '\0';    // 終端文字を追加します。
			DWORD size;
			WriteConsoleA( GetStdHandle( STD_OUTPUT_HANDLE ), buffer, static_cast<DWORD>( pptr() - pbase() ), &size, NULL );
			OutputDebugStringA( buffer );
			pbump( static_cast<int>( pbase() - pptr() ) );
			return 0;
		}
		virtual int_type overflow( int_type c = EOF )
			override
		{
			if( c != EOF )
			{
				char buf[ ] ={ c, '\0' };
				DWORD size;
				WriteConsoleA( GetStdHandle( STD_OUTPUT_HANDLE ), buffer, 1, &size, NULL );
				OutputDebugStringA( buffer );
			}
			return c;
		}
		virtual int underflow()
			override
		{
			return EOF;
		}
	};

	class DebugStreambufWide
		: public std::wstreambuf
	{
		wchar_t buffer[ 1024 ];
	public:
		DebugStreambufWide()
		{
			setp( buffer, buffer + 1024 );
			setg( buffer, buffer, buffer + 1024 );
		}
		virtual std::wstreampos seekoff(
			std::streamoff off,
			std::ios::seek_dir dir,
			std::ios::openmode nMode = std::ios::in | std::ios::out )
		{
			return EOF;
		}
		virtual std::wstreampos seekpos(
			std::wstreampos pos,
			std::ios::openmode nMode = std::ios::in | std::ios::out )
			override
		{
			return EOF;
		}
		virtual int sync()
			override
		{
			*pptr() = L'\0';    // 終端文字を追加します。
			DWORD size;
			WriteConsoleW( GetStdHandle( STD_OUTPUT_HANDLE ), buffer, static_cast<DWORD>( pptr() - pbase() ), &size, NULL );
			OutputDebugStringW( buffer );
			pbump( static_cast<int>( pbase() - pptr() ) );    // 書き込み位置をリセットします。
			return 0;
		}
		virtual int_type overflow( int_type c = EOF )
			override
		{
			if( c != EOF )
			{
				wchar_t buf[ ] ={ c, '\0' };
				DWORD size;
				WriteConsoleW( GetStdHandle( STD_OUTPUT_HANDLE ), buf, 1, &size, NULL );
				OutputDebugStringW( buf );
			}
			return c;
		}
		virtual wint_t underflow()
			override
		{
			return EOF;
		}
	};

	using MemoryState = _CrtMemState;

	class Debug
	{
	private:
		MemoryState data;
		DebugStreambuf stream;
		DebugStreambufWide streamWide;
		std::streambuf *oldstream;
		std::wstreambuf *oldstreamWide;

	public:
		Debug()
		{
			::_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
			this->oldstream = std::cout.rdbuf( &this->stream );
			this->oldstreamWide = std::wcout.rdbuf( &this->streamWide );
		}
		~Debug()
		{
			this->capture();
			this->dumpMemory( this->data );
			std::cout.rdbuf( this->oldstream );
			std::wcout.rdbuf( this->oldstreamWide );
		}

		MemoryState capture()
		{
			_CrtMemCheckpoint( &this->data );
			return this->data;
		}

		static void dumpMemory( const MemoryState& d )
		{
			std::cout << std::endl << "[DUMP HEAP MEMORY]" << std::endl;
			std::cout << "_NORMAL_BLOCK" << std::endl
				<< "\tCount: " << d.lCounts[ _NORMAL_BLOCK ] << std::endl
				<< "\tSize: " << d.lSizes[ _NORMAL_BLOCK ] << " byte." << std::endl;
			std::cout << "_CRT_BLOCK" << std::endl
				<< "\tCount: " << d.lCounts[ _CRT_BLOCK ] << std::endl
				<< "\tSize: " << d.lSizes[ _CRT_BLOCK ] << " byte." << std::endl;
			std::cout << "_CLIENT_BLOCK" << std::endl
				<< "\tCount: " << d.lCounts[ _CLIENT_BLOCK ] << std::endl
				<< "\tSize: " << d.lSizes[ _CLIENT_BLOCK ] << " byte." << std::endl;
			std::cout << "_FREE_BLOCK" << std::endl
				<< "\tCount: " << d.lCounts[ _FREE_BLOCK ] << std::endl
				<< "\tSize: " << d.lSizes[ _FREE_BLOCK ] << " byte." << std::endl;
			std::cout << "_IGNORE_BLOCK" << std::endl
				<< "\tCount: " << d.lCounts[ _IGNORE_BLOCK ] << std::endl
				<< "\tSize: " << d.lSizes[ _IGNORE_BLOCK ] << " byte." << std::endl;

			std::cout << std::endl << "TotalCount: " << d.lTotalCount << " byte." << std::endl;
		}
	};
#else
	class Debug
	{
	};
#endif
}

#endif