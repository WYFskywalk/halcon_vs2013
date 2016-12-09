/************************************************************************************
*	线程类
*
*	主线程，主运行函数
*	子线程，相机读取线程
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
	/* 存储读取图像 */
	HObject ImageL;
	HObject ImageR;

	/* 读取图像缓冲 */
	HObject BufImageL;
	HObject BufImageR;

	/* 线程句柄 */
	HANDLE ThreadCam1;
	HANDLE ThreadCam2;


	/* 事件对象 */
	HANDLE EventLeftWait;
	HANDLE EventRightWait;

	HANDLE EventMainLWait;
	HANDLE EventMainRWait;

public:

	/* 主线程函数 */
	void MainThread(void);

private:

	/* 线程参数初始化 */
	void ThreadParamInit(void);


	/* 左相机读取线程 */
	static DWORD WINAPI CameraReadL(LPVOID lpParamter);


	/* 右相机读取线程 */
	static DWORD WINAPI CameraReadR(LPVOID lpParamter);

};


#endif