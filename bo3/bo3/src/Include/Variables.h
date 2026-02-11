#pragma once
#include <Include.h>

class Variables {
public:
    struct {
        bool m_IsNoRecoil = false; 
    }Weapon;
};

inline std::unique_ptr<Variables> g_Variables = std::make_unique<Variables>( ); 