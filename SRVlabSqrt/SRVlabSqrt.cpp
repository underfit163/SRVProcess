// SRVlabSqrt.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <windows.h>
#include <iostream>
#include <stdio.h>
using namespace std;

HANDLE hSqrtMailslot;
HANDLE cfRes;
HANDLE hMutex1;
HANDLE hMutex2;
double x;
double y;
double result;
void log(const char* str);


int main()
{
    log("started");
    BOOL   fReturnCode;
    DWORD  cbMessages;
    DWORD  cbMsgNumber;
    DWORD  cbRead;
    DWORD  dw;
    DWORD  dw1;
    while ((hMutex1 = CreateMutex(NULL, FALSE, L"SqrtProcessMutex1")) == 0);
    log("mutex1 opened");
    WaitForSingleObject(hMutex1, INFINITE);
    log("mutex1 locked");
    while ((hMutex2 = CreateMutex(NULL, FALSE, L"SqrtProcessMutex2")) == 0);
    log("mutex2 opened");
    hSqrtMailslot = CreateMailslot(
        L"\\\\.\\mailslot\\$SqrtProcessMailslot$", 0,
        MAILSLOT_WAIT_FOREVER, NULL);
    if (hSqrtMailslot == INVALID_HANDLE_VALUE)
    {
        log("hSqrtMailslot not create");
        return 0;
    }
    log("mailslot created");

    cfRes = CreateFile(
        L"\\\\.\\mailslot\\$ResultMailslot$", GENERIC_WRITE,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (cfRes == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    log("cfRes opened");
    ReleaseMutex(hMutex1);
    log("mutex1 unlocked");
    WaitForSingleObject(hMutex2, INFINITE);
    log("mutex2 locked");
    while (1)
    {
        WaitForSingleObject(hMutex1, INFINITE);
        log("mutex1 locked");
        ReleaseMutex(hMutex2);
        log("mutex2 unlocked");
        log("read message");

        fReturnCode = GetMailslotInfo(
            hSqrtMailslot, NULL, &cbMessages,
            &cbMsgNumber, NULL);
        if (!fReturnCode)
        {
            break;
        }
        if (cbMsgNumber != 0)
        {
            
            if (ReadFile(hSqrtMailslot, &x, sizeof(double), &dw, NULL))
            {
                log("read message");
                if (x == -1)
                {
                    log("stop");
                    break;
                }
                result = sqrt(x);
                printf("sqrt(%.3f) = %.3f\n", x, result);

                if (!WriteFile(cfRes, &result, sizeof(double),
                    &dw1, NULL))
                {
                    log("dont write message");
                    return 0;
                }

                log("write message");
            }
            else
            {

                break;
            }
        }
        ReleaseMutex(hMutex1);
        log("mutex1 unlocked");
        WaitForSingleObject(hMutex2, INFINITE);
        log("mutex2 locked");
        
    }
    CloseHandle(hSqrtMailslot);
    CloseHandle(cfRes);
    CloseHandle(hMutex1);
    CloseHandle(hMutex2);
    log("End Sqrt");
    system("pause");

    return 0;
}

void log(const char* str)//Сообщение о выполняемой операции
{
    SYSTEMTIME sm;
    GetLocalTime(&sm);
    cout << sm.wHour << ":" << sm.wMinute << ":" << sm.wSecond << "." << sm.wMilliseconds << " > ";
    printf("%s\n", str);
}