/*******************************************************************************
*			视觉初始化部分
*   打开摄像头
*	创建显示窗口
*	摄像头参数读取
*	相机硬件检测
*
********************************************************************************/

#ifndef VISIONINIT_H
#define VISIONINIT_H

#include "stdafx.h"



class VisionInit
{
public:
	VisionInit(void);
	~VisionInit(void);

	/** 左右相机窗口句柄 */
	HTuple  WindowHandleL, WindowHandleR;
	/** 左右相机句柄 */
	HTuple  AcqHandleL, AcqHandleR;
	/** 左、右相机MAP */
	HObject MapL, MapR;
	/* 显示区域 */
	HObject StreetGrid;

	/** 左相机参数 */
	HTuple  CamParamL;
	/** 右相机参数 */
	HTuple  CamParamR;


	/** 相对位置关系 */
	HTuple  RelPose;

private:
	/** 左相机输出参数 */
	HTuple CamParamOutL;
	/** 右相机输出参数 */
	HTuple CamParamOutR;

public:
	/* 程序初始化 */
	void Init(void);

	/** 左相机硬件检测
	*
	*	循环检测相机并串口发送故障、直到相机恢复
	*/
	bool CameraLTest(void);

	/** 右相机硬件检测
	*
	*	循环检测相机并串口发送故障、直到相机恢复
	*/
	bool CameraRTest(void);



	/* 设置相机射击参数 */
	void SetCamShoot(void);

	/* 设置大符相机参数 */
	void SetCamMark(void);

private:

	/* 打开双目相机 */
	void OpenCameraAll(void);

	/* 建立窗口 */
	void CreatWindow(void);

	/** 相机参数初始化
	*
	* @note：读取相机参数，相对姿态
	*/
	void CameraParamInit(void);

	/* 读取左相机参数 */
	void ReadCamParamL(double* InputCamParamL);

	/* 读取右相机参数 */
	void ReadCamParamR(double* InputCamParamR);

	/* 读取相机相对位置关系 */
	void ReadRealPose(double* InputRelPose);



};



#endif