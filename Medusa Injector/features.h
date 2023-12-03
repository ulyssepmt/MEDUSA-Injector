#pragma once

#include <Windows.h>
#include <TlHelp32.h>                                         
#include <stdbool.h>
#include <tchar.h>
#include <Psapi.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <Shlwapi.h> 
#pragma comment(lib, "Shlwapi.lib")

void CenterText(const char* text);
void SaveConfig(const wchar_t* chemin_dll, const wchar_t* processus);
BOOL LoadConfig(); 

BOOL CheckPathExistence(const wchar_t* chemin_dll);
BOOL CheckDLLArch(const wchar_t* chemin_dll);
BOOL CheckpIDArch(DWORD processId); 
BOOL GetProcessModule(wchar_t* ModuleName, DWORD pid, int display); 
void ShowProcs();

wchar_t procTXTConfig[MAX_PATH];
wchar_t dllTXTConfig[MAX_PATH];

wchar_t configTXTPath[FILENAME_MAX];


RECT r;
int getConsoleWidth(); 