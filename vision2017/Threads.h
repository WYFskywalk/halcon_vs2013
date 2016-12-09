/************************************************************************************
*	�߳���
*
*	���̣߳������к���
*	���̣߳������ȡ�߳�
*
*************************************************************************************/

#ifndef THREADS_H
#define THREADS_H

#include "stdafx.h"

class Threads
{
public:
	Threads(void);
	~Threads(void);

private:
	/* �洢��ȡͼ�� */
	HObject ImageL;
	HObject ImageR;

	/* ��ȡͼ�񻺳� */
	HObject BufImageL;
	HObject BufImageR;

	/* �߳̾�� */
	HANDLE ThreadCam1;
	HANDLE ThreadCam2;


	/* �¼����� */
	HANDLE EventLeftWait;
	HANDLE EventRightWait;

	HANDLE EventMainLWait;
	HANDLE EventMainRWait;

public:

	/* ���̺߳��� */
	void MainThread(void);

private:

	/* �̲߳�����ʼ�� */
	void ThreadParamInit(void);


	/* �������ȡ�߳� */
	static DWORD WINAPI CameraReadL(LPVOID lpParamter);


	/* �������ȡ�߳� */
	static DWORD WINAPI CameraReadR(LPVOID lpParamter);

};


#endif