// SRVlabMul.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include "wincrypt.h"

using namespace std;
char   szBuf[512];
HANDLE hMulMailslot;
HANDLE cfRes;
HANDLE hMutex1;
HANDLE hMutex2;

const char* q = "PASSWORD"; // Пароль, для которого считаем хеш, а потом генерируем ключ
HCRYPTPROV hCryptProv;
BYTE r;
HCRYPTHASH hCryptHash;// Хеш-объект для алгоритма MD4
HCRYPTKEY hCryptKey; // Ключ для шифрования
DWORD cryptBlockSize; // Длина данных
DWORD bytesback; // Длина новых данных
BYTE* y;// Буфер реально требуемой длины
BYTE* x;// Буфер реально требуемой длины
LPCWSTR key = L"key";

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
    while ((hMutex1 = CreateMutex(NULL, FALSE, L"MultProcessMutex1")) == 0);
    log("mutex1 opened");
    WaitForSingleObject(hMutex1, INFINITE);
    log("mutex1 locked");
    while ((hMutex2 = CreateMutex(NULL, FALSE, L"MultProcessMutex2")) == 0);
    log("mutex2 opened");
    hMulMailslot = CreateMailslot(
        L"\\\\.\\mailslot\\$MultProcessMailslot$", 0,
        MAILSLOT_WAIT_FOREVER, NULL);
    if ( hMulMailslot == INVALID_HANDLE_VALUE)
    {
        log("hMulMailslot not create");
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
    //Ждать
    log("mutex2 locked");
    WaitForSingleObject(hMutex2, INFINITE);
    //Генерация случайного числа
    CryptAcquireContext(&hCryptProv, key, NULL, PROV_RSA_FULL, 0);
    CryptGenRandom(hCryptProv, 1, &r);
    if (!WriteFile(cfRes, &r, sizeof(r),
        &dw, NULL))
    {
        return 0;
    }
    ReleaseMutex(hMutex2);
    log("mutex1 unlocked");
    
//Ждать
        WaitForSingleObject(hMutex1, INFINITE);
        log("mutex1 locked");


        //Генерация из эталонного q ключа k
        CryptCreateHash(hCryptProv, CALG_MD4, 0, 0, &hCryptHash);// Создать хеш-объект
        CryptHashData(hCryptHash, (BYTE*)q, strlen(q), 0);// Посчитать хеш от пароля
        CryptDeriveKey(hCryptProv, CALG_MD4, hCryptHash, 0, &hCryptKey);// Ключ для  из хеша

        //расшифровать x=F(Y,K)  
        if (!ReadFile(hMulMailslot, y, sizeof(&y), &dw1, NULL))
        {
            return 0;
        }
        bytesback = sizeof(&y);

        CryptDecrypt(hCryptKey, NULL, TRUE, 0, (BYTE*)y, &bytesback);
        memcpy(x, (BYTE*)y, sizeof(y)); // Скопировать данные в новый буфер
        if (x == (BYTE*)r)
        {
            cout << "Пароль верен";
        }
        else
        {
            cout << "Пароль не верен";

            return 0;
        }
        ReleaseMutex(hMutex1);        
    
    CloseHandle(hMulMailslot);
    CloseHandle(cfRes);
    CloseHandle(hMutex1);
    CloseHandle(hMutex2);
    log("End Mul");
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