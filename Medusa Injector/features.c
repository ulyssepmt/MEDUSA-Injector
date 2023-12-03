#include "features.h"
#include "injection.h"

void ShowProcs() {
    DWORD processes[1024];
    DWORD numProcesses;
    if (!EnumProcesses(processes, sizeof(processes), &numProcesses)) {
       // fprintf(stderr, "Erreur lors de l'appel à EnumProcesses\n");
        LogError(L"Failed to call EnumProcesses()", L"", 0);
        return 1;
    }


    // Calcule le nombre de processus retournés par EnumProcesses
    DWORD numEntries = numProcesses / sizeof(DWORD);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DEFAULTUICOLOR);
    printf("[PID]->\tProcess\n------------------------------------\n");

    for (DWORD i = 0; i < numEntries; i++) {
        DWORD processID = processes[i];
        if (processID != 0) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
            if (hProcess != NULL) {
                BOOL isWow64;
                if (IsWow64Process(hProcess, &isWow64) && !isWow64) {
    
                    char processName[MAX_PATH];
                    if (GetModuleBaseNameA(hProcess, NULL, processName, sizeof(processName))) {
                        printf("[%lu]\t-> %s\n", processID, processName);

                       
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);
}

BOOL GetProcessModule(wchar_t* ModuleName, DWORD pid, int display)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    MODULEENTRY32 module32;
    module32.dwSize = sizeof(MODULEENTRY32);

    if (!Module32First(hSnap, &module32)) {
        CloseHandle(hSnap);
        LogError(L"Module32First failed", L"", GetLastError());
        return FALSE;
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
    do {
        if (display == 1) {
            if ((wcscmp(module32.szModule, GetModuleFromPath(ModuleName, bDLL)) == 0)) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
            }
            wprintf(L"Module : %s\n", module32.szModule);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
        }

        else if (wcscmp(module32.szModule, ModuleName) == 0 && display == 0) {

            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);
            return TRUE;
        }

    } while (Module32Next(hSnap, &module32));

    CloseHandle(hSnap);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);
    return FALSE;

}

void SaveConfig(wchar_t* pDll, wchar_t* proc) {
    FILE* fptr;
    char filename[] = "config.txt"; 
    wchar_t wfilename[MAX_PATH];
    if (fopen_s(&fptr, filename, "w") != 0) {
        
        return;
    }


    int ecritureDLL = fwprintf(fptr, L"%ws;", pDll);
    int ecritureProc = fprintf(fptr, "%ws;", proc);


    if (ecritureDLL <= 0 || ecritureProc <= 0) {

       // printf("[ERROR] Failed to save DLL path or process.\n");
        LogError(L"Failed to save DLL path or process", L"", 0); 
        fclose(fptr);
        return;
    }
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename, MAX_PATH);
    DWORD result = GetFullPathNameW(wfilename, FILENAME_MAX, configTXTPath, NULL);
    if (result == 0) {

       // printf("[ERROR] Failed to get config file path."); 
        LogError(L"Failed to get config file path", L"", 0); 
        return;
    }
    else {
        LogInfo(L"Config file saved at", configTXTPath, NULL); 
        //printf("[INFO] Config file saved at [%ws]\n", configTXTPath);

    }

    fclose(fptr);
}



BOOL LoadConfig() {
    FILE* fptr;
    wchar_t filename[] = L"config.txt";
    wchar_t line[256]; 
    if (_wfopen_s(&fptr, filename, L"r, ccs=UTF-8") != 0) {
     //   wprintf(L"Error opening the file.\n");
    }

    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    int isEmpty = (size == 0);

    if (isEmpty) {
        fclose(fptr); 
        return FALSE; // retourne une erreur si ya rien dans le fichier (=première utilisation de medusa ? | Injection echouée ? )
    }
    else {
        fseek(fptr, 0, SEEK_SET); //Reviens au début

        while (fgetws(line, sizeof(line) / sizeof(line[0]), fptr)) {
            wchar_t* context = L"";
            wchar_t* token = wcstok_s(line, L";", &context);
            if (token != NULL) {
                wcsncpy_s(dllTXTConfig, MAX_PATH, token, _TRUNCATE);

                token = wcstok_s(NULL, L";", &context);
                if (token != NULL) {
                    wcsncpy_s(procTXTConfig, MAX_PATH, token, _TRUNCATE);
                    fclose(fptr);
                    return TRUE; 
                }
            }

        }
            

        fclose(fptr);
        return FALSE;
    }
}

BOOL CheckPathExistence(const wchar_t* chemin_dll) {
    DWORD fAttr = GetFileAttributesW(chemin_dll);

    if (fAttr == INVALID_FILE_ATTRIBUTES) {
        return FALSE; 
    }

    if (fAttr & FILE_ATTRIBUTE_DIRECTORY) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


BOOL CheckDLLArch(const wchar_t* dllPath) { // 64 ou non
    HANDLE hFile = CreateFile(dllPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {

        return FALSE;
    }

    DWORD bytesRead = 0;
    IMAGE_DOS_HEADER dosHeader;
    IMAGE_NT_HEADERS ntHeader;


    if (!ReadFile(hFile, &dosHeader, sizeof(IMAGE_DOS_HEADER), &bytesRead, NULL)) {
        CloseHandle(hFile);
        return FALSE;
    }


    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        CloseHandle(hFile);
        return FALSE;
    }

    SetFilePointer(hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN);

    if (!ReadFile(hFile, &ntHeader, sizeof(IMAGE_NT_HEADERS), &bytesRead, NULL)) {
        CloseHandle(hFile);
        return FALSE;
    }

    if (ntHeader.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) {
        CloseHandle(hFile);
        return TRUE; // 64-bit (x64) 
    }
    else {
        CloseHandle(hFile);
        return FALSE; // 32-bit (x86) 
    }
}

BOOL CheckpIDArch(DWORD processId) {

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) {
        return FALSE;
    }

    BOOL isWow64 = FALSE;
    IsWow64Process(hProcess, &isWow64);

    CloseHandle(hProcess);

    return isWow64 ? FALSE : TRUE;
    
}


/*
void _64processes(DWORD processID) {
    wchar_t szProcessName[MAX_PATH];
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    int width = getConsoleWidth();
    if (NULL != hProcess) {
        HMODULE hMod;

        DWORD cbNeeded;
        if (EnumProcessModulesEx(hProcess, &hMod, sizeof(hMod), &cbNeeded, LIST_MODULES_64BIT)) {
            if (GetModuleBaseNameW(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(wchar_t)) == 0) {
                printf("%lu", GetLastError());
            }
        }

        CloseHandle(hProcess);
    }

    wchar_t concatenated[MAX_PATH + 20]; // The size 20 is sufficient to store a processID in text

    sprintf_s(concatenated, sizeof(concatenated), "%ws -> [%d]", szProcessName, processID);
    printf("%ws -> %d\n", szProcessName, processID);


    RECT r;
    GetWindowRect(GetConsoleWindow(), &r);
    SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left), 0, 700, 1080, SWP_SHOWWINDOW);
}

void Show64List() {
    DWORD aProcesses[1024], cbNeeded, cProcesses;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        // return 1;
    }

    cProcesses = cbNeeded / sizeof(DWORD);
    wprintf(L"%lu", cProcesses);
    for (unsigned int i = 0; i < cProcesses; i++) {
        if (aProcesses[i] != 0) {
            _64processes(aProcesses[i]);
        }
    }

    wprintf(L"\033[38;5;208m\n\n");
    // system("pause");
}*/




