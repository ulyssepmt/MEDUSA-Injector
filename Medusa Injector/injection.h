#pragma once
#define MY_MAX_PATH 1000
#include <Windows.h>
#include <TlHelp32.h> // on va en avoir besoin pour utiliser la fonction CreateToolhelp32Snapshot                                         
#include <stdbool.h>
#include <winternl.h>
#include <psapi.h>
#include "utils.h"
#include "features.h"

#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER 0x00000004
#define ZwCurrentProcess() ((HANDLE)(LONG_PTR)-1)

#define PAGE_SIZE 0x1000

wchar_t bDLL[256];

typedef CLIENT_ID* PCLIENT_ID;
typedef NTSTATUS(NTAPI* RtlAdjustPrivilegeDef) (IN ULONG Privilege, IN BOOLEAN Enable, IN BOOLEAN CurrentThread, OUT PBOOLEAN Enabled);
typedef NTSTATUS(NTAPI* RtlCreateUserThreadDef) (
	HANDLE,
	PSECURITY_DESCRIPTOR,
	BOOLEAN, ULONG,
	PULONG, PULONG,
	PVOID, PVOID,
	PHANDLE, PCLIENT_ID
	);

typedef NTSTATUS(NTAPI* pfnZwCreateThreadEx)(
	_Out_ PHANDLE ThreadHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_ HANDLE ProcessHandle,
	_In_ PVOID StartRoutine,
	_In_opt_ PVOID Argument,
	_In_ ULONG CreateFlags,
	_In_opt_ ULONG_PTR ZeroBits,
	_In_opt_ SIZE_T StackSize,
	_In_opt_ SIZE_T MaximumStackSize,
	_In_opt_ PVOID AttributeList
	);

typedef NTSTATUS(NTAPI* pfnZwClose)(IN HANDLE ObjectHandle);


// Déclarations pr injection.c
BOOL LoadDLL(wchar_t* dll, DWORD pID); 
BOOL UnloadDLL(wchar_t* dll, DWORD pID); 
BOOL LoadShellcode(/*PVOID Loadermemory, */wchar_t* pdll, DWORD pID); 










