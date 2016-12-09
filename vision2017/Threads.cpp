#include "Threads.h"
#include "VisionInit.h"
#include "ArmorDetect.h"
#include "BigMark.h"


#ifdef UNICODE
#define CreateEvent CreateEventW
#else
#define CreateEvent CreateEventA
#endif


//extern CSerialPort LSerialPort;
extern VisionInit LInit;
extern Threads LThreads;
extern ArmorDetect LArmorDetect;
extern BigMark LBigMark;

extern byte MODEFLAG;
extern bool CamSetFlag;


Threads::Threads(void)
{
}


Threads::~Threads(void)
{
}


/* �¼�������ʼ�� */
void Threads::ThreadParamInit(void)
{
	EventLeftWait = CreateEvent(NULL, TRUE, TRUE, NULL);
	EventRightWait = CreateEvent(NULL, TRUE, TRUE, NULL);

	EventMainLWait = CreateEvent(NULL, TRUE, FALSE, NULL);
	EventMainRWait = CreateEvent(NULL, TRUE, FALSE, NULL);

}


/* ���߳� */
void Threads::MainThread(void)
{
	DWORD ThreadID1;
	DWORD ThreadID2;
//	DWORD ThreadID3;

	ThreadParamInit();
	LArmorDetect.ShootInit();
	//LBigMark.Init();

	/* �����߳� */
	ThreadCam1 = CreateThread(NULL, 100 * 1024 * 1024, CameraReadL, this, 0, &ThreadID1);
	ThreadCam2 = CreateThread(NULL, 100 * 1024 * 1024, CameraReadR, this, 0, &ThreadID2);

	Sleep(1000);

	CloseHandle(ThreadCam1);
	CloseHandle(ThreadCam2);

	while (1)
	{
		/** �ȴ��߳�1��2 */
		WaitForSingleObject(EventMainLWait, INFINITE);
		ResetEvent(EventMainLWait);
		WaitForSingleObject(EventMainRWait, INFINITE);
		ResetEvent(EventMainRWait);

		/** ���浽ͼƬ������ */
		BufImageL = ImageL;
		BufImageR = ImageR;

		SetEvent(EventLeftWait);
		SetEvent(EventRightWait);

		if (CamSetFlag == 0)
		{
			switch (MODEFLAG)
			{
				/* ����ģʽ */
			case(SHOOTENEMY) :
				LInit.SetCamShoot();
				break;

				/* ���ģʽ */
			case(BIGSYMBOL) :
				LInit.SetCamMark();
				break;

			default:
				break;
			}
			CamSetFlag = 1;
		}

		switch (MODEFLAG)
		{
		case(SHOOTENEMY) :
			LArmorDetect.Shooting(BufImageL, BufImageR);
			break;

		case(BIGSYMBOL) :
			LBigMark.ShootMark(BufImageL, BufImageR);
			break;

		default:
			break;
		}
	}
}

/* �������ȡ�߳� */
DWORD WINAPI Threads::CameraReadL(LPVOID lpParamter)
{
	Threads *pClass = reinterpret_cast<Threads*>(lpParamter);
	while (1)
	{
		WaitForSingleObject(pClass->EventLeftWait, INFINITE);
		ResetEvent(pClass->EventLeftWait);
		try
		{
			GrabImage(&pClass->ImageL, LInit.AcqHandleL);
		}
		catch (...)
		{
			LInit.CameraLTest();
		}
		SetEvent(pClass->EventMainLWait);
	}
	return 0;
}


/* �������ȡ�߳� */
DWORD WINAPI Threads::CameraReadR(LPVOID lpParamter)
{
	Threads *pClass = reinterpret_cast<Threads*>(lpParamter);
	while (1)
	{
		WaitForSingleObject(pClass->EventRightWait, INFINITE);
		ResetEvent(pClass->EventRightWait);
		try
		{
			GrabImage(&pClass->ImageR, LInit.AcqHandleR);
		}
		catch (...)
		{
			LInit.CameraRTest();
		}
		SetEvent(pClass->EventMainRWait);
	}
	return 0;
}



