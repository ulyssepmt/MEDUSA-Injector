#include "injection.h"
#include "features.h"
#include "utils.h"

BOOL LoadDLL(wchar_t* dll, DWORD pID) {

	if ((GetProcessModule(GetModuleFromPath(dll, bDLL), pID, 0) == TRUE)) {
		if (UnloadDLL(dll, pID) == FALSE) {
			LogError(L"Can't unload DLL", NULL, NULL); 
		}
	   
	}
	HANDLE hThread = NULL; 
	HMODULE hNtdll = GetModuleHandleW(L"ntdll"); 
	RtlCreateUserThreadDef RtlCreateUserThread = (RtlCreateUserThreadDef)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlCreateUserThread");

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE,  pID); // droits requis pour la création de threads 
	
	if (hProcess == NULL) {
		LogError(L"Failed to open targeted process", L"", GetLastError()); 
		return FALSE; 
	}
	DWORD dwSize = (lstrlenW(dll) + 1) * sizeof(wchar_t); /*if (dwSize == 0) printf("error size\n");*/ 

	LPVOID pLibFileRemote = (PWSTR)VirtualAllocEx(hProcess, NULL, dwSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (pLibFileRemote != NULL) {

		PTHREAD_START_ROUTINE LoadLib = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32"), "LoadLibraryW");
		//PTHREAD_START_ROUTINE LoadLib = NULL; 
		if (LoadLib != NULL) {

			pfnZwCreateThreadEx ZwCreateThreadEx = (pfnZwCreateThreadEx)GetProcAddress(hNtdll, "ZwCreateThreadEx");
			if (!ZwCreateThreadEx) {
				LogError(L"Error loading ZwCreateThreadEx function", L"", GetLastError()); 
				VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
				CloseHandle(hProcess);
				return FALSE;
			}
			pfnZwClose ZwClose = (pfnZwClose)GetProcAddress(hNtdll, "ZwClose");
			if (!ZwClose) {
				LogError(L"Error loading ZwClose function", L"", GetLastError()); 
				VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
				CloseHandle(hProcess);
				return FALSE;
			}
			int WriteMem = WriteProcessMemory(hProcess, pLibFileRemote, dll, dwSize, NULL);
			if (WriteMem == 0) {
				LogError(L"Error writing to remote process memory", L"", GetLastError()); 
				VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
				return FALSE;
			}
	
		HRESULT ntStat = ZwCreateThreadEx(&hThread, THREAD_ALL_ACCESS, 0, hProcess, (LPTHREAD_START_ROUTINE)LoadLib, pLibFileRemote, THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER, 0, 0, 0, 0);
			WaitForSingleObject(hThread, INFINITE); 
			if ((GetProcessModule(GetModuleFromPath(dll, bDLL), pID, 0) == FALSE) || hThread == NULL) {
			     wprintf(L"[ERROR %lu] Failed with NtCreateThreadEx %p!\n", GetLastError(), hThread);
				 //Qqs cas ou la dll peut ne pas être injectée avec un handle hThread diff de 0

				HRESULT RTLThread = RtlCreateUserThread(hProcess, NULL, FALSE, 0, NULL, NULL, (PVOID)GetProcAddress(GetModuleHandleW(L"Kernel32"), "LoadLibraryW"), pLibFileRemote, &hThread, NULL);
				WaitForSingleObject(hThread, INFINITE);

					if ((GetProcessModule(GetModuleFromPath(dll, bDLL), pID, 0) == FALSE) || hThread <= 0) {
						wprintf(L"[ERROR %lu] Failed with RtlCreateUserThread | hThread = %p!\n", GetLastError(), hThread);

						HANDLE RemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLib, pLibFileRemote, 0, NULL);
							WaitForSingleObject(RemoteThread, INFINITE);

							if ((GetProcessModule(GetModuleFromPath(dll, bDLL), pID, 0) == FALSE) || RemoteThread <= 0) {
								wprintf(L"[ERROR %lu] Failed with CreateRemoteThread | hThread = %p!\n", GetLastError(), hThread);
									return FALSE;
								}
								
								else {
									LogSuccess(L" - (OK) Injected using CreateRemoteThread() function\n", L"");
									WaitForSingleObject(RemoteThread, INFINITE);
									CloseHandle(RemoteThread);
									VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
									CloseHandle(hProcess);
								}
							}
						
						else {
							LogSuccess(L" - (OK) Injected using RtlCreateUserThread() function", L"");
							WaitForSingleObject(hThread, INFINITE);
							CloseHandle(hThread);
							VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
							CloseHandle(hProcess);
						}

					}

				else {
					LogSuccess(L" - (OK) Injected using ZwCreateThread function()", L"");
					WaitForSingleObject(hThread, INFINITE);
					ZwClose(hThread);
					VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
					CloseHandle(hProcess);
				}
			}
		else {
			LogError(L"Failed to load library (LoadLibraryW)", L"", GetLastError()); 
			VirtualFreeEx(hProcess, pLibFileRemote, 0, MEM_RELEASE);
			CloseHandle(hProcess);
			return FALSE;
		}
	}
	else {
		LogError(L"Allocating memory in the remote process", L"", GetLastError());
		return FALSE;
	}

	return TRUE; 
}

BOOL UnloadDLL(wchar_t* dll, DWORD pID) {
		HMODULE hMod = NULL;
		HMODULE hMods[0x400];
		DWORD dwNeeded;

		if (!pID) {
			return FALSE;
		}

		HANDLE Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
		if (!Proc) {
			return FALSE;
		}

		EnumProcessModules(Proc, hMods, sizeof(hMods), &dwNeeded);
		for (unsigned int i = 0; i < (dwNeeded / sizeof(HMODULE)); i++) {

			wchar_t szPath[MAX_PATH] = L"";

			GetModuleFileNameExW(Proc, hMods[i], szPath, MAX_PATH);
			//PathStripPathW(szPath);

			if (_wcsicmp(szPath, dll) == 0) {

				hMod = hMods[i];

			}
		}

		LPVOID callFreeLibrary = (LPVOID)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "FreeLibrary");

		if (callFreeLibrary != NULL) {
			HANDLE hThread = CreateRemoteThread(Proc, NULL, 0, (LPTHREAD_START_ROUTINE)callFreeLibrary, (LPVOID)hMod, 0, NULL);
			if (hThread != NULL) {

				WaitForSingleObjectEx(hThread, INFINITE, FALSE);
				return TRUE;
				//FreeLibraryAndExitThread(hMod, 0);
			}
		}
		CloseHandle(Proc);
		return FALSE; 
		
	}
