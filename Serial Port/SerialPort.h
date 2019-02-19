#include <iostream>
#include "Windows.h"
#include "atlstr.h"
#include "CString"
#include "stdlib.h"
#include "stdio.h"
#include <io.h>
#include "conio.h"
#include <tchar.h>

#define READ_TIMEOUT	500
const wchar_t COM[] = L"\\\\.\\COM3";

const int BUFSIZE = 4096;

void COMsetup(DCB *dcb);
BOOL WriteABuffer(char * lpBuf, DWORD dwToWrite);
BOOL readData(char *rBuf, int *Len, DWORD *dwRead);
