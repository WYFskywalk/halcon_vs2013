#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <Windows.h>
#include "minwindef.h"
#include "stdafx.h"



#define UartFrameHead1 0x44
#define UartFrameHead2 0xBB
#define UartFrameTail 0xFF


typedef enum{
	Enemy0,
	Enemy1,
	Enemy2,
	Enemy3,
	Enemy4,
	Enemy5,
	Enemy6,
	Enemy7,
	Enemy8,
	Enemy9,

	PauseShoot=0x20, // 暂停射击 
	Shoot =0x21,     //射击

	BigSymbolShoot = 0x40,  //射击大符
	SudokuRegionNotFind = 0x41, //
	SudokuNumNotFind = 0x42,
	MatchedSudoku = 0x43,
	SingleSudoku = 0x44,
	SudokuPrepare = 0x45,
	
	PasswordRegionNotFind = 0x50,
	PasswordNumNotFind = 0x51,
	PasswordPrepare =0x52,
	PasswordOK = 0x53,
	
	MinPCCameraError = 0xEC,
	MinPCSerialPortTest = 0xED,
	ShutDownCallBack = 0xEE,

	BigSymbol =0xF8,
	ShootEnemy = 0XF9,
	RedDetect = 0xFA,
	BlueDetect = 0xFB,
	ShutDown = 0xFF,
}DataId;

typedef enum{
	Head1 = 0,  //0x44
	Head2,			//0xBB
	DataID,
	Byte03, // slope distance  minPC，00是高位
	Byte02,
	Byte01,
	Byte00,
	Byte13, // x
	Byte12,
	Byte11,
	Byte10,
	Byte23, // y
	Byte22,
	Byte21,
	Byte20,
	Byte33, // kong
	Byte32,
	Byte31,
	Byte30,
	Byte43, // 
	Byte42,
	Byte41,
	Byte40,

	SumChecki,
	Tail,				//0xFF
	End,				//0xFF
	FrameLength,    //length = 26
}Uart3FrameFormatIndex;  //串口收发数据帧格式的顺序。

union Format32To8
{
	unsigned char u8[4];
	INT32 u32;
	float f32;
};



class CSerialPort
{
public:
	CSerialPort(void);
	~CSerialPort(void);

public:

	/** 初始化串口函数
	*
	*  @param:  UINT portNo 串口编号,默认值为1,即COM1
	*  @param:  UINT baud   波特率,默认为115200
	*  @param:  char parity 是否进行奇偶校验,'Y'表示需要奇偶校验,'N'表示不需要奇偶校验
	*  @param:  UINT databits 数据位的个数,默认值为8个数据位
	*  @param:  UINT stopsbits 停止位使用格式,默认值为1
	*  @param:  DWORD dwCommEvents 默认为EV_RXCHAR,即只要收发任意一个字符,则产生一个事件
	*  @return: bool  初始化是否成功
	*  @note:   在使用其他本类提供的函数前,请先调用本函数进行串口的初始化
	*　　　　　 \n本函数提供了一些常用的串口参数设置,若需要自行设置详细的DCB参数,可使用重载函数
	*           \n本串口类析构时会自动关闭串口,无需额外执行关闭串口
	*  @see:
	*/
	bool InitPort(UINT  portNo = 3, UINT  baud = CBR_115200, char  parity = 'N', UINT  databits = 8,
		UINT  stopsbits = 1, DWORD dwCommEvents = EV_RXCHAR);

	/** 串口初始化函数
	*
	*  本函数提供直接根据DCB参数设置串口参数
	*  @param:  UINT portNo
	*  @param:  const LPDCB & plDCB
	*  @return: bool  初始化是否成功
	*  @note:   本函数提供用户自定义地串口初始化参数
	*  @see:
	*/
	bool InitPort(UINT  portNo, const LPDCB& plDCB);


	/** 打开串口
	*
	*
	*  @param:  UINT portNo 串口设备号
	*  @return: bool  打开是否成功
	*  @note:
	*  @see:
	*/
	bool openPort(UINT  portNo);


	/** 关闭串口
	*
	*
	*  @return: void  操作是否成功
	*  @note:
	*  @see:
	*/
	void ClosePort();

	/** 开启监听线程
	*
	*  本监听线程完成对串口数据的监听,并将接收到的数据打印到屏幕输出
	*  @return: bool  操作是否成功
	*  @note:   当线程已经处于开启状态时,返回flase
	*  @see:
	*/
	bool OpenListenThread();

	/** 关闭监听线程
	*
	*
	*  @return: bool  操作是否成功
	*  @note:   调用本函数后,监听串口的线程将会被关闭
	*  @see:
	*/
	bool CloseListenTread();

	/** 向串口写数据
	*
	*  将缓冲区中的数据写入到串口
	*  @param:  unsigned char * pData 指向需要写入串口的数据缓冲区
	*  @param:  unsigned int length 需要写入的数据长度
	*  @return: bool  操作是否成功
	*  @note:   length不要大于pData所指向缓冲区的大小
	*  @see:
	*/
	bool WriteData(unsigned char* pData, unsigned int length);


	/** 获取串口缓冲区中的字节数
	*
	*
	*  @return: UINT  操作是否成功
	*  @note:   当串口缓冲区中无数据时,返回0
	*  @see:
	*/
	UINT GetBytesInCOM();

	/** 读取串口接收缓冲区中一个字节的数据
	*
	*
	*  @param:  char & cRecved 存放读取数据的字符变量
	*  @return: bool  读取是否成功
	*  @note:
	*  @see:
	*/
	bool ReadChar(char &cRecved);
	bool ReadBytes(unsigned char* cRecved, unsigned int length);

	/**串口号**/
	int portNum;

	bool syncFlag;
	UINT32 ComSendCount;
	UINT32 *ComRecCount;
	UINT32 ComRxCount;

	int ReadCount;
	byte LastFlag;
	/*发送一个32位数-------------------------------------------*/
	void SendWord(UINT8 command, UINT32 data);
	void SendData(float Data0, float Data1, float Data2, float Data3, float Data4, int DataId);
	UINT ReadData(unsigned char* cRecved, int len);

	/** 串口硬件检测
	*
	*
	*/
	bool SerialPortTest(void);

private:

	/** 串口监听线程
	*
	*  监听来自串口的数据和信息
	*  @param:  void * pParam 线程参数
	*  @return: UINT WINAPI 线程返回值
	*  @note:
	*  @see:
	*/
	static UINT WINAPI ListenThread(void* pParam);


	void RxCallBackFunc(UINT32 BytesInQue);//每一个串口唯一的回调函数，暴力虚拟

	/** 串口句柄 */
	HANDLE  m_hComm;

	/** 线程退出标志变量 */
	static bool s_bExit;

	/** 线程句柄 */
	volatile HANDLE    m_hListenThread;

	/** 同步互斥,临界区保护 */
	CRITICAL_SECTION   m_csCommunicationSync;       //!< 互斥操作串口

	/*时间同步*/
	HANDLE hSyncEvent;
	UINT32 t2;
	double dly;
	double dev;
	double dt;
	double T;
	double K;

	OVERLAPPED m_osRead, m_osWrite; // 重叠I/O
	OVERLAPPED _wait_o; //WaitCommEvent useo;

	/*自定义--------------------------------------------------------*/
private:
	typedef struct
	{
		unsigned char DLC;
		unsigned char Data[64];
	} UsartRxMsg;

	UINT8 SumCheck(UINT8 *pData, UINT32 length);

	/* INT14转换-----------------------------------------------*/
	UINT16 POS_ChangSign(INT16 num);

	/* 串口发送函数 */
	void SendXyzt(UINT8 command, INT16 x, INT16 y, UINT16 z, UINT32 t);

	UINT32 ReadWord(UINT8 *pData);

private:
	UINT32 SlaveTick;
	double  SlaveTickRecTime;
};

#endif //SERIALPORT_H_
