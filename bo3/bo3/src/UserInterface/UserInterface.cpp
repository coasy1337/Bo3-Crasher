#include "UserInterface.h"

void UserInterface::Render( ) {
    if (GetAsyncKeyState( VK_DELETE ) & 1)
        m_IsOpened = !m_IsOpened;

    if (m_IsOpened) {
        ImGui::Begin( "Menu", nullptr ); {
            if (ImGui::Button( "Crash All" )) {
                g_Pointers->ExecuteSingleCommand( 0, 0, "callvote map \"^H\xff\xff\xff\loser\"", false ); 
            }

            if (ImGui::Button( "Crash Server" )) {
                g_Pointers->SendModifiedStats( 0, 1 );
            }

            if (ImGui::Button( "Disconnect" )) {
                g_Pointers->CbufAddText( 0, "disconnect" );
            }

            if (ImGui::Button( "End Round" )) {
                char buf[255];
                __int64 serverId = (uintptr_t)GetModuleHandleA( 0 ) + 0x569D250;
                sprintf_s( buf, "mr %d -1 endround", *(DWORD*)serverId );
                g_Pointers->AddReliableCommand( 0, buf );
            }
        }
        ImGui::End( );
    }
}