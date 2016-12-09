


#include "stdafx.h"
#include "SerialPort.h"
#include "VisionInit.h"
#include "BigMark.h"
#include "ArmorDetect.h"

#include <iostream>
#include <stdio.h>

#include <process.h>
#include <array>
#include <algorithm>

#pragma   comment(lib, "winmm.lib ")
#define LISTENING_INT

/** �����ⲿ���� */
extern CSerialPort LSerialPort;
extern VisionInit LInit;
extern BigMark LBigMark;

extern byte MODEFLAG;

/** �߳��˳���־ */
bool CSerialPort::s_bExit = false;
bool avoidFlag = false;


extern int chCar;
extern double ImageTimeHal;
extern HTuple curTime;
extern bool CamSetFlag;
extern bool ROB;


//���ڷ���
#define DEBUG(format,...)	printf("FILE: %s, LINE: %d:\n"format, __FILE__,__LINE__, ##__VA_ARGS__)

#define CallBackMode 1

CSerialPort::CSerialPort(void)
	: m_hListenThread(INVALID_HANDLE_VALUE)
{
	m_hComm = INVALID_HANDLE_VALUE;
	hSyncEvent = CreateEventW(NULL, FALSE, FALSE, NULL); //��һ���ȴ��̱߳��ͷ�ʱ���Զ�����Ϊ���ź�״̬����ʼ�����ź�״̬
	InitializeCriticalSection(&m_csCommunicationSync);
	syncFlag = false;
	K = 1;
	ComSendCount = 0;
	ComRecCount = NULL;
	ComRxCount = 0;
	ReadCount = 0;
	LastFlag = 0;
}

CSerialPort::~CSerialPort(void)
{
	CloseListenTread();
	ClosePort();
	DeleteCriticalSection(&m_csCommunicationSync);
	CloseHandle(hSyncEvent);
}

bool CSerialPort::InitPort(UINT portNo /*= 1*/, UINT baud /*= CBR_115200*/, char parity /*= 'N'*/,
	UINT databits /*= 8*/, UINT stopsbits /*= 1*/, DWORD dwCommEvents /*= EV_RXCHAR*/)
{

	/** ��ʱ����,���ƶ�����ת��Ϊ�ַ�����ʽ,�Թ���DCB�ṹ */
	char szDCBparam[50];
	sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

	/** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,�����벻Ҫ�ӱ��� */
	if (!openPort(portNo))
	{
		return false;
	}

	/** �����ٽ�� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** �Ƿ��д����� */
	BOOL bIsSuccess = TRUE;

	/** �ڴ˿���������������Ļ�������С,���������,��ϵͳ������Ĭ��ֵ.
	*  �Լ����û�������Сʱ,Ҫע�������Դ�һЩ,���⻺�������
	*/
	/*if (bIsSuccess )
	{
	bIsSuccess = SetupComm(m_hComm,10,10);
	}*/

	/** ���ô��ڵĳ�ʱʱ��,����Ϊ0,��ʾ��ʹ�ó�ʱ���� */
	COMMTIMEOUTS  CommTimeouts;
	CommTimeouts.ReadIntervalTimeout = 0;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutMultiplier = 200;
	CommTimeouts.WriteTotalTimeoutConstant = 5000;

	if (bIsSuccess)
	{
		bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
	}

	DCB  dcb;
	if (bIsSuccess)
	{
		// ��ANSI�ַ���ת��ΪUNICODE�ַ���
		DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, NULL, 0);
		wchar_t *pwText = new wchar_t[dwNum];
		if (!MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
		{
			bIsSuccess = FALSE;
			/** �ͷ��ڴ�ռ� */
			delete[] pwText;
			LeaveCriticalSection(&m_csCommunicationSync);
			return false;
		}

		/** ��ȡ��ǰ�������ò���,���ҹ��촮��DCB���� */
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(pwText, &dcb);
		/** ����RTS flow���� */
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		/** �ͷ��ڴ�ռ� */
		delete[] pwText;
	}

	if (bIsSuccess)
	{
		/** ʹ��DCB�������ô���״̬ */
		bIsSuccess = SetCommState(m_hComm, &dcb);
	}

	/**  ��մ��ڻ����� */
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** �뿪�ٽ�� */
	LeaveCriticalSection(&m_csCommunicationSync);

	/*���ںŸ�ֵ�������ж���*/
	portNum = portNo;
	return bIsSuccess == TRUE;
}

bool CSerialPort::InitPort(UINT portNo, const LPDCB& plDCB)
{
	/** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,�����벻Ҫ�ӱ��� */
	if (!openPort(portNo))
	{
		return false;
	}

	/** �����ٽ�� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** ���ô��ڲ��� */
	if (!SetCommState(m_hComm, plDCB))
	{
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/**  ��մ��ڻ����� */
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** �뿪�ٽ�� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

void CSerialPort::ClosePort()
{
	/** ����д��ڱ��򿪣��ر��� */
	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}
	if (m_osWrite.hEvent != INVALID_HANDLE_VALUE)
		CloseHandle(m_osWrite.hEvent);
}

bool CSerialPort::openPort(UINT portNo)
{
	/** �����ٽ�� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** �Ѵ��ڵı��ת��Ϊ�豸�� */
	char szPort[50];
	sprintf_s(szPort, "\\\\.\\COM%d", portNo);

	// ��ANSI�ַ���ת��ΪUNICODE�ַ���
	DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, szPort, -1, NULL, 0);
	wchar_t *pwPort = new wchar_t[dwNum];
	if (!MultiByteToWideChar(CP_ACP, 0, szPort, -1, pwPort, dwNum))
	{
		/** �ͷ��ڴ�ռ� */
		delete[] pwPort;
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** ��ָ���Ĵ��� */
	m_hComm = CreateFile(pwPort,		                /** �豸��,COM1,COM2�� */
		GENERIC_READ | GENERIC_WRITE,  /** ����ģʽ,��ͬʱ��д */
		0,                             /** ����ģʽ,0��ʾ������ */
		NULL,							/** ��ȫ������,һ��ʹ��NULL */
		OPEN_EXISTING,					/** �ò�����ʾ�豸�������,���򴴽�ʧ�� */
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,//dwFlagsAndAttributes������ָ���ô����Ƿ�����첽��������ֵΪFILE_FLAG_OVERLAPPED����ʾʹ���첽��I/O����ֵΪ0����ʾͬ��I/O������
		0);

	/** �����ʧ�ܣ��ͷ���Դ������ */
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** �˳��ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);

	memset(&m_osWrite, 0, sizeof(m_osWrite));
	m_osWrite.hEvent = CreateEventW(NULL, true, false, NULL);

	return true;
}

bool CSerialPort::OpenListenThread()
{
	/** ����߳��Ƿ��Ѿ������� */
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** �߳��Ѿ����� */
		return false;
	}

	//�����¼�---------------------------------------------------------
	BOOL fSuccess;

	// Set the event mask. 
	fSuccess = SetCommMask(m_hComm, EV_RXCHAR | EV_ERR);

	if (!fSuccess)
	{
		// Handle the error. 
		printf("SetCommMask failed with error %d.\n", GetLastError());
		return false;
	}

	memset(&m_osRead, 0, sizeof(m_osRead));
	m_osRead.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);


	memset(&_wait_o, 0, sizeof(_wait_o));
	_wait_o.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);


	//�����߳�---------------------------------------------------------------------
	s_bExit = false;
	/** �߳�ID */
	UINT threadId;
	/** �����������ݼ����߳� */
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!m_hListenThread)
	{
		return false;
	}
	/** �����̵߳����ȼ�,������ͨ�߳� */
	if (!SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL))
	{
		return false;
	}

	return true;
}

bool CSerialPort::CloseListenTread()
{
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** ֪ͨ�߳��˳� */
		s_bExit = true;

		/** �ȴ��߳��˳� */
		Sleep(10);

		/** ���߳̾����Ч */
		if (m_hListenThread != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hListenThread);
			m_hListenThread = INVALID_HANDLE_VALUE;
		}

		if (m_osRead.hEvent != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_osRead.hEvent);
			m_osRead.hEvent = INVALID_HANDLE_VALUE;
		}

		if (_wait_o.hEvent != INVALID_HANDLE_VALUE)
		{
			CloseHandle(_wait_o.hEvent);
			_wait_o.hEvent = INVALID_HANDLE_VALUE;
		}

	}
	return true;
}

UINT CSerialPort::GetBytesInCOM()
{
	DWORD dwError = 0;	/** ������ */
	COMSTAT  comstat;   /** COMSTAT�ṹ��,��¼ͨ���豸��״̬��Ϣ */
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;

	/* �ڵ���ReadFile��WriteFile֮ǰ,ͨ�������������ǰ�����Ĵ����־ */
	if (ClearCommError(m_hComm, &dwError, &comstat))
	{
		BytesInQue = comstat.cbInQue; /** ��ȡ�����뻺�����е��ֽ��� */
	}

	return BytesInQue;
}






UINT WINAPI CSerialPort::ListenThread(void* pParam)
{
	/** �õ������ָ�� */
	CSerialPort *pSerialPort = reinterpret_cast<CSerialPort*>(pParam);

	COMSTAT  ComStat;
	DWORD dwErrorFlags;

	unsigned char buf[2048];
	UINT32 index_i = 0;
	unsigned char come_data = 0;
	UsartRxMsg UsartRx;
	memset(&UsartRx, 0, sizeof(UsartRxMsg));
	if (pSerialPort->m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	if (!SetCommMask(pSerialPort->m_hComm, EV_RXCHAR))
		return 0;
	for (DWORD length, mask = 0; !pSerialPort->s_bExit; mask = 0)
	{
		try{
			if (!WaitCommEvent(pSerialPort->m_hComm, &mask, &pSerialPort->_wait_o))
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					ClearCommError(pSerialPort->m_hComm, &dwErrorFlags, &ComStat);
					GetOverlappedResult(pSerialPort->m_hComm, &pSerialPort->_wait_o, &length, TRUE);
				}
				else
				{
					throw 1;
					DEBUG("COM ERR: !!!");
				}
			}
			if (mask & EV_ERR) // == EV_ERR
			{
				throw 1;
				DWORD dwError = GetLastError();
				DEBUG("COM ERR: %d!!!", dwError);
				ClearCommError(pSerialPort->m_hComm, &dwErrorFlags, &ComStat);
			}
		}
		catch (...){
			if (LSerialPort.SerialPortTest())
				break;
		}

		if (mask & EV_RXCHAR) // == EV_RXCHAR
		{
			UINT32 BytesInQue = pSerialPort->GetBytesInCOM();

#if CallBackMode == 1
			/* ��ȡ���뻺�����е����ݲ������ʾ */
			//char cRecved = 0x00;
			//while (BytesInQue--)
			//{
			//	cRecved = 0x00;
			//	if (pSerialPort->ReadChar(cRecved) == true)
			//	{
			//		std::cout << cRecved;
			//	}
			//}
			pSerialPort->RxCallBackFunc(BytesInQue);
#else
			if (BytesInQue > 2048)
				BytesInQue = 2048;
			pSerialPort->ReadBytes(buf, BytesInQue);

			for (index_i = 0; index_i < BytesInQue; index_i++)
			{
				come_data = buf[index_i];

				if (come_data == 0xff)  /* ���������ʼ�ֽ� */
				{
					UsartRx.DLC = 0;
				}
				else
				{
					UsartRx.Data[UsartRx.DLC++] = come_data;
					if (UsartRx.DLC == 13)	//���ݳ���Ϊ 13+1
					{

					}
					else if (UsartRx.DLC > 63)
					{
						UsartRx.DLC = 63;
					}
				}
			}
#endif
		}
	}
	return 0;
}




bool CSerialPort::ReadChar(char &cRecved)
{
	BOOL  bResult = TRUE;
	DWORD BytesRead = 0;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	/* �ٽ������� */
	EnterCriticalSection(&m_csCommunicationSync);

	/* �ӻ�������ȡһ���ֽڵ����� */
	bResult = ReadFile(m_hComm, &cRecved, 1, &BytesRead, &m_osRead);
	if (!bResult)
	{
		throw 1;
		/* ��ȡ������,���Ը��ݸô�����������ԭ�� */
		DWORD dwError = GetLastError();
		DEBUG("COM ERR: %d!!!/n", dwError);

		DWORD dwErrorFlags;
		COMSTAT  ComStat;
		ClearCommError(m_hComm, &dwErrorFlags, &ComStat);
		/* ��մ��ڻ����� */
		//PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/* �뿪�ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return (BytesRead == 1);

}

bool CSerialPort::ReadBytes(unsigned char* cRecved, unsigned int length)
{
	//COMSTAT  ComStat;

	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD dwBytesRead;
	//dwBytesRead = min(length, (DWORD)ComStat.cbInQue);

	//if (!dwBytesRead)
	//	return 0;

	/* �ٽ������� */
	EnterCriticalSection(&m_csCommunicationSync);

	BOOL bResult;
	bResult = ReadFile(m_hComm, cRecved, length, &dwBytesRead, &m_osRead);

	if (!bResult) //���ReadFile��������FALSE
	{
		DWORD dwError = GetLastError();
		if (ERROR_IO_PENDING != dwError)
			DEBUG("COM ERR: %d!!!/n", dwError);

		DWORD dwErrorFlags;
		COMSTAT  ComStat;
		ClearCommError(m_hComm, &dwErrorFlags, &ComStat);

		//PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		dwBytesRead = 0;
	}

	/* �뿪�ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return (dwBytesRead == length);
}

bool CSerialPort::WriteData(unsigned char* pData, unsigned int length)
{
	BOOL   bResult = TRUE;
	DWORD  dwBytesWritten = 0;

	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	/* �ٽ������� */
	EnterCriticalSection(&m_csCommunicationSync);

	/* �򻺳���д��ָ���������� */
	bResult = WriteFile(m_hComm, pData, length, &dwBytesWritten, &m_osWrite);

	if (!bResult)
	{
		DWORD dwError = GetLastError();
		if (GetLastError() != ERROR_IO_PENDING)
		{
			DEBUG("COM ERR: %d!!!/n", dwError);
			/* ��մ��ڻ����� */
			//PurgeComm(m_hComm, PURGE_TXABORT);
			LeaveCriticalSection(&m_csCommunicationSync);
			return false;
		}
		else
		{
			COMSTAT ComStat;
			ClearCommError(m_hComm, &dwError, &ComStat);
		}
	}

	/* �뿪�ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

UINT8 CSerialPort::SumCheck(UINT8 *pData, UINT32 length)
{
	UINT32 sum = 0;
	for (UINT32 i = 0; i < length; i++)
	{
		sum += *pData++;
	}
	return (UINT8)(sum & 0x7f);
}


/* ����32λ�޷����� */
void CSerialPort::SendWord(UINT8 command, UINT32 data)
{
	UINT8 DataTemp[14];
	UINT32 num = ComSendCount++;
	DataTemp[0] = 0xff;
	DataTemp[1] = command;
	DataTemp[2] = (data >> 0) & 0x7f;
	DataTemp[3] = (data >> 7) & 0x7f;
	DataTemp[4] = (data >> 14) & 0x7f;
	DataTemp[5] = (data >> 21) & 0x7f;
	DataTemp[6] = (data >> 28) & 0x0f;
	DataTemp[7] = 0;
	DataTemp[8] = 0;
	DataTemp[9] = 0;
	DataTemp[10] = (num >> 0) & 0x7f;
	DataTemp[11] = (num >> 7) & 0x7f;
	DataTemp[12] = (num >> 14) & 0x7f;
	DataTemp[13] = SumCheck(&DataTemp[2], 11);

	WriteData(DataTemp, 14);
}

/* ��ȡ32λ�޷����� */
UINT32 CSerialPort::ReadWord(UINT8 *pData)
{
	return (UINT32)((pData[5] << 28) | (pData[4] << 21) | (pData[3] << 14) | (pData[2] << 7) | pData[1]);
}



/* 14λ�з����� */
UINT16 CSerialPort::POS_ChangSign(INT16 num)
{
	if (num < 0)
	{
		num = -num;
		num |= 0x2000;
	}
	else
	{
		num = num;
	}
	return num;
}


/*--------------------------------�Զ���--------------------------------*/
void CSerialPort::SendData(float Data0, float Data1, float Data2, float Data3, float Data4, int DataId)
{
	unsigned char SendBuff[FrameLength];
	SendBuff[Head1] = UartFrameHead1;
	SendBuff[Head2] = UartFrameHead2;
	SendBuff[DataID] = DataId;
	SendBuff[Tail] = UartFrameTail;
	SendBuff[End] = 0xff;
	
	Format32To8 temp32To8;

	temp32To8.f32 = Data0;
	SendBuff[Byte00] = temp32To8.u8[0];
	SendBuff[Byte01] = temp32To8.u8[1];
	SendBuff[Byte02] = temp32To8.u8[2];
	SendBuff[Byte03] = temp32To8.u8[3];
	temp32To8.f32 = Data1;
	SendBuff[Byte10] = temp32To8.u8[0];
	SendBuff[Byte11] = temp32To8.u8[1];
	SendBuff[Byte12] = temp32To8.u8[2];
	SendBuff[Byte13] = temp32To8.u8[3];
	temp32To8.f32 = Data2;
	SendBuff[Byte20] = temp32To8.u8[0];
	SendBuff[Byte21] = temp32To8.u8[1];
	SendBuff[Byte22] = temp32To8.u8[2];
	SendBuff[Byte23] = temp32To8.u8[3];
	temp32To8.f32 = Data3;
	SendBuff[Byte30] = temp32To8.u8[0];
	SendBuff[Byte31] = temp32To8.u8[1];
	SendBuff[Byte32] = temp32To8.u8[2];
	SendBuff[Byte33] = temp32To8.u8[3];
	temp32To8.f32 = Data4;
	SendBuff[Byte40] = temp32To8.u8[0];
	SendBuff[Byte41] = temp32To8.u8[1];
	SendBuff[Byte42] = temp32To8.u8[2];
	SendBuff[Byte43] = temp32To8.u8[3];
	SendBuff[SumChecki] = (unsigned char)(SendBuff[Byte00] + SendBuff[Byte01] +
		SendBuff[Byte02] + SendBuff[Byte03] +
		SendBuff[Byte10] + SendBuff[Byte11] +
		SendBuff[Byte12] + SendBuff[Byte13] +
		SendBuff[Byte20] + SendBuff[Byte21] +
		SendBuff[Byte22] + SendBuff[Byte23] +
		SendBuff[Byte30] + SendBuff[Byte31] +
		SendBuff[Byte32] + SendBuff[Byte33] +
		SendBuff[Byte40] + SendBuff[Byte41] +
		SendBuff[Byte42] + SendBuff[Byte43]
		)
		; 

	WriteData(SendBuff, 26);

}


UINT CSerialPort::ReadData(unsigned char *cRecved, int len)
{
	char temp;
	for (int i = 0; i < len; ++i)
	{
		try{
			ReadChar(temp);
			cRecved[i] = temp;
		}
		catch (...){
			if (SerialPortTest())
				break;
		}
	}
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);//��ȡȡ������
	return len;
}


void CSerialPort::RxCallBackFunc(UINT32 BytesInQue)
{
	if (BytesInQue != FrameLength)
	{
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);//��ȡȡ������

		return;
	} 
	unsigned char RxBuff[FrameLength];
	ReadData(RxBuff, BytesInQue);
	//cout << "weadgffwweeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" <<RxBuff[DataID] << endl;

	switch (RxBuff[DataID])
	{
		/* �ػ� */
	case (ShutDown) :
		if (ReadCount == 0)
			LastFlag = ShutDown;
		if (RxBuff[DataID] == LastFlag)
		{
			ReadCount++;
		}
		else
		{
			ReadCount = 0;
		}
		if (ReadCount >= 3)
		{
			SendData(0, 0, 0, 0, 0, ShutDownCallBack);
			system("shutdown -s -t 3");
			if (ReadCount > 10)
				ReadCount = 10;
		}

		break;
		/* ���� */
	case(ShootEnemy) :
		LastFlag = SHOOTENEMY;
		MODEFLAG = SHOOTENEMY;
		CamSetFlag = 0;
		break;
		/* ��� */
	case(BigSymbol) :
		LastFlag = BIGSYMBOL;
		MODEFLAG = BIGSYMBOL;
		CamSetFlag = 0;
		cout << "���" << endl;
		break;
	case(RedDetect) :
		ROB = RED;
		CamSetFlag = 0;
		cout << "������ɫ" << endl;
		break;
	case(BlueDetect) :
		ROB = BLUE;
		CamSetFlag = 0;
		cout << "������ɫ" << endl;
		break;
	default:
		break;
	}
}


bool CSerialPort::SerialPortTest(void)
{
	cout << "SerialPort Error..." << endl;
	CloseListenTread();
	ClosePort();
	int port = 0;
	for (;;)
	{
		try{
			port++;
			if (port >= 1000)
			{
				port = 0;
			}
			if (!InitPort(port % 11, CBR_115200, 'N', 8, 1))
				throw 1;
			if (!OpenListenThread())
				throw 1;
			cout << "������������..." << endl;
			break;
		}
		catch (...){
			cout << "���ڳ�������..." << endl;
			continue;
		}
		break;
	}
	return 1;
}
