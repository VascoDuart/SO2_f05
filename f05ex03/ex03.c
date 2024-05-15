#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

//PRODUTOR.EXE

#define NOME_SHM _T("mem")
#define NOME_SEM_L _T("livres")
#define NOME_SEM_O _T("ocupados")
#define NOME_MUTEX _T("mutex_in");
#define TAM 10

typedef struct {
	DWORD id, valor;
}ITEM;

typedef struct {
	DWORD p, c, in, out;
	ITEM item[TAM];
}SDATA;

typedef struct {
	HANDLE hMutexIN, hSemL, hSemO;
	SDATA* shm;
	BOOL continua;
	DWORD id, conta;
}TDATA;

DWORD WINAPI produz(LPVOID dados) {
	TDATA* td = (TDATA*)dados;
	ITEM it;

	do {
		//CRIAR ITEM (ID, VALOR); ESPERAR SEM_L, ESPERAR MUTEX_IN; ESCREVER SHM; LIBERTAR MUTEX_IN;
		//  ASSINALAR SEM_O; MOSTRAR INFO/ITEM, DORMI 2-4 s

		it.id = td->id;
		it.valor = rand() % (99 - 10 + 1) + 10;		//valor de 10 a 99

		WaitForSingleObject(td->hSemL, INFINITE);
		WaitForSingleObject(td->hMutexIN, INFINITE);

		CopyMemory(&td->shm->item[td->shm->in], &it, sizeof(ITEM));
		td->shm->in = (td->shm->in + 1) % TAM;
		ReleaseMutex(td->hMutexIN);
		ReleaseSemaphore(td->hSemO, 1, NULL);
		td->conta++;
		_tprintf(_T("Item %d produzido... (%d %d)"), td->conta, it.id, it.valor);
		Sleep((rand() % (4 - 2 + 1) + 2) * 1000);		//2 a 4 segundos

	} while (td->continua);

	return 0;
}



int _tmain(int argc, TCHAR* argv[]) {

	HANDLE  hMap, hThread;
	TCHAR str[TAM];
	TDATA td;
	BOOL primeiro;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	td.hMutexIN = CreateMutex(NULL, FALSE, _T("trinco")); //CRIAR MUTEX - "trinco"
	td.hSemL = CreateSemaphore(NULL, TAM, TAM, NOME_SEM_L);
	td.hSemO = CreateSemaphore(NULL, TAM, TAM, NOME_SEM_O);


	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SDATA), NOME_SHM);
	if (hMap != NULL && GetLastError() != ERROR_ALREADY_EXISTS) {
		primeiro = TRUE;
	}
	td.shm = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);	//!=NULL
	if ( primeiro) {
		td.shm->c = 0;
		td.shm->p = 0;
		td.shm->in = 0;
		td.shm->out = 0;
	}
	td.id = td.shm->p++;
	td.continua = TRUE;
	hThread = CreateThread(NULL, 0, produz, (LPVOID)&td, 0, NULL);//CRIAR THREAD

	do {
		_tscanf_s(_T("%s"), str, TAM - 1);
	} while (_tcscmp(str, _T("fim")) != 0);
	td.continua = FALSE;
	WaitForSingleObject(hThread, INFINITE);
	UnmapViewOfFile(td.shm);
	CloseHandle(hMap);
	CloseHandle(td.hSemL);
	CloseHandle(td.hSemO);
	CloseHandle(td.hMutexIN);


	return 0;
}