#ifndef NOTMOON_DEBUG_DEBUG_HPP
#define NOTMOON_DEBUG_DEBUG_HPP

// あれ　デバッグ関連のアレ
// NotMoonは全てこのヘッダいんくるーどしてね

#include <iostream>
#include <boost/predef.h>
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

namespace NotMoon
{
#if defined( _DEBUG )
	class DebugStreambuf
		: public std::streambuf
	{
	public:
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
		virtual int_type overflow( int_type c = EOF )
			override
		{
			if( c != EOF )
			{
				char buf[ ] ={ c, '\0' };
				DWORD size;
				WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), buf, 1, &size, NULL );
				OutputDebugStringA( buf );
			}
			return c;
		}
		virtual int underflow()
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
		std::streambuf *oldstream;

	public:
		Debug()
		{
			::_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
			oldstream = std::cout.rdbuf( &this->stream );
		}
		~Debug()
		{
			this->capture();
			this->dumpMemory( this->data );
			std::cout.rdbuf( this->oldstream );
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

#endif