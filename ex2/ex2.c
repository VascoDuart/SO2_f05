#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

#define NOME_SHM _T("memória")
#define NOME_EVENT _T("evento")
#define NOME_MUTEX _T("trinco");
#define TAM 100

typedef struct {
	HANDLE hMutex, hEv;
	TCHAR* str;
	BOOL continua;
}TDATA;

DWORD WINAPI mostra(LPVOID dados) {
	TDATA* td = (TDATA*)dados;
	TCHAR str[TAM];
	do {
		//ESPERAR EVENTO, ESPERAR MUTEX, LER SHM, LIBERTAR MUTEX, MOSTRAR INFORMAÇÃO, DORMIR 1 SEGUNDO
		WaitForSingleObject(td->hEv, INFINITE);
		WaitForSingleObject(td->hMutex, INFINITE);
		CopyMemory(str, td->str, (_tcslen(str) + 1) * sizeof(TCHAR));
		ReleaseMutex(td->hMutex);
		_tprintf_s(_T("LI: %s\n"), str);
		Sleep(1000);
	} while (td->continua);

	return 0;
}



int _tmain(int argc, TCHAR * argv[]) {

	HANDLE  hMap,hThread;
	TCHAR str[TAM];
	TDATA td;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	td.hMutex = CreateMutex(NULL,FALSE, _T("trinco")); //CRIAR MUTEX - "trinco"
	td.hEv = CreateEvent(NULL, TRUE, FALSE, NOME_EVENT);//CRIAR EVENTO


	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, TAM * sizeof(char), NULL);			//!=NULL
	td.str = (char*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);	//!=NULL
	td.continua = TRUE;
	hThread = CreateThread(NULL, 0, mostra, (LPVOID) &td, 0, NULL);//CRIAR THREAD

	do {
		_tscanf_s(_T("%s"), str, TAM - 1);
		WaitForSingleObject(td.hMutex, INFINITE);
		CopyMemory(td.str, str, (_tcslen(str) + 1) * sizeof(TCHAR));
		ReleaseMutex(td.hMutex);
		SetEvent(td.hEv);
		Sleep(500);
		ResetEvent(td.hEv);
	} while (_tcscmp(str, _T("fim")) != 0);

	// LER TEXTO, ESPERAR MUTEX, ESCREVER SHM, LIBERTAR MUTEX, ASSINALAR EVENTO, DORMIR 0.5s, RESET EVENT

	td.continua = FALSE;
	WaitForSingleObject(hThread, INFINITE);
	UnmapViewOfFile(td.str);
	CloseHandle(hMap);
	CloseHandle(td.hEv);
	CloseHandle(td.hMutex);


	return 0;
}