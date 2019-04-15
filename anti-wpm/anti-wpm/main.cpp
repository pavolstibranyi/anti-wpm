#include <Windows.h>

#include "magic.h"

auto main( ) -> int
{
	pvar<int> x = 0;

	while( !GetAsyncKeyState( VK_SPACE ) )
	{
		++x;
		system( "cls" );
		printf( "x:	%d\n" , ( int ) x );
		Sleep( 10 );
	}

	return 0;
}
