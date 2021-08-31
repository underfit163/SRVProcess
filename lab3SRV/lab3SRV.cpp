// lab3SRV.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include "wincrypt.h"
using namespace std;

HANDLE hResultMailslot;

HANDLE hMultProcessMailslot;
HANDLE hMultProcessMutex1;
HANDLE hMultProcessMutex2;





void log(const char* str);
int createResultMailslot();
int createMultProcess();
int createProcesses();
int createProcessesMailslots();
void stopMultProcess();
const char* p = "PASSWORD"; // Пароль, для которого считаем хеш, а потом генерируем ключ
DWORD  dw;
DWORD  dw1;
HCRYPTPROV hCryptProv;
BYTE r;
HCRYPTHASH hCryptHash;// Хеш-объект для алгоритма MD4
HCRYPTKEY hCryptKey; // Ключ для шифрования
DWORD cryptBlockSize; // Длина данных
DWORD bytesback; // Длина новых данных
LPCWSTR key = L"key";
int main()
{
	setlocale(LC_ALL, "rus");
	log("started");
	hMultProcessMutex1 = CreateMutex(NULL, TRUE, L"MultProcessMutex1");
	hMultProcessMutex2 = CreateMutex(NULL, TRUE, L"MultProcessMutex2");

	
	log("mutexes created");
	createResultMailslot();
	log("mailslot created");
	createProcesses();
	
	createProcessesMailslots();
	
	
	stopMultProcess();


	CloseHandle(hResultMailslot);

	log("stop");

	system("pause");

	return 0;
}




void stopMultProcess()
{
	

	//WaitForSingleObject(hMultProcessMutex2, INFINITE);//
	CloseHandle(hMultProcessMailslot);
	CloseHandle(hMultProcessMutex1);
	CloseHandle(hMultProcessMutex2);
	log("Close Mult");
}






int createResultMailslot()
{
	hResultMailslot = CreateMailslot(
		L"\\\\.\\mailslot\\$ResultMailslot$", 0,
		MAILSLOT_WAIT_FOREVER, NULL);

	if (hResultMailslot == INVALID_HANDLE_VALUE)
	{
		log("ResultMailslot not create");
		return 0;
		
	}
	return 1;
}

int createProcesses()
{
	return createMultProcess();
}
int createMultProcess()
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	PROCESS_INFORMATION pi;

	//LPCWSTR lpar = L"C:\\Users\\163ty\\Desktop\\SRV3lab\\lab3SRV\\x64\\Debug\\SRVlabMul.exe"
	if (!CreateProcess(L"C:\\Users\\163ty\\Desktop\\SRV3lab\\lab3SRV\\x64\\Debug\\SRVlabMul.exe", NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
	{
		log("MultProcess not created");
		return 0;
		
	}

	log("MultProcess created");

	return 1;
}



int createProcessesMailslots()
{
	ReleaseMutex(hMultProcessMutex1);
	log("MultProcessMutex1 unlocked");
	WaitForSingleObject(hMultProcessMutex1, INFINITE);
	log("MultProcessMutex1 locked");

	hMultProcessMailslot = CreateFile(
		L"\\\\.\\mailslot\\$MultProcessMailslot$", GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hMultProcessMailslot == INVALID_HANDLE_VALUE)
	{
		log("dont CreateFile");
		return 0;
	}
	//Запрос
	ReleaseMutex(hMultProcessMutex2);
	log("MultProcessMutex2 unlocked");
	//ждать
	WaitForSingleObject(hMultProcessMutex2, INFINITE);
	//Читаем r
	if (!ReadFile(hResultMailslot, &r, sizeof(r), &dw, NULL))
	{
		return 0;
	}
	//Генерация из пароля p ключа k: MD4
	CryptAcquireContext(&hCryptProv, key, NULL, PROV_RSA_FULL, 0);
	CryptCreateHash(hCryptProv, CALG_MD4, 0, 0, &hCryptHash);// Создать хеш-объект
	CryptHashData(hCryptHash, (BYTE*)p, strlen(p), 0);// Посчитать хеш от пароля
	CryptDeriveKey(hCryptProv, CALG_MD4, hCryptHash, 0, &hCryptKey);// Ключ для  из хеша
	//Шифрование числа r, шифром RC2 с ключом k, получаем y
	cryptBlockSize = sizeof(r);
	bytesback = sizeof(r);
	CryptEncrypt(hCryptKey, NULL, TRUE, 0, (BYTE*)r, &cryptBlockSize, 0);// Попытаться с 0-м буфером
	BYTE* y = new BYTE[cryptBlockSize];// Буфер реально требуемой длины
	memcpy(y, (BYTE*)r, sizeof(r)); // Скопировать данные в новый буфер
	CryptEncrypt(hCryptKey, NULL, TRUE, 0, y, &bytesback, cryptBlockSize); // Шифруем																		  
	//Отправка y
	if (!WriteFile(hMultProcessMailslot, y, sizeof(&y),
		&dw1, NULL))
	{
		return 0;
	}
//собщение об отправке Y
	ReleaseMutex(hMultProcessMutex1);
	log("MultProcessMutex1 unlocked");

	WaitForSingleObject(hMultProcessMutex1, INFINITE);

	return 1;
}



void log(const char* str)//Сообщение о выполняемой операции
{
	SYSTEMTIME sm;
	GetLocalTime(&sm);
	cout << sm.wHour << ":" << sm.wMinute << ":" << sm.wSecond << "." << sm.wMilliseconds << " > ";
	printf("%s\n", str);
}
