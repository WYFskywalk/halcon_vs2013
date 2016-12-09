


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

/** 引用外部对象 */
extern CSerialPort LSerialPort;
extern VisionInit LInit;
extern BigMark LBigMark;

extern byte MODEFLAG;

/** 线程退出标志 */
bool CSerialPort::s_bExit = false;
bool avoidFlag = false;


extern int chCar;
extern double ImageTimeHal;
extern HTuple curTime;
extern bool CamSetFlag;
extern bool ROB;


//串口发送
#define DEBUG(format,...)	printf("FILE: %s, LINE: %d:\n"format, __FILE__,__LINE__, ##__VA_ARGS__)

#define CallBackMode 1

CSerialPort::CSerialPort(void)
	: m_hListenThread(INVALID_HANDLE_VALUE)
{
	m_hComm = INVALID_HANDLE_VALUE;
	hSyncEvent = CreateEventW(NULL, FALSE, FALSE, NULL); //当一个等待线程被释放时，自动重置为无信号状态，初始是无信号状态
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

	/** 临时变量,将制定参数转化为字符串形式,以构造DCB结构 */
	char szDCBparam[50];
	sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

	/** 打开指定串口,该函数内部已经有临界区保护,上面请不要加保护 */
	if (!openPort(portNo))
	{
		return false;
	}

	/** 进入临界段 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 是否有错误发生 */
	BOOL bIsSuccess = TRUE;

	/** 在此可以设置输入输出的缓冲区大小,如果不设置,则系统会设置默认值.
	*  自己设置缓冲区大小时,要注意设置稍大一些,避免缓冲区溢出
	*/
	/*if (bIsSuccess )
	{
	bIsSuccess = SetupComm(m_hComm,10,10);
	}*/

	/** 设置串口的超时时间,均设为0,表示不使用超时限制 */
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
		// 将ANSI字符串转换为UNICODE字符串
		DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, NULL, 0);
		wchar_t *pwText = new wchar_t[dwNum];
		if (!MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
		{
			bIsSuccess = FALSE;
			/** 释放内存空间 */
			delete[] pwText;
			LeaveCriticalSection(&m_csCommunicationSync);
			return false;
		}

		/** 获取当前串口配置参数,并且构造串口DCB参数 */
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(pwText, &dcb);
		/** 开启RTS flow控制 */
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		/** 释放内存空间 */
		delete[] pwText;
	}

	if (bIsSuccess)
	{
		/** 使用DCB参数配置串口状态 */
		bIsSuccess = SetCommState(m_hComm, &dcb);
	}

	/**  清空串口缓冲区 */
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** 离开临界段 */
	LeaveCriticalSection(&m_csCommunicationSync);

	/*串口号赋值，用于判定车*/
	portNum = portNo;
	return bIsSuccess == TRUE;
}

bool CSerialPort::InitPort(UINT portNo, const LPDCB& plDCB)
{
	/** 打开指定串口,该函数内部已经有临界区保护,上面请不要加保护 */
	if (!openPort(portNo))
	{
		return false;
	}

	/** 进入临界段 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 配置串口参数 */
	if (!SetCommState(m_hComm, plDCB))
	{
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/**  清空串口缓冲区 */
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** 离开临界段 */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

void CSerialPort::ClosePort()
{
	/** 如果有串口被打开，关闭它 */
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
	/** 进入临界段 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 把串口的编号转换为设备名 */
	char szPort[50];
	sprintf_s(szPort, "\\\\.\\COM%d", portNo);

	// 将ANSI字符串转换为UNICODE字符串
	DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, szPort, -1, NULL, 0);
	wchar_t *pwPort = new wchar_t[dwNum];
	if (!MultiByteToWideChar(CP_ACP, 0, szPort, -1, pwPort, dwNum))
	{
		/** 释放内存空间 */
		delete[] pwPort;
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** 打开指定的串口 */
	m_hComm = CreateFile(pwPort,		                /** 设备名,COM1,COM2等 */
		GENERIC_READ | GENERIC_WRITE,  /** 访问模式,可同时读写 */
		0,                             /** 共享模式,0表示不共享 */
		NULL,							/** 安全性设置,一般使用NULL */
		OPEN_EXISTING,					/** 该参数表示设备必须存在,否则创建失败 */
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,//dwFlagsAndAttributes：用于指定该串口是否进行异步操作，该值为FILE_FLAG_OVERLAPPED，表示使用异步的I/O；该值为0，表示同步I/O操作；
		0);

	/** 如果打开失败，释放资源并返回 */
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** 退出临界区 */
	LeaveCriticalSection(&m_csCommunicationSync);

	memset(&m_osWrite, 0, sizeof(m_osWrite));
	m_osWrite.hEvent = CreateEventW(NULL, true, false, NULL);

	return true;
}

bool CSerialPort::OpenListenThread()
{
	/** 检测线程是否已经开启了 */
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** 线程已经开启 */
		return false;
	}

	//接收事件---------------------------------------------------------
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


	//开启线程---------------------------------------------------------------------
	s_bExit = false;
	/** 线程ID */
	UINT threadId;
	/** 开启串口数据监听线程 */
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!m_hListenThread)
	{
		return false;
	}
	/** 设置线程的优先级,高于普通线程 */
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
		/** 通知线程退出 */
		s_bExit = true;

		/** 等待线程退出 */
		Sleep(10);

		/** 置线程句柄无效 */
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
	DWORD dwError = 0;	/** 错误码 */
	COMSTAT  comstat;   /** COMSTAT结构体,记录通信设备的状态信息 */
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;

	/* 在调用ReadFile和WriteFile之前,通过本函数清除以前遗留的错误标志 */
	if (ClearCommError(m_hComm, &dwError, &comstat))
	{
		BytesInQue = comstat.cbInQue; /** 获取在输入缓冲区中的字节数 */
	}

	return BytesInQue;
}






UINT WINAPI CSerialPort::ListenThread(void* pParam)
{
	/** 得到本类的指针 */
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
			/* 读取输入缓冲区中的数据并输出显示 */
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

				if (come_data == 0xff)  /* 如果遇到起始字节 */
				{
					UsartRx.DLC = 0;
				}
				else
				{
					UsartRx.Data[UsartRx.DLC++] = come_data;
					if (UsartRx.DLC == 13)	//数据长度为 13+1
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

	/* 临界区保护 */
	EnterCriticalSection(&m_csCommunicationSync);

	/* 从缓冲区读取一个字节的数据 */
	bResult = ReadFile(m_hComm, &cRecved, 1, &BytesRead, &m_osRead);
	if (!bResult)
	{
		throw 1;
		/* 获取错误码,可以根据该错误码查出错误原因 */
		DWORD dwError = GetLastError();
		DEBUG("COM ERR: %d!!!/n", dwError);

		DWORD dwErrorFlags;
		COMSTAT  ComStat;
		ClearCommError(m_hComm, &dwErrorFlags, &ComStat);
		/* 清空串口缓冲区 */
		//PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/* 离开临界区 */
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

	/* 临界区保护 */
	EnterCriticalSection(&m_csCommunicationSync);

	BOOL bResult;
	bResult = ReadFile(m_hComm, cRecved, length, &dwBytesRead, &m_osRead);

	if (!bResult) //如果ReadFile函数返回FALSE
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

	/* 离开临界区 */
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

	/* 临界区保护 */
	EnterCriticalSection(&m_csCommunicationSync);

	/* 向缓冲区写入指定量的数据 */
	bResult = WriteFile(m_hComm, pData, length, &dwBytesWritten, &m_osWrite);

	if (!bResult)
	{
		DWORD dwError = GetLastError();
		if (GetLastError() != ERROR_IO_PENDING)
		{
			DEBUG("COM ERR: %d!!!/n", dwError);
			/* 清空串口缓冲区 */
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

	/* 离开临界区 */
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


/* 发送32位无符号数 */
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

/* 读取32位无符号数 */
UINT32 CSerialPort::ReadWord(UINT8 *pData)
{
	return (UINT32)((pData[5] << 28) | (pData[4] << 21) | (pData[3] << 14) | (pData[2] << 7) | pData[1]);
}



/* 14位有符号数 */
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


/*--------------------------------自定义--------------------------------*/
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
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);//读取取完就清空
	return len;
}


void CSerialPort::RxCallBackFunc(UINT32 BytesInQue)
{
	if (BytesInQue != FrameLength)
	{
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);//读取取完就清空

		return;
	} 
	unsigned char RxBuff[FrameLength];
	ReadData(RxBuff, BytesInQue);
	//cout << "weadgffwweeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" <<RxBuff[DataID] << endl;

	switch (RxBuff[DataID])
	{
		/* 关机 */
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
		/* 攻击 */
	case(ShootEnemy) :
		LastFlag = SHOOTENEMY;
		MODEFLAG = SHOOTENEMY;
		CamSetFlag = 0;
		break;
		/* 大符 */
	case(BigSymbol) :
		LastFlag = BIGSYMBOL;
		MODEFLAG = BIGSYMBOL;
		CamSetFlag = 0;
		cout << "大符" << endl;
		break;
	case(RedDetect) :
		ROB = RED;
		CamSetFlag = 0;
		cout << "攻击红色" << endl;
		break;
	case(BlueDetect) :
		ROB = BLUE;
		CamSetFlag = 0;
		cout << "攻击蓝色" << endl;
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
			cout << "串口正常了啦..." << endl;
			break;
		}
		catch (...){
			cout << "串口出错了啦..." << endl;
			continue;
		}
		break;
	}
	return 1;
}
