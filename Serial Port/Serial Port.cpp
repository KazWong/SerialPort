// Serial Port.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include "SerialPort.h"

using namespace std;

//_ConnectionPtr m_pConnection, m_pConn_access; 
//_RecordsetPtr m_pRecordset,m_pRecord_access;
HANDLE hComm;

int _tmain(int argc, _TCHAR* argv[])
{
	BOOL fWaitingOnRead = FALSE;
	DCB dcb;
	DWORD dwRead;
	OVERLAPPED osReader = {0};
	OVERLAPPED osWriter = {0};
	char *wBuf = new char;
	char rBuf[BUFSIZE] = "";
	char ch;

	CoInitialize(NULL);
	printf("COM Setup....\n");
	COMsetup(&dcb);
	Sleep(1500);
	printf("Setup Finished...\n\n");

	while(true) {
		if ( _kbhit() ) {
			ch = getch();
			*wBuf = ch;
			if ( !WriteABuffer(wBuf, 1) ) {
				printf("WRITE ERROR\n");
				//system("Pause");
			}
		}
		int Len = sizeof(rBuf);	
		if ( !readData(rBuf, &Len, &dwRead) ) {
			printf("READ ERROR\n");
			//system("Pause");
			//exit(0);
		}

		rBuf[dwRead] = '\0';
		printf("%s\n", rBuf);
		Sleep(100);
		system("cls");
	}

	system("Pause");
	CloseHandle(hComm);
	CoUninitialize();
	return 0;
}


void COMsetup(DCB *dcb) {
	hComm = CreateFile(COM,  
					   GENERIC_READ | GENERIC_WRITE, 
					   0, 
					   0, 
					   OPEN_EXISTING,
					   FILE_FLAG_OVERLAPPED,                    
					   0);	

	FillMemory(dcb, sizeof(*dcb), 0);
	dcb->BaudRate = CBR_115200;
	dcb->Parity = NOPARITY;
	dcb->ByteSize = 8;
	dcb->StopBits = ONESTOPBIT;

	if ((hComm == INVALID_HANDLE_VALUE)
		|| (!GetCommState(hComm, dcb)) 
		|| (!SetCommState(hComm, dcb)) ) {
		printf("Cannot open COM!\n");
		printf("Conform the device is connect to correct COM\n\n");
		printf("=================STATUS=================\n");
		printf("COM Port : \t\t%s\n", COM);
		printf("Bit Per Second : \t%i\n", CBR_115200);
		printf("\n");
		system("Pause");
		exit(0);
	}
}

BOOL WriteABuffer(char * lpBuf, DWORD dwToWrite) {
	OVERLAPPED osWrite = {0};
	DWORD dwWritten;
	DWORD dwRes = 0;
	BOOL fRes;

	// Create this write operation's OVERLAPPED structure's hEvent.
	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osWrite.hEvent == NULL)
		// error creating overlapped event handle
		return FALSE;

	// Issue write.
	if (!WriteFile(hComm, lpBuf, dwToWrite, &dwWritten, &osWrite)) {
		if (GetLastError() != ERROR_IO_PENDING) { 
			// WriteFile failed, but isn't delayed. Report error and abort.
			fRes = FALSE;
		}else
			// Write is pending.
			dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
		switch(dwRes)
		{
			// OVERLAPPED structure's event has been signaled. 
		case WAIT_OBJECT_0:
			if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, FALSE))
				fRes = FALSE;
			else
				// Write operation completed successfully.
				fRes = TRUE;
			break;

		default:
			// An error has occurred in WaitForSingleObject.
			// This usually indicates a problem with the
			// OVERLAPPED structure's event handle.
			fRes = FALSE;
			break;
		}
	} else
		// WriteFile completed immediately.
		fRes = TRUE;

	CloseHandle(osWrite.hEvent);
	return fRes;
}

BOOL readData(char *rBuf, int *Len, DWORD *dwRead) {
	OVERLAPPED osRead = {0};
	BOOL fRes = false;
	DWORD dwRes;
	
	osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osRead.hEvent == NULL)
		// error creating overlapped event handle
		return FALSE;

	if (!ReadFile(hComm, rBuf, *Len, dwRead, &osRead)) {
		if (GetLastError() == ERROR_IO_PENDING) {
			dwRes = WaitForSingleObject(osRead.hEvent, READ_TIMEOUT);
		switch(dwRes)
		{
			// Read completed.
		case WAIT_OBJECT_0:
			if (!GetOverlappedResult(hComm, &osRead, dwRead, FALSE))
				// Error in communications; report it.
				printf("Reead Overlapped ERROR\n");
			else
			// Read completed successfully.
			fRes = true;

			//  Reset flag so that another opertion can be issued.
			break;

		case WAIT_TIMEOUT:
			// Operation isn't complete yet. fWaitingOnRead flag isn't
			// changed since I'll loop back around, and I don't want
			// to issue another read until the first one finishes.
			//
			// This is a good time to do some background work.
			printf("Read Timeout\n");
			break;                       

		default:
			// Error in the WaitForSingleObject; abort.
			// This indicates a problem with the OVERLAPPED structure's
			// event handle.
			break;
		}
		}
		GetOverlappedResult(hComm, &osRead, dwRead, FALSE);
	} else
		fRes = true;
	
	CloseHandle(osRead.hEvent);
	return fRes;
}
