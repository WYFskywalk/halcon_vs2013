#include "stdafx.h"
#include "VisionInit.h"
#include "SerialPort.h"
#include "Threads.h"
#include "ArmorDetect.h"
#include "mathhandle.h"
#include "BigMark.h"


CSerialPort LSerialPort;
VisionInit LInit;
Threads LThreads;
ArmorDetect LArmorDetect;
MathClass LMathClass;
BigMark LBigMark;

//SHOOTENEMY  BIGSYMBOL  AUTOGETBULLETHEROCAR  AUTOGETBULLETSTATION
byte MODEFLAG;
bool CamSetFlag = 0;
bool ROB;

int main(int argc, char *argv[])
{
	//int i = 6;
	MODEFLAG = BIGSYMBOL;//Ĭ�ϴ��ģʽ
	ROB = RED;//Ĭ�Ϲ�����ɫװ��
	LSerialPort.SerialPortTest();//���ڲ���
	LInit.Init();//����ͷ��ʼ��
	LThreads.MainThread();//���̺߳���
	/*���´���Ϊ����ͨ�Ų��Դ��룬��ע�͵�����ͷ��ʼ�������̺߳���*/
	//while (1)
	//{
	//	i = i % 50;
	//	Sleep(1000);//��ʱ
	//	LSerialPort.SendData((float)(i+0.5), (float)(i+1.1), (float)(i+2.2), (float)(i+6.6), (float)(i+7.7), Shoot);
	//	i++;
	//}
	return 0;
}

