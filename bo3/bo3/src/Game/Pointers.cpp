#include "Pointers.h"

Pointers::Pointers( ) {
    Initialize( ); 
}

void Pointers::Initialize( )
{
    const auto moduleHandle = ::GetModuleHandleA( nullptr );
    if (!moduleHandle)
        return;

    const auto baseAddress = reinterpret_cast<uintptr_t>(moduleHandle);

    ExecuteSingleCommand = reinterpret_cast<ExecuteSingleCommandFn>(baseAddress + 0x20EDC20);
    SendModifiedStats = reinterpret_cast<SendModifiedStatsFn>(baseAddress + 0x1ED42A0);
    AddReliableCommand = reinterpret_cast<AddReliableCommandFn>(baseAddress + 0x135D4C0);
    CbufAddText = reinterpret_cast<CbufAddTextFn>(baseAddress + 0x135D4C0);
}