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
	MODEFLAG = BIGSYMBOL;//默认大符模式
	ROB = RED;//默认攻击蓝色装甲
	LSerialPort.SerialPortTest();//串口测试
	LInit.Init();//摄像头初始化
	LThreads.MainThread();//主线程函数
	/*以下代码为串口通信测试代码，需注释掉摄像头初始化和主线程函数*/
	//while (1)
	//{
	//	i = i % 50;
	//	Sleep(1000);//延时
	//	LSerialPort.SendData((float)(i+0.5), (float)(i+1.1), (float)(i+2.2), (float)(i+6.6), (float)(i+7.7), Shoot);
	//	i++;
	//}
	return 0;
}

