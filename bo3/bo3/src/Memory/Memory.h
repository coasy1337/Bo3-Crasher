#pragma once
#include <Include.h>
#include "PEB_Internal.h"

class Memory {
public:
	std::wstring charToWchar( const char* str );
	uintptr_t GetModuleBase( const wchar_t* moduleName );
	uint64_t ReturnScan( uint64_t pModuleBaseAddress, const char* sSignature, size_t nSelectResultIndex = 0 );
	uint64_t ResolveRelative( uint64_t Address, int InstructionLength );
	uint64_t FindPattern( const char* szModuleName, const char* sSignature, int InstructionLength = 0 );

	uintptr_t Resolve( const char* szModuleName, int64_t targetValue, BYTE compareByte );
	DWORD_PTR GetProcBase( DWORD procId );
};

inline std::unique_ptr<Memory> g_Memory = std::make_unique<Memory>( );