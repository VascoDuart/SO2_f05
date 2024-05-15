#include "utils.h"

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hFich, hMap;
	char* pStr;
	DWORD aux = 0,i=0,j=TAM-1;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif


	hFich = CreateFile(NOME_FICH, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);		// != NULL
	if (hFich == NULL) {	//hFich == INVALID_HANDLE_VALUE
		_tprintf_s(_T("\nErro ao criar ficheiro\n"));
		CloseHandle(hFich);
		return -1; 
	}

	hMap = CreateFileMapping(hFich, NULL, PAGE_READWRITE, 0, TAM * sizeof(char), NULL);			//!=NULL
	if (hMap == NULL) {			
		_tprintf_s(_T("\nErro ao mapear ficheiro\n"));
		CloseHandle(hMap);
		return -1;
	}

	pStr = (char*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, TAM * sizeof(char));	//!=NULL
	if (pStr == NULL) {
		_tprintf_s(_T("\nErro ao criar map view\n"));
		UnmapViewOfFile(pStr);
		return -1;
	}

	for (i = 0, j = TAM - 1; i < (TAM - 1) / 2; i++, j--) {
		aux = pStr[i];
		pStr[i] = pStr[j];
		pStr[j] = aux;
	}




	_tprintf(_T("%c"), pStr[0]);


	UnmapViewOfFile(pStr);
	CloseHandle(hMap);
	CloseHandle(hFich);

	return 0;
}