#pragma once
#include <Include.h>

class UserInterface {
public:
    void Render( ); 
    bool m_IsOpened; 
};

inline std::unique_ptr<UserInterface> g_UserInterface = std::make_unique<UserInterface>( );