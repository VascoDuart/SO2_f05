#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

//CONSUMIDOR.EXE

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
	HANDLE hMutexOUT, hSemL, hSemO;
	SDATA* shm;
	BOOL continua;
	DWORD id, conta, soma;
}TDATA;

DWORD WINAPI consome(LPVOID dados) {
	TDATA* td = (TDATA*)dados;
	ITEM it;
	DWORD pos;

	do {
		//ESPERA SEM_O; ESPERAR MUTEX_OUT; LER_SHM; LIBERTAR MUTEX_OUT; ASSINALAR SEM_L;
		//MOSTRAR INFO; INCREMENTA CONTA; CALCULA SOMA; DORMIR 2-4 SEGUNDOS

		WaitForSingleObject(td->hSemO, INFINITE);
		WaitForSingleObject(td->hMutexOUT, INFINITE);
		CopyMemory(&it, &td->shm->item[td->shm->out], sizeof(ITEM));
		pos = td->shm->out;
		td->shm->out = (td->shm->out + 1) % TAM;
		td->conta++;
		td->soma += it.valor;
		ReleaseMutex(td->hMutexOUT);
		ReleaseSemaphore(td->hSemL, 1, NULL);

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

	td.hMutexOUT = CreateMutex(NULL, FALSE, _T("trinco")); //CRIAR MUTEX - "trinco"
	td.hSemL = CreateSemaphore(NULL, TAM, TAM, NOME_SEM_L);
	td.hSemO = CreateSemaphore(NULL, TAM, TAM, NOME_SEM_O);


	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SDATA), NOME_SHM);
	if (hMap != NULL && GetLastError() != ERROR_ALREADY_EXISTS) {
		primeiro = TRUE;

	}
	td.shm = (TCHAR*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);	//!=NULL
	if ( primeiro) {
		WaitForSingleObject(td.hMutexOUT, INFINITE);
		td.shm->c = 0;
		td.shm->p = 0;
		td.shm->in = 0;
		td.shm->out = 0;
	}
	td.id = td.shm->p++;
	td.continua = TRUE;
	hThread = CreateThread(NULL, 0, consome, (LPVOID)&td, 0, NULL);//CRIAR THREAD

	do {
		_tscanf_s(_T("%s"), str, TAM - 1);
	} while (_tcscmp(str, _T("fim")) != 0);
	td.continua = FALSE;
	WaitForSingleObject(hThread, INFINITE);
	UnmapViewOfFile(td.shm);
	CloseHandle(hMap);
	CloseHandle(td.hSemL);
	CloseHandle(td.hSemO);
	CloseHandle(td.hMutexOUT);


	return 0;
}