#include "utils.h"

const int colorError = 12;
const int colorInfo = 9;
const int colorSuccess = 15;

const int DEFAULTUICOLOR = 11;
const int UICOLOR = 10;
const int UIFONTCOLOR = BACKGROUND_GREEN /*| FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE*/;

void LogError(char* err, wchar_t* precisions, DWORD lastError) {

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorError);
	wprintf(L"[ERROR % lu] %s %ws\n\n", lastError, err, precisions);
	
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);

}

void LogInfo(char* info, wchar_t* precisions, DWORD rpID) {

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorInfo);
	wprintf(L"[INFO] %s [%ws] %lu", info, precisions, rpID);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);
	printf("\n\n"); 

}

void LogSuccess(char* success, wchar_t* precisions) {

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorSuccess);
	wprintf(L"[SUCCESS] %s %ws!\n\n", success, precisions);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);

}

void CenterText(
	char* text) {
	int width = getConsoleWidth();
	int padding = (width - strlen(text)) / 2; 
	printf("%*s%s\n", padding, "", text);
}

HANDLE GetHijackThread(DWORD pID) {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	if (Thread32First(snapshot, &te32)) {
		while (Thread32Next(snapshot, &te32)) {
			if (te32.th32OwnerProcessID == pID) {
				CloseHandle(snapshot);
				return OpenThread(THREAD_ALL_ACCESS, 0, te32.th32ThreadID);
			}
		}
	}

	return INVALID_HANDLE_VALUE;
	CloseHandle(snapshot);
}

DWORD GetpIDFrompName(LPCTSTR pName) { 
	// récupére l'ID du processus par son nom. Le but de cette partie du code est de retourner un message d'erreur si le nom du processus entré n'est pas en cours d'éxécution {

	PROCESSENTRY32 pt;
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // récupère instantanément un processus ciblé, les modules & threads qui lui sont associés
	pt.dwSize = sizeof(PROCESSENTRY32); // renvoie la quantité de mémoire occupée par pt

	if (Process32First(hsnap, &pt))
	{
		do {

			if (!lstrcmpi(pt.szExeFile, pName))
			{
				CloseHandle(hsnap);
				return pt.th32ProcessID;
			}
		} while (Process32Next(hsnap, &pt));
	}

	CloseHandle(hsnap);//Ferme 

	return 0;
}

wchar_t *GetModuleFromPath(wchar_t* path, wchar_t* filename) {
	/*const?*/ wchar_t* filenameStart = wcsrchr(path, L'\\');

	if (filenameStart != NULL) {
		filenameStart++;
		wcscpy_s(filename, 256, filenameStart);
		return filename;
	}
	else {
		wcscpy_s(filename, 256, path);
		return filename;
	}
}

DWORD GetModuleBaseAddress(DWORD ProcID, wchar_t* pMod) {
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcID);
	if (Snapshot == INVALID_HANDLE_VALUE)

		printf("Could not get module snapshot for remote process");

	MODULEENTRY32W ModEntry = { sizeof(ModEntry) };
	BOOL Found = FALSE;
	BOOL bMoreMods = Module32FirstW(Snapshot, &ModEntry);

	for (; bMoreMods; bMoreMods = Module32NextW(Snapshot, &ModEntry)) {
		Found = (wcscmp(ModEntry.szModule, pMod) == 0 || wcscmp(ModEntry.szExePath, pMod) == 0);

		if (Found)
			return (DWORD)ModEntry.modBaseAddr;
	}
	return NULL;
}


typedef NTSTATUS(NTAPI* RtlAdjustPrivilegeDef) (IN ULONG Privilege, IN BOOLEAN Enable, IN BOOLEAN CurrentThread, OUT PBOOLEAN Enabled);
	HMODULE hModuleNtDll = GetModuleHandleW(L"ntdll.dll");

	BOOLEAN bEnabled;
	RtlAdjustPrivilegeDef RtlAdjustPrivilege = (RtlAdjustPrivilegeDef)GetProcAddress(hModuleNtDll, "RtlAdjustPrivilege");
	RtlAdjustPrivilege(0x00000014, TRUE, FALSE, &bEnabled);

	if (!NT_SUCCESS(RtlAdjustPrivilege(0x00000014, TRUE, FALSE, &bEnabled))) {
		return FALSE;
	}
	else if (!bEnabled) {
		return FALSE;
	}
	else {
		return TRUE; 
	}
}

