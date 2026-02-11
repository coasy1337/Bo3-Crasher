#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string.h>  
#include <stdio.h>  
#include <locale.h>  
#include <tchar.h>
#include <vector>
#include <dwmapi.h>
#include <filesystem>
#include <Windows.h>
#include <TlHelp32.h>
#include <DirectXMath.h>
#include <minwindef.h>
#include <thread>
#include <sstream>
#include <Shlwapi.h>
#include <Psapi.h>
#include <fstream>
#include <iomanip>
#include <codecvt>
#include <optional>
#include <memory>
#include <array>
#include <mutex>
#include <unordered_map>

//DX11
#include <d3d11.h>
#include <D3DX11tex.h>
#pragma comment(lib, "D3DX11.lib")
#pragma comment(lib, "dxgi.lib")

//Logger
#include <Logger.h>

//ImGui
#include <imgui.h>
#include <imgui_internal.h>
#include <backend/imgui_impl_win32.h>
#include <backend/imgui_impl_dx11.h>

//Variables
#include <Variables.h>

//Memory
#include <Memory.h>

//Pointers
#include <Pointers.h>

//UserInterface
#include <UserInterface.h>

//Hooking
#include <Hooking.h>

//Setup
#include <Setup.h>