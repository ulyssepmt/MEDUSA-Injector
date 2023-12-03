#include <stdio.h>
#include <windows.h>
#include "injection.h"
#include "features.h"
#include "utils.h"

//const int menu = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
//const int UIFONTCOLOR = BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

void MedusaMenu(HWND console, HANDLE hConsole) {

            SetConsoleTitleA("MEDUSA Injector v1.0 by sk0za");
            GetWindowRect(console, &r); 
            SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left), 0, 600, 600, SWP_SHOWWINDOW);

            SetConsoleTextAttribute(hConsole, UICOLOR);
            char injectorName[][23] = { "MEDUSA Injector\n\n" };

            for (int i = 0; i < sizeof(injectorName) / sizeof(injectorName[0]); i++) {
                size_t sentenceLength = strlen(injectorName[i]);
                size_t spaces = (getConsoleWidth() - sentenceLength) / 2;

                for (int j = 0; j < spaces; j++) {
                    putchar(' ');
                }

                for (int j = 0; j < sentenceLength; j++) {
                    putchar(injectorName[i][j]); // un par un 
                    Sleep(40); 
                }
              //  CenterText("**********************************************************\n");
                CenterText("         ( (`  ,--.    ");
                CenterText("   ,--.  ) )\\  `) ).--,  ");
                CenterText("   `) )/ /) ) ( (( (`    ");
                CenterText("   / /( (.  \"-.)) )) )   ");
                CenterText(" ,-.( ( /        / /,--,  ");
                CenterText(" `) \\ ` ==.  .==  ( (`   ");
                CenterText(" ( (_) ~9~      ~9~ (_) ) ");
                CenterText("  \\_ (     )(      )_/  ");
                CenterText(",-,\\ \     ^^     /\\ ,-. ");
                CenterText("( (_/ /\\    __,   /\\ \\_) )");
                CenterText(" '._.' _\\  /__/  /_ '._.' ");
                CenterText("      .-\\ `    /  `--.  ");
                CenterText("   '----'   ");
                CenterText("\n\n");
                CenterText("*********************************************************\n");

            }
            SetConsoleTextAttribute(hConsole, UIFONTCOLOR);
            printf("Options: \n\n");
            SetConsoleTextAttribute(hConsole, UICOLOR);
            printf("[0] About\n");
            printf("[1] Load a config in a running process.\n");
            printf("[2] Show x64 processes running.\n");
            printf("[3] Show modules in a specific process.\n\n");
            SetConsoleTextAttribute(hConsole, UIFONTCOLOR);
            printf("Injection: \n\n");
            SetConsoleTextAttribute(hConsole, UICOLOR);
            printf("[4] DLL injection.\n\n");
            SetConsoleTextAttribute(hConsole, UICOLOR);
         

}

void ClearInput(int input) {
    while (((input = getchar()) != '\n') && (input != EOF));
}

void main() {
    HWND hw_console = GetConsoleWindow();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    LPCTSTR Process = NULL;

    MedusaMenu(hw_console, hConsole);

    wchar_t proc[MAX_PATH];
    wchar_t* szDllPath = (wchar_t*)malloc(MAX_PATH);

    char askHijack = ' ';

    int input = 0;
    int validDLLInput = 0;

    while (TRUE) {
        input = 0; 
        validDLLInput = 0;
        printf("Option > ");
        scanf_s("%d", &input);

        if (input == 0) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            printf("Medusa injector v1.0, coded in C by sk0za. For more informations about usages and injection process, see: https://github.com/ulyssepmt");  
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);
            printf("\n\n");             //déborde sur l'autre ligne sinon 
        }
        if (input == 1) {

            if ((LoadConfig() == FALSE)) {

                LogError(L"No config saved in [config.txt]", L"", 0);
            }
            else {
                DWORD oldpID = GetpIDFrompName(procTXTConfig); // Ancien PID et non dans le fichier config.txt
                if (oldpID == 0 || CheckPathExistence == FALSE) {

                    LogError(L"Have you changed your dll path ? Is the target process open ? Check that", L"", 0);

                }
                else if (LoadDLL(dllTXTConfig, oldpID) == FALSE) {
                    LogError(L"Dll injection failed ! Trying to hijack a thread in", procTXTConfig, GetLastError());

                    if (LoadShellcode(dllTXTConfig, oldpID) == FALSE) {
                        LogError(L" - (HIJACK) Failed to inject in", procTXTConfig, GetLastError());
                    }
                    else {
                        LogSuccess(L" - (HIJACK) Loaded and injected with success in", procTXTConfig);
                    }
                }
                else {
                    LogSuccess(L"Loaded and injected with success in", procTXTConfig);
                }
            }
            /*  else if (wcscmp(injectionType, L"ManualMap") == 0) {
                  if (ManualMap(dll_config, old_id)) {
                      printf("[SUCCESS] Loaded and mapped with success!\n");
                  }
                  else {
                      printf("[ERROR] Failed to map in %ws\n", proc_config);
                  }

              }*/
        }



        else if (input == 2) {
            SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left), 0, 600, GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
            printf("\n");
            ShowProcs();
            printf("\n");
        }


        else if (input == 3) {
            printf("\nEnter an x64 process name: \n> ");
            wscanf_s(L"%ls", proc, MAX_PATH);
            DWORD pID = GetpIDFrompName(proc);
            if (pID == 1 || pID == 0) {

                LogError(L"Process not found! Please enter a valid process name", L"", 0); // L"" pour éviter l'affichage "(null)"
                continue;
            }
            else {
                GetProcessModule(szDllPath, pID, 1);
                printf("\n");
            }
        }
        else if (input == 4) {
            //askHijack = (char*)malloc(MAX_PATH);
            if (EnablePrivilege() == TRUE) {

                LogSuccess(L"Privileges enabled", L"");
            }
            else {

                LogError(L"Can't ajust privileges. Maybe you already got it from running with right privileges", L"", GetLastError());
            }
            while (1) {
                printf("\nEnter an x64 process name: \n> ");
                wscanf_s(L"%ls", proc, MAX_PATH);

                DWORD pID = GetpIDFrompName(proc);
                if (pID == 1 || pID == 0) {

                    LogError(L"Process not found! Please enter a valid process name", L"", 0);
                    continue;
                }
                else {
                    if ((CheckpIDArch(pID) == FALSE)) {
                        LogError(L"Please enter an x64 process", L"", 0);

                        continue;
                    }
                    else {
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_BLUE | BACKGROUND_INTENSITY);
                        printf("[INFO] Found! Process ID is %lu", pID); // <3 2019
                        //LogInfo(L"Found! Process ID is", L"", pID);
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);
                        printf("\n\n");
                    }

                    do {
                        printf("DLL file : >_");
                        wscanf_s(L"%ws", szDllPath, MAX_PATH);;

                        if (!(CheckDLLArch(szDllPath))) {

                            LogError(L"Please enter a x64 DLL", L"", 0);
                        }
                        else {
                            validDLLInput = 1;
                        }

                    } while (!validDLLInput);

                    if ((LoadDLL(szDllPath, pID) == FALSE)) {
                        LogError(L"Failed to inject ! In the last case, it is possible to use a remote hijack thread, but this would lead to a crash of the target program. Would you like to try this procedure ? [y / n]",
                            L"", 0);
                        printf("[y / n] > ");

                        while (1) {

                            scanf_s(" %c", &askHijack, 1);
                            ClearInput(askHijack); 

                            switch (askHijack) {
                            case 'y':
                                if (LoadShellcode(szDllPath, pID) == FALSE) {
                                    LogError(L" - (HIJACK) Failed to inject DLL in ", proc, 0);
                                }
                                else {
                                    LogSuccess(L" - (HIJACK) Successful injection", L"");
                                    SaveConfig(szDllPath, proc);
                                }
                                break;
                            case 'n': 
                                LogError(L"Failed to inject DLL in ", proc, 0);
                                break;

                            default:
                                LogError(L"Please enter a valid option below [y / n] : ", L"", 0);
                                continue; 
                            }
                            break; 
                        }
               
                    }
                    else {
                        LogSuccess(L"Successful injection", L"");
                        SaveConfig(szDllPath, proc);
                        break;
                    }
                }
                break; 
            }
        }


        else if (input < 0 || input > 4 || (input = getchar()) != '\n') {
            ClearInput(input);
            LogError(L"Please enter a valid option ! [0-4]", NULL, NULL);
        }
    }

 }






