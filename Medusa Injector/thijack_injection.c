#include "injection.h"

/*Galère car je pensais pas possible la manipulation d'un thread distant à l'aide de la structure CONTEXT en x64 (rip et non eip).
Grand merci au forum www.unknowncheats.me pour ça*/

HANDLE hProcess = 0;

//Ce shellcode n'a pas été construit par moi ! 

unsigned char x64_shellcode[] = {
	0x48, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 
	0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 
	0x48, 0x83, 0xEC, 0x40, 
	0x48, 0x83, 0xE4, 0xF7, 
	0xFF, 0xD0, 
	0x48, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 
	0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 
	0x48, 0x31, 0xd2, 
	0xFF, 0xD0 
};

int PrepareShellcode(DWORD64 dllPathAddr, DWORD64 cAddr) {
	HMODULE hmodules[1024];
	DWORD needed;
	EnumProcessModules(hProcess, hmodules, sizeof(hmodules), &needed);
	wchar_t module_name[MAX_PATH];

	for (int i = 0; i < needed; i++) {
		if (GetModuleBaseNameW(hProcess, hmodules[i], module_name, MAX_PATH) != 0) {
			if (lstrcmpiW(module_name, L"kernel32.dll") == 0) {
				/*Recherche de kernel32.dll pour calculer les adresses des fonctions LoadLibraryW et RtlRestoreContext 
				en soustrayant leur adresse à celle du module ciblé (kernel32) dans l'espace mémoire du pid cible.*/
				DWORD64 loadlibraryaddress = (DWORD64)LoadLibraryW - (DWORD64)GetModuleHandleW(L"kernel32.dll") + (DWORD64)hmodules[i];
				DWORD64 rtlrestorecontext = (DWORD64)RtlRestoreContext - (DWORD64)GetModuleHandleW(L"kernel32.dll") + (DWORD64)hmodules[i];
				memcpy(&x64_shellcode[2], &dllPathAddr, sizeof(DWORD64));
				memcpy(&x64_shellcode[12], &loadlibraryaddress, sizeof(DWORD64));
				memcpy(&x64_shellcode[32], &cAddr, sizeof(DWORD64));
				memcpy(&x64_shellcode[42], &rtlrestorecontext, sizeof(DWORD64));
				return 1;
			}
		}
		else {
			break;
		}
	}

	return 0;
}

BOOL LoadShellcode(/*PVOID Loadermemory, */wchar_t* pdll, DWORD pID) {

	CONTEXT cThread; 
	HANDLE hHijackThread; 
	int dll_path_size = wcslen(pdll) + 1;
	if (dll_path_size == 0) { LogError(L" - (HIJACK) Failed to get the dll size", L"", GetLastError()); }
	cThread.ContextFlags = CONTEXT_FULL;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

	if (hProcess != INVALID_HANDLE_VALUE) {

		DWORD64 allocDLLPath = VirtualAllocEx(hProcess, 0, dll_path_size * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (allocDLLPath != 0) {
		    DWORD64 allocDLLContext = VirtualAllocEx(hProcess, 0, sizeof(CONTEXT), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			
		if (allocDLLContext != 0) {
		    DWORD64 allocDLLShellcode = VirtualAllocEx(hProcess, 0, sizeof(x64_shellcode), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			
		if (allocDLLShellcode != 0) {

			if (PrepareShellcode(allocDLLPath, allocDLLContext)) {
				hHijackThread = GetHijackThread(pID); //GetHijackThread();

				if (hHijackThread == INVALID_HANDLE_VALUE) { return FALSE; }

				if (hHijackThread != NULL && hHijackThread != INVALID_HANDLE_VALUE) {
					
				if (SuspendThread(hHijackThread) == (DWORD)-1) {

					LogError(L" - (HIJACK) Failed to suspend the thread", L"", GetLastError());
					return FALSE;
					}

				if (!GetThreadContext(hHijackThread, &cThread)) {

					LogError(L" - (HIJACK) Failed to get thread context", L"", GetLastError());
					ResumeThread(hHijackThread); // relance qd mm pour éviter crashs
					return FALSE;
					}
					
				BOOL writePathDAddress = WriteProcessMemory(hProcess, allocDLLPath, pdll, dll_path_size * sizeof(wchar_t), 0);
				if (writePathDAddress == NULL) {
					LogError(L" - (HIJACK) Failed to write memory in process (dll)", L"", GetLastError());
					return FALSE;
					}

				BOOL writeContextAddress = WriteProcessMemory(hProcess, allocDLLContext, &cThread, sizeof(CONTEXT), 0);
				if (writeContextAddress == NULL) {
					LogError(L" - (HIJACK) Failed to write memory in process (Thread CONTEXT)", L"", GetLastError());
					return FALSE;
					}

				BOOL writeShellcodeAddress = WriteProcessMemory(hProcess, allocDLLShellcode, x64_shellcode, sizeof(x64_shellcode), 0);
				if (writeShellcodeAddress == NULL) {

				LogError(L" - (HIJACK) Failed to write memory in process (Shellcode)", L"", GetLastError());
				return FALSE;
					}
					
				cThread.Rip = allocDLLShellcode;

				if (!SetThreadContext(hHijackThread, &cThread)) {
					LogError(L" - (HIJACK) Failed to set thread context", L"", GetLastError());
					return FALSE;
					}

				if (ResumeThread(hHijackThread) == (DWORD)-1) {
					LogError(L" - (HIJACK) Failed to resume thread", L"", GetLastError());
					return FALSE;
					}
					
				if (hHijackThread == NULL) {
					return FALSE;
					}
							//printf("0x%p", hHijackThread); 
							
				WaitForSingleObject(hHijackThread, 5000);
				if ((GetProcessModule(GetModuleFromPath(pdll, bDLL), pID, 0) == FALSE)) {
					// if(hHihackThread) peut être non null et échouer
					return FALSE;
					}
				PostThreadMessage(hHijackThread, WM_NULL, 0, 0);
				return TRUE;
						}
					}
				}
				else {
				LogError(L" - (HIJACK) Can't allocate memory for shellcode", L"", GetLastError());
				return FALSE;
					}
				}
				else {
					LogError(L" - (HIJACK) Can't allocate memory for Thread CONTEXT", L"", GetLastError());
					return FALSE;
					}
				}
				else {
					LogError(L" - (HIJACK) Can't allocate memory for dll", L"", GetLastError());
					return FALSE;
				}

			} else {
				LogError(L" - (HIJACK) Failed to open targeted process", L"", GetLastError());
				return FALSE;
			}
		}
