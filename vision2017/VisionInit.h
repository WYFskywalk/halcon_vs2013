/*******************************************************************************
*			�Ӿ���ʼ������
*   ������ͷ
*	������ʾ����
*	����ͷ������ȡ
*	���Ӳ�����
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

	/** ����������ھ�� */
	HTuple  WindowHandleL, WindowHandleR;
	/** ���������� */
	HTuple  AcqHandleL, AcqHandleR;
	/** �������MAP */
	HObject MapL, MapR;
	/* ��ʾ���� */
	HObject StreetGrid;

	/** ��������� */
	HTuple  CamParamL;
	/** ��������� */
	HTuple  CamParamR;


	/** ���λ�ù�ϵ */
	HTuple  RelPose;

private:
	/** ������������ */
	HTuple CamParamOutL;
	/** ������������ */
	HTuple CamParamOutR;

public:
	/* �����ʼ�� */
	void Init(void);

	/** �����Ӳ�����
	*
	*	ѭ�������������ڷ��͹��ϡ�ֱ������ָ�
	*/
	bool CameraLTest(void);

	/** �����Ӳ�����
	*
	*	ѭ�������������ڷ��͹��ϡ�ֱ������ָ�
	*/
	bool CameraRTest(void);



	/* �������������� */
	void SetCamShoot(void);

	/* ���ô��������� */
	void SetCamMark(void);

private:

	/* ��˫Ŀ��� */
	void OpenCameraAll(void);

	/* �������� */
	void CreatWindow(void);

	/** ���������ʼ��
	*
	* @note����ȡ��������������̬
	*/
	void CameraParamInit(void);

	/* ��ȡ��������� */
	void ReadCamParamL(double* InputCamParamL);

	/* ��ȡ��������� */
	void ReadCamParamR(double* InputCamParamR);

	/* ��ȡ������λ�ù�ϵ */
	void ReadRealPose(double* InputRelPose);



};



#endif