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

	PauseShoot=0x20, // ��ͣ��� 
	Shoot =0x21,     //���

	BigSymbolShoot = 0x40,  //������
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
	Byte03, // slope distance  minPC��00�Ǹ�λ
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
}Uart3FrameFormatIndex;  //�����շ�����֡��ʽ��˳��

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

	/** ��ʼ�����ں���
	*
	*  @param:  UINT portNo ���ڱ��,Ĭ��ֵΪ1,��COM1
	*  @param:  UINT baud   ������,Ĭ��Ϊ115200
	*  @param:  char parity �Ƿ������żУ��,'Y'��ʾ��Ҫ��żУ��,'N'��ʾ����Ҫ��żУ��
	*  @param:  UINT databits ����λ�ĸ���,Ĭ��ֵΪ8������λ
	*  @param:  UINT stopsbits ֹͣλʹ�ø�ʽ,Ĭ��ֵΪ1
	*  @param:  DWORD dwCommEvents Ĭ��ΪEV_RXCHAR,��ֻҪ�շ�����һ���ַ�,�����һ���¼�
	*  @return: bool  ��ʼ���Ƿ�ɹ�
	*  @note:   ��ʹ�����������ṩ�ĺ���ǰ,���ȵ��ñ��������д��ڵĳ�ʼ��
	*���������� \n�������ṩ��һЩ���õĴ��ڲ�������,����Ҫ����������ϸ��DCB����,��ʹ�����غ���
	*           \n������������ʱ���Զ��رմ���,�������ִ�йرմ���
	*  @see:
	*/
	bool InitPort(UINT  portNo = 3, UINT  baud = CBR_115200, char  parity = 'N', UINT  databits = 8,
		UINT  stopsbits = 1, DWORD dwCommEvents = EV_RXCHAR);

	/** ���ڳ�ʼ������
	*
	*  �������ṩֱ�Ӹ���DCB�������ô��ڲ���
	*  @param:  UINT portNo
	*  @param:  const LPDCB & plDCB
	*  @return: bool  ��ʼ���Ƿ�ɹ�
	*  @note:   �������ṩ�û��Զ���ش��ڳ�ʼ������
	*  @see:
	*/
	bool InitPort(UINT  portNo, const LPDCB& plDCB);


	/** �򿪴���
	*
	*
	*  @param:  UINT portNo �����豸��
	*  @return: bool  ���Ƿ�ɹ�
	*  @note:
	*  @see:
	*/
	bool openPort(UINT  portNo);


	/** �رմ���
	*
	*
	*  @return: void  �����Ƿ�ɹ�
	*  @note:
	*  @see:
	*/
	void ClosePort();

	/** ���������߳�
	*
	*  �������߳���ɶԴ������ݵļ���,�������յ������ݴ�ӡ����Ļ���
	*  @return: bool  �����Ƿ�ɹ�
	*  @note:   ���߳��Ѿ����ڿ���״̬ʱ,����flase
	*  @see:
	*/
	bool OpenListenThread();

	/** �رռ����߳�
	*
	*
	*  @return: bool  �����Ƿ�ɹ�
	*  @note:   ���ñ�������,�������ڵ��߳̽��ᱻ�ر�
	*  @see:
	*/
	bool CloseListenTread();

	/** �򴮿�д����
	*
	*  ���������е�����д�뵽����
	*  @param:  unsigned char * pData ָ����Ҫд�봮�ڵ����ݻ�����
	*  @param:  unsigned int length ��Ҫд������ݳ���
	*  @return: bool  �����Ƿ�ɹ�
	*  @note:   length��Ҫ����pData��ָ�򻺳����Ĵ�С
	*  @see:
	*/
	bool WriteData(unsigned char* pData, unsigned int length);


	/** ��ȡ���ڻ������е��ֽ���
	*
	*
	*  @return: UINT  �����Ƿ�ɹ�
	*  @note:   �����ڻ�������������ʱ,����0
	*  @see:
	*/
	UINT GetBytesInCOM();

	/** ��ȡ���ڽ��ջ�������һ���ֽڵ�����
	*
	*
	*  @param:  char & cRecved ��Ŷ�ȡ���ݵ��ַ�����
	*  @return: bool  ��ȡ�Ƿ�ɹ�
	*  @note:
	*  @see:
	*/
	bool ReadChar(char &cRecved);
	bool ReadBytes(unsigned char* cRecved, unsigned int length);

	/**���ں�**/
	int portNum;

	bool syncFlag;
	UINT32 ComSendCount;
	UINT32 *ComRecCount;
	UINT32 ComRxCount;

	int ReadCount;
	byte LastFlag;
	/*����һ��32λ��-------------------------------------------*/
	void SendWord(UINT8 command, UINT32 data);
	void SendData(float Data0, float Data1, float Data2, float Data3, float Data4, int DataId);
	UINT ReadData(unsigned char* cRecved, int len);

	/** ����Ӳ�����
	*
	*
	*/
	bool SerialPortTest(void);

private:

	/** ���ڼ����߳�
	*
	*  �������Դ��ڵ����ݺ���Ϣ
	*  @param:  void * pParam �̲߳���
	*  @return: UINT WINAPI �̷߳���ֵ
	*  @note:
	*  @see:
	*/
	static UINT WINAPI ListenThread(void* pParam);


	void RxCallBackFunc(UINT32 BytesInQue);//ÿһ������Ψһ�Ļص���������������

	/** ���ھ�� */
	HANDLE  m_hComm;

	/** �߳��˳���־���� */
	static bool s_bExit;

	/** �߳̾�� */
	volatile HANDLE    m_hListenThread;

	/** ͬ������,�ٽ������� */
	CRITICAL_SECTION   m_csCommunicationSync;       //!< �����������

	/*ʱ��ͬ��*/
	HANDLE hSyncEvent;
	UINT32 t2;
	double dly;
	double dev;
	double dt;
	double T;
	double K;

	OVERLAPPED m_osRead, m_osWrite; // �ص�I/O
	OVERLAPPED _wait_o; //WaitCommEvent useo;

	/*�Զ���--------------------------------------------------------*/
private:
	typedef struct
	{
		unsigned char DLC;
		unsigned char Data[64];
	} UsartRxMsg;

	UINT8 SumCheck(UINT8 *pData, UINT32 length);

	/* INT14ת��-----------------------------------------------*/
	UINT16 POS_ChangSign(INT16 num);

	/* ���ڷ��ͺ��� */
	void SendXyzt(UINT8 command, INT16 x, INT16 y, UINT16 z, UINT32 t);

	UINT32 ReadWord(UINT8 *pData);

private:
	UINT32 SlaveTick;
	double  SlaveTickRecTime;
};

#endif //SERIALPORT_H_
