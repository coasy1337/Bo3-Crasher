#include "memory.h"
#include <tchar.h>

uintptr_t Memory::GetModuleBase( const wchar_t* moduleName )
{
    PPEB peb = PPEB_CURRENT;
    if (!peb || !peb->Ldr)
        return 0;

    if (!moduleName || moduleName[0] == L'\0') {
        return reinterpret_cast<uintptr_t>(peb->ImageBaseAddress);
    }

    for (auto node = peb->Ldr->InLoadOrderModuleList.Flink;
        node != &peb->Ldr->InLoadOrderModuleList;
        node = node->Flink)
    {
        auto entry = CONTAINING_RECORD( node, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks );
        if (entry->BaseDllName.Buffer &&
            _wcsicmp( entry->BaseDllName.Buffer, moduleName ) == 0) {
            return reinterpret_cast<uintptr_t>(entry->DllBase);
        }
    }

    return 0;
}

uint64_t Memory::ReturnScan( uint64_t pModuleBaseAddress, const char* sSignature, size_t nSelectResultIndex )
{
    static auto patternToByte = []( const char* pattern ) {
        auto bytes = std::vector<int>{};
        const auto start = const_cast<char*>(pattern);
        const auto end = const_cast<char*>(pattern) + strlen( pattern );

        for (auto current = start; current < end; ++current) {
            if (*current == '?') {
                ++current;
                if (*current == '?')
                    ++current;
                bytes.push_back( -1 );
            }
            else
                bytes.push_back( strtoul( reinterpret_cast<const char*>( current ), &current, 16 ) );
        }
        return bytes;
        };

    if (!pModuleBaseAddress || pModuleBaseAddress == static_cast<uint64_t>(-1))
        return 0;

    const auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pModuleBaseAddress);
    if (!dosHeader || dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        return 0;

    const auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(pModuleBaseAddress) + dosHeader->e_lfanew);
    if (!ntHeaders || ntHeaders->Signature != IMAGE_NT_SIGNATURE)
        return 0;

    if (!sSignature || strlen( sSignature ) == 0)
        return 0;

    const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    auto patternBytes = patternToByte( sSignature );
    const auto size = patternBytes.size( );
    if (size == 0 || size > sizeOfImage)
        return 0;

    int anchorIndex = -1;
    for (int i = static_cast<int>(size) - 1; i >= 0; --i) {
        if (patternBytes[i] != -1) {
            anchorIndex = i;
            break;
        }
    }

    if (anchorIndex == -1) {
        if (nSelectResultIndex == 0)
            return pModuleBaseAddress;
        uint64_t possible = (sizeOfImage >= size) ? (sizeOfImage - size + 1) : 0;
        if (nSelectResultIndex < possible)
            return pModuleBaseAddress + nSelectResultIndex;
        return 0;
    }

    const uint8_t anchorByte = static_cast<uint8_t>( patternBytes[anchorIndex] );
    const auto data = patternBytes.data( );

    MEMORY_BASIC_INFORMATION mbi;
    size_t nFoundResults = 0;
    uint8_t* currentAddr = reinterpret_cast<uint8_t*>( pModuleBaseAddress );
    uint8_t* endAddr = currentAddr + sizeOfImage;

    while (currentAddr < endAddr) {
        if (currentAddr < reinterpret_cast<uint8_t*>( pModuleBaseAddress ) || currentAddr >= endAddr) {
            break;
        }

        memset( &mbi, 0, sizeof( mbi ) );
        if (!VirtualQueryEx( GetCurrentProcess( ), currentAddr, &mbi, sizeof( mbi ) )) {
            break;
        }

        if (!mbi.BaseAddress || mbi.RegionSize == 0) {
            currentAddr += 0x1000;
            continue;
        }

        if (reinterpret_cast<uintptr_t>(mbi.BaseAddress) > static_cast<uintptr_t>(-1) - mbi.RegionSize) {
            break;
        }

        bool readable =
            (mbi.Protect & PAGE_READONLY) ||
            (mbi.Protect & PAGE_READWRITE) ||
            (mbi.Protect & PAGE_WRITECOPY) ||
            (mbi.Protect & PAGE_EXECUTE_READ) ||
            (mbi.Protect & PAGE_EXECUTE_READWRITE);

        if (mbi.Protect != PAGE_NOACCESS && !(mbi.Protect & PAGE_GUARD) && readable && mbi.RegionSize >= size) {
            uint8_t* regionBase = reinterpret_cast<uint8_t*>(mbi.BaseAddress);
            size_t regionSize = mbi.RegionSize;

            if (regionSize >= size) {
                uint8_t* searchStart = regionBase + anchorIndex;
                uint8_t* searchEndExclusive = regionBase + regionSize - (size - 1) + anchorIndex;

                if (searchStart < regionBase)
                    searchStart = regionBase + anchorIndex;

                if (searchStart < regionBase || searchStart >= (regionBase + regionSize)) {
                    // nothing to search
                }
                else {
                    size_t firstSearchLen = (searchEndExclusive > searchStart) ? static_cast<size_t>( searchEndExclusive - searchStart ) : 0;
                    uint8_t* foundPtr = nullptr;
                    if (firstSearchLen)
                        foundPtr = reinterpret_cast<uint8_t*>( memchr( searchStart, anchorByte, firstSearchLen ) );

                    while (foundPtr) {
                        uint8_t* candidate = foundPtr - anchorIndex;

                        if (candidate < regionBase) {
                            foundPtr = reinterpret_cast<uint8_t*>( memchr( foundPtr + 1, anchorByte, static_cast<size_t>( searchEndExclusive - (foundPtr + 1) ) ) );
                            continue;
                        }

                        if (candidate + size > regionBase + regionSize)
                            break;

                        bool matched = true;
                        for (size_t j = 0; j < size; ++j) {
                            int pb = data[j];
                            if (pb != -1 && candidate[j] != static_cast<uint8_t>( pb )) {
                                matched = false;
                                break;
                            }
                        }

                        if (matched) {
                            if (nFoundResults == nSelectResultIndex) {
                                return reinterpret_cast<uint64_t>( candidate );
                            }
                            ++nFoundResults;
                        }

                        uint8_t* nextFrom = foundPtr + 1;
                        if (nextFrom >= (regionBase + regionSize))
                            break;

                        size_t remain = 0;
                        if (searchEndExclusive > nextFrom)
                            remain = static_cast<size_t>(searchEndExclusive - nextFrom);

                        if (remain == 0)
                            break;

                        foundPtr = reinterpret_cast<uint8_t*>(memchr( nextFrom, anchorByte, remain ));
                    }
                }
            }
        }

        uint8_t* nextAddr = reinterpret_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
        if (nextAddr <= currentAddr) {
            // Prevent infinite loop if RegionSize is 0
            currentAddr += 0x1000;
        }
        else {
            currentAddr = nextAddr;
        }
    }

    return 0;
}

uint64_t Memory::ResolveRelative( uint64_t Address, int InstructionLength ) {
    DWORD Offset = *(DWORD*)(Address + (InstructionLength - 4));
    return Address + InstructionLength + Offset;
}

std::wstring Memory::charToWchar( const char* str ) {
    if (!str)
        return L"";

    int len = MultiByteToWideChar( CP_UTF8, 0, str, -1, NULL, 0 );
    if (len <= 0)
        return L"";

    std::wstring wstr( len - 1, L'\0' );
    MultiByteToWideChar( CP_UTF8, 0, str, -1, &wstr[0], len );
    return wstr;
}

uint64_t Memory::FindPattern( const char* szModuleName, const char* sSignature, int InstructionLength ) {
    uintptr_t moduleBase;

    if (!szModuleName) {
        moduleBase = GetModuleBase( nullptr );
    }
    else {
        std::wstring wModuleName = charToWchar( szModuleName );
        moduleBase = GetModuleBase( wModuleName.c_str( ) );
    }

    if (!moduleBase) {
        g_Logger->Log( Info, "Module Base: ", moduleBase ); 
        return 0;
    }

    auto ret = ReturnScan( moduleBase, sSignature );

    if (InstructionLength != 0) {
        ret = ResolveRelative( ret, InstructionLength );
    }

    return ret;
}

uintptr_t Memory::Resolve( const char* szModuleName, int64_t targetValue, BYTE compareByte ) {
    uintptr_t baseaddy;

    if (!szModuleName) {
        baseaddy = GetModuleBase( nullptr );
    }
    else {
        std::wstring wModuleName = charToWchar( szModuleName );
        baseaddy = GetModuleBase( wModuleName.c_str( ) );
    }

    if (!baseaddy)
        return 0;

    const auto dosHeader = (PIMAGE_DOS_HEADER)baseaddy;
    const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)baseaddy + dosHeader->e_lfanew);
    const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;

    int64_t currentAddress = baseaddy + 0x13000;

    while (currentAddress < baseaddy + sizeOfImage) {
        auto cp = *(int64_t*)currentAddress;
        if (cp == targetValue) {
            if (*(BYTE*)cp == compareByte)
                return currentAddress;
        }
        currentAddress++;
    }

    return NULL;
}

DWORD_PTR Memory::GetProcBase( DWORD procId ) {
    DWORD_PTR baseaddress = 0;
    HANDLE processHandle = OpenProcess( PROCESS_ALL_ACCESS, FALSE, procId );
    HMODULE* moduleArray;
    LPBYTE moduleArrayBytes;
    DWORD bytesRequired;
    if (processHandle)
    {
        if (K32EnumProcessModules( processHandle, NULL, 0, &bytesRequired ))
        {
            if (bytesRequired)
            {
                moduleArrayBytes = (LPBYTE)LocalAlloc( LPTR, bytesRequired );
                if (moduleArrayBytes)
                {
                    unsigned int moduleCount;
                    moduleCount = bytesRequired / sizeof( HMODULE );
                    moduleArray = (HMODULE*)moduleArrayBytes;
                    if (EnumProcessModules( processHandle, moduleArray, bytesRequired, &bytesRequired ))
                    {
                        baseaddress = (DWORD_PTR)moduleArray[0];
                    }
                    LocalFree( moduleArrayBytes );
                }
            }
        }
        CloseHandle( processHandle );
    }
    return baseaddress;
}