#pragma once
#include <Windows.h>
#include <TlHelp32.h>                                    
#include <stdbool.h>
#include <winternl.h>
#include <psapi.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
DWORD GetpIDFrompName(LPCTSTR pName); 
wchar_t* GetModuleFromPath(const wchar_t* path, wchar_t* filename);
DWORD GetModuleBaseAddress(DWORD ProcID, wchar_t* pMod); 
RECT rect_console; 
BOOL EnablePrivilege();
HANDLE GetHijackThread(DWORD pID); 


extern const int UICOLOR;
extern const int DEFAULTUICOLOR; 
extern const int UIFONTCOLOR; 

void LogError(wchar_t* err, DWORD lastError); 
extern void LogInfo(wchar_t* info, DWORD rpID); 
extern void LogSuccess(wchar_t* success); 