#include "Setup.h"

Setup::Setup( ) {
	if (m_IsDebug) { g_Logger->CreateConsole( ); }
	this->Initialize( );
}

void Setup::Initialize( )
{
	system( "cls" );
	g_Logger->Log( Info, "Setup.." );
	
	g_Pointers = std::make_unique<Pointers>( ); 

	g_Hooking = std::make_unique<Hooking>( ); 
}
