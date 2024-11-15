#include "utils.h"

const int colorError = 12;
const int colorInfo = /*9*/BACKGROUND_BLUE | BACKGROUND_INTENSITY;
const int colorSuccess = 15;

const int DEFAULTUICOLOR = 11;
const int UICOLOR = 10;
const int UIFONTCOLOR = BACKGROUND_GREEN /*| FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE*/;

void LogError(wchar_t* err, DWORD lastError) {

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorError);
	wprintf(L"[ERROR % lu] %ws.\n\n", lastError, err);
	
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);

}

void LogInfo(wchar_t* info, DWORD rpID) {

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorInfo);
	wprintf(L"[INFO] %ws %lu.", info, rpID);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);
	printf("\n\n"); 

}

void LogSuccess(wchar_t* success) {

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorSuccess);
	wprintf(L"[SUCCESS] %ws.\n\n", success);
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
		return NULL; 

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
BOOL EnablePrivilege() { 
	BOOLEAN bEnabled;
	HMODULE hModuleNtDll = GetModuleHandleW(L"ntdll.dll");
	RtlAdjustPrivilegeDef RtlAdjustPrivilege = (RtlAdjustPrivilegeDef)GetProcAddress(hModuleNtDll, "RtlAdjustPrivilege");

	NTSTATUS status = RtlAdjustPrivilege(0x00000014, TRUE, FALSE, &bEnabled);

	if (status) {
		return TRUE;
	}
	else if (!bEnabled) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}


