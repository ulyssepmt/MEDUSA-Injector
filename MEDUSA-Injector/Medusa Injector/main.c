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

            SetConsoleTitleA("MEDUSA Injector v1.1 by sk0za");
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
    fflush(stdin); 
}

void main() {
    HWND hw_console = GetConsoleWindow();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    LPCTSTR Process = NULL;

    MedusaMenu(hw_console, hConsole);

    wchar_t proc[MAX_PATH];
    wchar_t* szDllPath = (wchar_t*)malloc(MAX_PATH);

    char askHijack = ' ';
    char inputStr[10]; // Tampon pour l'entrée
    int input = 0, validDLLInput = 0;
    if (EnablePrivilege() == TRUE) {
        LogSuccess(L"Privileges enabled");
    }
    else {
        LogError(L"Can't set privileges. Maybe you already have them by running with the right privileges", GetLastError());
    }
    while (TRUE) {
        input = -1; 
        validDLLInput = 0;
        printf("Option > ");

        if (scanf_s("%9s", inputStr, (unsigned)_countof(inputStr)) != 1) {
            LogError(L"Please enter a valid option! [0-4]", NULL);
            ClearInput(input); 
            continue;
        }

        
        if (sscanf_s(inputStr, "%d", &input) != 1 || input < 0 || input > 4) {
            LogError(L"Please enter a valid option! [0-4]", NULL);
            ClearInput(input);  
            continue;
        }

        
        switch (input) {
        case 0:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            printf("Medusa injector v1.1, coded in C by sk0za.\nCredits:\n- Forum www.unknowncheat.me\n- Forum http://www.rohitab.com/\n- Victeezy(icon medusa FREE LICENSE): https://www.vecteezy.com/\nAuthor: https://www.vecteezy.com/members/latiefzubet736081\n\nFor more information about usages and injection process, see: https://github.com/ulyssepmt\n");
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), UICOLOR);
            printf("\n\n");
            break;

        case 1:
            if ((LoadConfig() == FALSE)) {
                LogError(L"No configuration saved in [config.txt]", GetLastError());
            }
            else {
                DWORD oldpID = GetpIDFrompName(procTXTConfig);
                if (oldpID == 0 || CheckPathExistence == FALSE) {
                    LogError(L"Have you changed your DLL path? Is the target process open? Check this", NULL);
                }
                else if (LoadDLL(dllTXTConfig, oldpID) == FALSE) {
                    LogError(L"DLL injection failed! Trying thread hijacking..", GetLastError());
                    if (LoadShellcode(dllTXTConfig, oldpID) == FALSE) {
                        LogError(L"(HIJACK) - Failed to inject", GetLastError());
                    }
                    else {
                        LogSuccess(L"(HIJACK) - Loaded and injected with success in target process");
                    }
                }
                else {
                    LogSuccess(L"Loaded and injected successfully in target process");
                }
            }
            break;

        case 2:
            SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left), 0, 600, GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
            printf("\n");
            ShowProcs();
            printf("\n");
            break;

        case 3:
            printf("\nEnter x64 process: \n> ");
            wscanf_s(L"%ls", proc, MAX_PATH);
            DWORD pID = GetpIDFrompName(proc);
            if (pID == 1 || pID == 0) {
                LogError(L"Process not found! Please enter a valid process name", NULL);
                continue;
            }
            else {
                GetProcessModule(szDllPath, pID, 1);
                printf("\n");
            }
            break; 
            
        case 4: 
            while (1) {
                printf("\nPlease, enter a x64 process: \n> ");
                wscanf_s(L"%ls", proc, MAX_PATH);
                DWORD pID = GetpIDFrompName(proc);
                if (pID == 1 || pID == 0) {
                    LogError(L"Process not found", NULL);
                    continue;
                }
                else if ((CheckpIDArch(pID) == FALSE)) {
                    LogError(L"Please enter x64 process", L"", 0);
                    continue;
                }
                else {
                    LogInfo(L"Found target process! ID is", pID);
                    do {
                        printf("DLL file: >_ ");
                        wscanf_s(L"%ws", szDllPath, MAX_PATH);
                        if (!(CheckDLLArch(szDllPath))) {
                            LogError(L"Please enter a x64 DLL", NULL);
                        }
                        else {
                            validDLLInput = 1;
                        }
                    } while (!validDLLInput);

                    if ((LoadDLL(szDllPath, pID) == FALSE)) {
                        LogError(L"DLL injection failed ! Wanna try thread hijacking ? This technique can cause the target program to crash [y/n]", NULL);
                        printf("[y/n] > ");
                        while (1) {
                            scanf_s(" %c", &askHijack, 1);
                            ClearInput(input);
                            if (askHijack == 'y') {
                                if (LoadShellcode(szDllPath, pID) == FALSE) {
                                    LogError(L" - (HIJACK) Failed to hijack in target process", 0);
                                    break; 
                                }
                                else {
                                    LogSuccess(L" - (HIJACK) Successfull injection");
                                    SaveConfig(szDllPath, proc);
                                    break;
                                }
                                
                            }
                            else if (askHijack == 'n') {
                                LogError(L"Injection failed", NULL);
                                break;
                            }
                            else {
                                LogError(L"Please enter a valid option [y/n]", NULL);
                            }
                        }
                        break; 
                    }
                    else {
                        LogSuccess(L"Successfull injection");
                        SaveConfig(szDllPath, proc);
                        break; 
                    }
                }
            }
            break;

        default:
            LogError(L"No a valid option. Please enter an option between 0 and 4", NULL);
            break;
        }
    }
}






