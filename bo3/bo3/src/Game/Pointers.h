#pragma once
#include <Include.h>

class Pointers {
public:
    Pointers( );
    ~Pointers( ) = default;

    using ExecuteSingleCommandFn = uintptr_t( __fastcall* )(int controller, int client, const char* command, bool isReliable);
    using SendModifiedStatsFn = uintptr_t( __fastcall* )(int controller, int statsGroup);
    using AddReliableCommandFn = uintptr_t( __fastcall* )(int controller, const char* command);
    using CbufAddTextFn = std::uintptr_t( __fastcall* )(unsigned int controller, const char* text);

    ExecuteSingleCommandFn ExecuteSingleCommand{ nullptr };
    SendModifiedStatsFn    SendModifiedStats{ nullptr };
    AddReliableCommandFn   AddReliableCommand{ nullptr };
    CbufAddTextFn   CbufAddText{ nullptr };

    void Initialize( ); 
};

inline std::unique_ptr<Pointers> g_Pointers; 