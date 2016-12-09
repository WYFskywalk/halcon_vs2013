#include "BigMark.h"
#include "VisionInit.h"
#include "SerialPort.h"
#include "MathHandle.h"
#include "ArmorDetect.h"

extern CSerialPort LSerialPort;
extern VisionInit LInit;
extern MathClass LMathClass;
extern ArmorDetect LArmorDetect;

using namespace std;
using namespace HalconCpp;



BigMark::BigMark(void)
{
	/*������0*/
	OAP.High = 150;
	OAP.Wide = 200;
	OAP.ReadyCount = 0;
	OAP.StartShoot = 0;
	OAP.LastID = 0;
	OAP.PresentID = 0;
}

BigMark::~BigMark(void)
{

}

/*���̴߳���������*/
void BigMark::ShootMark(const HObject& ImageL, const HObject& ImageR)
{
	bool ReadSudokuFlagL = false;
	bool ReadSudokuFlagR = false;
	/*ȫ�ֱ�����ʼ��*/
	ClearList();
	
	/*OCR�Ķ�����ʼ��*/
	OCR_Ready();

	/*�ָ�������,���ʧ�ܱ�����*/
	if (!PasswordDivition(ImageL, ImageR))
	{
		LSerialPort.SendData(0, 0, 0, 0, 0, PasswordRegionNotFind);
		cout << "δ�ָܷ�������" << endl;
		return;
	}
	/*������ʶ�����ʧ�ܱ�����*/
	if (!ReadPassword())
	{
		LSerialPort.SendData(0, 0, 0, 0, 0, PasswordNumNotFind);
		cout << "δ��ʶ������" << endl;
		return;
	}

	/*���봫��*/
	PasswordTransmit();

	/*�ָ�Ź����������ʧ�ܱ�����*/
	if (!SudokuDivition(ImageL, ImageR))
	{
		LSerialPort.SendData(0, 0, 0, 0, 0, SudokuRegionNotFind);
		cout << "δ�ָܷ�Ź�������" << endl;
		return;
	}

	/*��ȡ�Ź�������*/
	if (LeftID_OK)
	{
		ReadSudokuFlagL = ReadSudokuNumber('L');
	}
	if (RightID_OK)
	{
		ReadSudokuFlagR = ReadSudokuNumber('R');
	}

	/*˫Ŀƥ��&&3D���*/
	if (ReadSudokuFlagL && ReadSudokuFlagR)
	{
		SudokuMatched();			
		/*������ݴ���*/
		MatchedTransmitBigMark();
		cout << "˫Ŀ���ݴ���ɹ�..." << endl;
	}
	/*��Ŀֱ�Ӵ���ID��������3D����*/
	/*�������Ŀ*/
	else if (ReadSudokuFlagL && !ReadSudokuFlagR)
	{
		SingleTransmitBigMark('L');
		cout << "��Ŀ���ݴ���ɹ�..." << endl;
	}
	/*�������Ŀ*/
	else if (!ReadSudokuFlagL && ReadSudokuFlagR)
	{
		SingleTransmitBigMark('R');
		cout << "��Ŀ���ݴ���ɹ�..." << endl;
	}
	/*˫Ŀ�����ȡʧ��*/
	else
	{
		LSerialPort.SendData(0, 0, 0, 0, 0, SudokuNumNotFind);
		cout << "δ�ܶ�ȡ�Ź�������..." << endl;
	}

	/*�ر�OCR�Ķ���*/
	OCR_Close();
}



/*******************************************************************************
Function:ClearList 
Description:���������ʼ��ȫ�ֱ���
Calls:��
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR)
Input:��
Output:��
Return:��
Others:
********************************************************************************/
void BigMark::ClearList(void)
{
	LeftSingleRect.clear();
	RightSingleRect.clear();
	SortedLeftSingleRect.clear();
	SortedRightSingleRect.clear();
	WorldRectPosition.clear();

	LeftCounter = 0;
	RightCounter = 0;
	ReadNum = 0;
	LeftID_OK = false;
	RightID_OK = false;
	WorldPositionMeasure = false;

}



/*******************************************************************************
Function:OCR_Ready 
Description:��ʼ��OCR
Calls:��
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR) 
Input:��
Output:��
Return:��
Others:OCR�ļ�����ص�����Ŀ¼�£��ļ���ö���ں����ڲ�д��
********************************************************************************/
void BigMark::OCR_Ready(void)
{
	/*���п�ѡ���ļ���*/
	/**************************************
	Document_0-9_NoRej.omc
	Document_0-9_Rej.omc
	Document_0-9A-Z_NoRej.omc
	Document_0-9A-Z_Rej.omc
	Document_A-Z+_NoRej.omc
	Document_A-Z+_Rej.omc
	Document_NoRej.omc
	Document_Rej.omc
	DotPrint.omc
	DotPrint_0-9.omc
	DotPrint_0-9+.omc
	DotPrint_0-9A-Z.omc
	DotPrint_A-Z+.omc
	HandWritten_0-9.omc
	Industrial_0-9_NoRej.omc
	Industrial_0-9_Rej.omc
	Industrial_0-9+_NoRej.omc
	Industrial_0-9+_Rej.omc
	Industrial_0-9A-Z_NoRej.omc
	Industrial_0-9A-Z_Rej.omc
	Industrial_A-Z+_NoRej.omc
	Industrial_A-Z+_Rej.omc
	Industrial_NoRej.omc
	Industrial_Rej.omc
	MICR.omc
	OCRA.omc
	OCRA_0-9.omc
	OCRA_0-9A-Z.omc
	OCRA_A-Z+.omc
	OCRB.omc
	OCRB_0-9.omc
	OCRB_0-9A-Z.omc
	OCRB_A-Z+.omc
	OCRB_passport.omc
	Pharma.omc
	Pharma_0-9.omc
	Pharma_0-9+.omc
	Pharma_0-9A-Z.omc
	SEMI.omc
	***************************************/
	OCRStructure.FileNameSudoku = "E:/halcon_vs2013/vision2017/Document_0-9_NoRej.omc";
	ReadOcrClassMlp("E:/halcon_vs2013/vision2017/Document_0-9_NoRej.omc", &OCRStructure.OCRHandlerSudoku);
}



/*******************************************************************************
Function:OCR_Close 
Description:�ر�OCR�ļ�
Calls:��
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR) 
Input:��
Output:��
Return:��
Others:��OCR�������Ҫ���ô˺����ر��ļ�
********************************************************************************/
void BigMark::OCR_Close(void)
{
	ClearOcrClassMlp(OCRStructure.OCRHandlerSudoku);
}



/*******************************************************************************
Function:SudokuDivition 
Description:�ָ�Ź��񣬻�ȡ�Ź��������������ݣ�д��ṹ��LeftSingleRect��RightSingleRect������Ϣ
Calls:
      GetRelativePosition(void)
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR) 
Input:
      ImageL�������ͼ��
      ImageR�������ͼ��
Output:��
Return:�ָ�ɹ�����TRUE,���򷵻�FALSE
Others:���ɹ��ָ���Ź���λ�ã�LeftCounter == 9 || RightCounter == 9�����ܵ���GetRelativePosition()������
       ����ֱ�ӷ��ر���

	   ��Ҫ���£�
	   2016/11/24 ��ǰ��ͼ�ν�������
	   2016/12/8 �򻯴����� 
********************************************************************************/
bool BigMark::SudokuDivition(const HObject& ImageL, const HObject& ImageR)
{
	/*��������ʱ����*/
	HTuple TempNumL, TempNumR;
	
	//LeftCounter = 0;
	//RightCounter = 0;
	/*ת��Ϊ�Ҷ�ͼ��*/
	Rgb1ToGray(ImageL, &SDP.GrayImageL);
	Rgb1ToGray(ImageR, &SDP.GrayImageR);

	/*Ȧ��ROI����*/
	GenRectangle1(&SDP.ImageROI_L, 164, 523, 770, 1517);
	GenRectangle1(&SDP.ImageROI_R, 57, 391, 613, 1302);

	/*ͼ��������*/
	ReduceDomain(SDP.GrayImageL, SDP.ImageROI_L, &SDP.ImageReducedL);
	ReduceDomain(SDP.GrayImageR, SDP.ImageROI_R, &SDP.ImageReducedR);

	/*�Ҷ�ͼת��*/
	//GrayRangeRect(SDP.ImageReducedL, &SDP.ImageResultL, 3, 3);
	//GrayRangeRect(SDP.ImageReducedR, &SDP.ImageResultR, 3, 3);
	
	/*�Ҷ�ֵ��ת*/
	//InvertImage(SDP.ImageResultL, &SDP.ImageInvertL);
	//InvertImage(SDP.ImageResultR, &SDP.ImageInvertR);

	/*��ֵ���ָ�*/
	//Threshold(SDP.ImageResultL, &SDP.RegionL, 50, 255);
	//Threshold(SDP.ImageResultR, &SDP.RegionR, 50, 255);

	/*��ֵ�˲�*/
	MeanImage(SDP.ImageReducedL, &SDP.ImageMeanL, 40, 40);
	MeanImage(SDP.ImageReducedR, &SDP.ImageMeanR, 40, 40);

	/*��ͨ�˲�*/
	DynThreshold(SDP.ImageReducedL, SDP.ImageMeanL, &SDP.RegionDynThreshL, 10, "light");
	DynThreshold(SDP.ImageReducedR, SDP.ImageMeanR, &SDP.RegionDynThreshR, 10, "light");


	/*�������*/
	FillUp(SDP.RegionDynThreshL, &SDP.RegionFillUpL);
	FillUp(SDP.RegionDynThreshR, &SDP.RegionFillUpR);

	/*������ͨ*/
	Connection(SDP.RegionFillUpL, &SDP.ConnectedRegionsL);
	Connection(SDP.RegionFillUpR, &SDP.ConnectedRegionsR);

	/*����ɸѡ*/
	SelectShape(SDP.ConnectedRegionsL, &SDP.SelectedRegionsL1, "area", "and", 10000, 35000);
	SelectShape(SDP.ConnectedRegionsR, &SDP.SelectedRegionsR1, "area", "and", 10000, 35000);

	SelectShape(SDP.SelectedRegionsL1, &SDP.SelectedRegionsL2, "rectangularity", "and", 0.8, 1);
	SelectShape(SDP.SelectedRegionsR1, &SDP.SelectedRegionsR2, "rectangularity", "and", 0.8, 1);

	/*ȡ������ԭ�Ź�������*/
	//Intersection(SDP.RegionL, SDP.SelectedRegionsL2, &SDP.RegionIntersectionL);
	//Intersection(SDP.RegionR, SDP.SelectedRegionsR2, &SDP.RegionIntersectionR);

	/*ɸѡ���������*/
	CountObj(SDP.SelectedRegionsL2, &TempNumL);
	CountObj(SDP.SelectedRegionsR2, &TempNumR);

	cout << TempNumL.I() << endl;
	cout << TempNumR.I() << endl;
	/*���������ѭ��*/
	for (int i = 0; i != TempNumL.I(); ++i)
	{
		try
		{
			/*ѡ��Ŀ�꣬��ȡ��С��Χ���γ���*/
			SelectObj(SDP.SelectedRegionsL2, &SDP.ObjectSelected, i + 1);
			SmallestRectangle2(SDP.ObjectSelected, &SDP.Row, &SDP.Column,
				&SDP.Phi, &SDP.Length1, &SDP.Length2);
			/*������������ĵ�ͱ�Ե������*/
			AreaCenter(SDP.ObjectSelected, &SDP.Area, &SDP.CenterRow, &SDP.CenterColumn);
			SmallestRectangle1(SDP.ObjectSelected, &SDP.Row1, &SDP.Column1, &SDP.Row2, &SDP.Column2);
		}
		catch (...)
		{
			continue;
		}

		/*ɸѡҪ��*/
		if (SDP.Length1 >= RECT_MIN_LENGTH1
			&&SDP.Length1 <= RECT_MAX_LENGTH1
			&&SDP.Length2 >= RECT_MIN_LENGTH2
			&&SDP.Length2 <= RECT_MAX_LENGTH2)
		{		
			/*�������ݵ�����*/
			//TempRect.RectID = LeftCounter++;
			TempRect.Area = (float)SDP.Area.D();
			TempRect.Row1 = (float)SDP.Row1.D();
			TempRect.Column1 = (float)SDP.Column1.D();
			TempRect.Row2 = (float)SDP.Row2.D();
			TempRect.Column2 = (float)SDP.Column2.D();
			TempRect.CenterRow = (float)SDP.CenterRow.D();
			TempRect.CenterColumn = (float)SDP.CenterColumn.D();
			TempRect.Width = (float)SDP.Column2.D() - (float)SDP.Column1.D();
			TempRect.Height = (float)SDP.Row2.D() - (float)SDP.Row1.D();

			LeftSingleRect.push_back(TempRect);
			LeftCounter++;
		}
	}
	
	/*���������ѭ��*/
	for (int j = 0; j!= TempNumR.I(); ++j)
	{
		try
		{
			/*ѡ��Ŀ�꣬��ȡ��С��Χ���γ���*/
			SelectObj(SDP.SelectedRegionsR2, &SDP.ObjectSelected, j + 1);
			SmallestRectangle2(SDP.ObjectSelected, &SDP.Row, &SDP.Column,
				&SDP.Phi, &SDP.Length1, &SDP.Length2);
			/*������������ĵ�ͱ�Ե������*/
			AreaCenter(SDP.ObjectSelected, &SDP.Area, &SDP.CenterRow, &SDP.CenterColumn);
			SmallestRectangle1(SDP.ObjectSelected, &SDP.Row1, &SDP.Column1, &SDP.Row2, &SDP.Column2);
		}
		catch (...)
		{
			continue;
		}
		/*ɸѡҪ��*/
		if (SDP.Length1 >= RECT_MIN_LENGTH1
			&&SDP.Length1 <= RECT_MAX_LENGTH1
			&&SDP.Length2 >= RECT_MIN_LENGTH2
			&&SDP.Length2 <= RECT_MAX_LENGTH2)
		{
			//TempRect.RectID = RightCounter++;
			TempRect.Area = (float)SDP.Area.D();
			TempRect.Row1 = (float)SDP.Row1.D();
			TempRect.Column1 = (float)SDP.Column1.D();
			TempRect.Row2 = (float)SDP.Row2.D();
			TempRect.Column2 = (float)SDP.Column2.D();
			TempRect.CenterRow = (float)SDP.CenterRow.D();
			TempRect.CenterColumn = (float)SDP.CenterColumn.D();
			TempRect.Width = (float)SDP.Column2.D() - (float)SDP.Column1.D();
			TempRect.Height = (float)SDP.Row2.D() - (float)SDP.Row1.D();

			RightSingleRect.push_back(TempRect);
			RightCounter++;
		}
	}

	/*�ҵ��㹻�ľ��ο�*/
	/*note:�ſ�����������������ͷ��һ��ʶ���9�����ο�ʱ���ɽ�����غ�������֤
	������һ������������*/
	if (LeftCounter == 9 || RightCounter == 9)
	{
		/*��ȡ���λ�ú�ID*/
		GetRelativePosition();
		return true;
	}
	else
	{
		LeftCounter = 0;
		RightCounter = 0;
		cout << "Can not find enough Rectangle,Sudoku distinguish error!" << endl;
		return false;
	}
     
}



/*******************************************************************************
Function:GetRelativePosition
Description:��ȡ�Ź������λ�ú�ID��д�뵽�ṹ��SingleRect��Ա����RectID
Calls:��
Called By:
      SudokuDivition(const HObject& ImageL, const HObject& ImageR)
Input:��
Output:��
Return:��
Others:�������ͼ��IDд��ɹ�9�κ�LeftID_OKд1�������ͼ��IDд��ɹ�9�κ�RightID_OKд1
********************************************************************************/
void BigMark::GetRelativePosition(void)
{
    
	/*************ID����*************/
	/*1--------------2-------------3*/


	/*4--------------5-------------6*/


	/*7--------------8-------------9*/

	/*�������*/
	FGRP.GetPositionL = 0;
	FGRP.GetPositionR = 0;
	FGRP.MidColumn = 0;
	FGRP.MidRow = 0;

	/*�������鶨��*/
	int LeftID_Flag[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0};
	int RightID_Flag[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0};
	/*****************************���������ʼ***************************/
	if (LeftCounter == 9)
	{
		for (FGRP.it1 = LeftSingleRect.begin(); FGRP.it1 != LeftSingleRect.end(); ++FGRP.it1)
		{
			FGRP.MidColumn += (double)FGRP.it1->CenterColumn;
			FGRP.MidRow += (double)FGRP.it1->CenterRow;
		}
		/*��ȡ���ĵ��������*/
		FGRP.MidColumn /= 9.0;
		FGRP.MidRow /= 9.0;
		/*************************���λ���жϣ�д��ID****************************/
		/*���������ʼ*/
		for (FGRP.it2 = LeftSingleRect.begin(); FGRP.it2 != LeftSingleRect.end(); ++FGRP.it2)
		{
			/*������λ��ȷ����ʼ*/
			if ((double)FGRP.it2->CenterRow - FGRP.MidRow >= RECT_ROW_DIS)
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && LeftID_Flag[9] == 0)//���½�
				{				
					FGRP.it2->RectID = 9;//д��ID
					FGRP.GetPositionL++;//�������ۼ�
					LeftID_Flag[9] = 1;//����������ӦIDλ��λ
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && LeftID_Flag[7] == 0)//���½�
				{					
					FGRP.it2->RectID = 7;
					FGRP.GetPositionL++;
					LeftID_Flag[7] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn)<RECT_COLUMN_DIS && LeftID_Flag[8] == 0)//���·�
				{
					FGRP.it2->RectID = 8;
					FGRP.GetPositionL++;
					LeftID_Flag[8] = 1;
				}
				else//ID�ѱ�ռ�ã�д��ʧ��
				{
					LeftID_OK = 0;
					//return;
				}
			}
			/*������λ��ȷ������*/

			/*��һ��λ��ȷ����ʼ*/
			else if (FGRP.MidRow - (double)FGRP.it2->CenterRow >= RECT_ROW_DIS)
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && LeftID_Flag[3]==0)//���Ͻ�
				{
					FGRP.it2->RectID = 3;
					FGRP.GetPositionL++;
					LeftID_Flag[3] = 1;
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && LeftID_Flag[1] == 0)//���Ͻ�
				{
					FGRP.it2->RectID = 1;
					FGRP.GetPositionL++;
					LeftID_Flag[1] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn)<RECT_COLUMN_DIS && LeftID_Flag[2] == 0)//���Ϸ�
				{
					FGRP.it2->RectID = 2;
					FGRP.GetPositionL++;
					LeftID_Flag[2] = 1;
				}
				else
				{
					LeftID_OK = 0;
					//return;
				}
			}
			/*��һ��λ��ȷ������*/

			/*�ڶ���λ��ȷ����ʼ*/
			else
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && LeftID_Flag[6] == 0)//���ҷ�
				{
					FGRP.it2->RectID = 6;
					FGRP.GetPositionL++;
					LeftID_Flag[6] = 1;
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && LeftID_Flag[4] == 0)//����
				{
					FGRP.it2->RectID = 4;
					FGRP.GetPositionL++;
					LeftID_Flag[4] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn < RECT_COLUMN_DIS) && LeftID_Flag[5] == 0)//����
				{
					FGRP.it2->RectID = 5;//����
					FGRP.GetPositionL++;
					LeftID_Flag[5] = 1;
				}
				else
				{
					LeftID_OK = 0;
					//return;
				}
			}
			/*�ڶ���λ��ȷ������*/
        
		/*�����������*/
		}

		/*IDȫ��д��ɹ�*/
		if (FGRP.GetPositionL == 9)
		{
			LeftID_OK = 1;//��־λ��1
		}

		else
		{
			LeftID_OK = 0;
		}
	/*������������*/
	}

	/**************************���������ʼ***************************/
	FGRP.MidColumn = 0;
	FGRP.MidRow = 0;
	if (RightCounter == 9)
	{
		for (FGRP.it1 = RightSingleRect.begin(); FGRP.it1 != RightSingleRect.end(); ++FGRP.it1)
		{
			FGRP.MidColumn += (double)FGRP.it1->CenterColumn;
			FGRP.MidRow += (double)FGRP.it1->CenterRow;
		}
		FGRP.MidColumn /= 9.0;
		FGRP.MidRow /= 9.0;
		/*************************���λ���жϣ�д��ID****************************/
		/*���������ʼ*/
		for (FGRP.it2 = RightSingleRect.begin(); FGRP.it2 != RightSingleRect.end(); ++FGRP.it2)
		{
			/*������λ��ȷ����ʼ*/
			if ((double)FGRP.it2->CenterRow - FGRP.MidRow >= RECT_ROW_DIS)
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && RightID_Flag[9] == 0)//���½�
				{
					FGRP.it2->RectID = 9;
					FGRP.GetPositionR++;
					RightID_Flag[9] = 1;
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && RightID_Flag[7] == 0)//���½�
				{
					FGRP.it2->RectID = 7;
					FGRP.GetPositionR++;
					RightID_Flag[7] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn) < RECT_COLUMN_DIS && RightID_Flag[8] == 0)//���·�
				{
					FGRP.it2->RectID = 8;
					FGRP.GetPositionR++;
					RightID_Flag[8] = 1;
				}
				else
				{
					RightID_OK = 0;
				}
			}
			/*������λ��ȷ������*/

			/*��һ��λ��ȷ����ʼ*/
			else if (FGRP.MidRow - (double)FGRP.it2->CenterRow >= RECT_ROW_DIS)
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && RightID_Flag[3] == 0)//���Ͻ�
				{
					FGRP.it2->RectID = 3;
					FGRP.GetPositionR++;
					RightID_Flag[3] = 1;
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && RightID_Flag[1] == 0)//���Ͻ�
				{
					FGRP.it2->RectID = 1;
					FGRP.GetPositionR++;
					RightID_Flag[1] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn) < RECT_COLUMN_DIS && RightID_Flag[2] == 0)//���Ϸ�
				{
					FGRP.it2->RectID = 2;
					FGRP.GetPositionR++;
					RightID_Flag[2] = 1;
				}
				else
				{
					RightID_OK = 0;
				}
			}
			/*��һ��λ��ȷ������*/

			/*�ڶ���λ��ȷ����ʼ*/
			else
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && RightID_Flag[6] == 0)//���ҷ�
				{
					FGRP.it2->RectID = 6;
					FGRP.GetPositionR++;
					RightID_Flag[6] = 1;
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && RightID_Flag[4] == 0)//����
				{
					FGRP.it2->RectID = 4;
					FGRP.GetPositionR++;
					RightID_Flag[4] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn) < RECT_COLUMN_DIS && RightID_Flag[5] == 0)//����
				{
					FGRP.it2->RectID = 5;//����
					FGRP.GetPositionR++;
					RightID_Flag[5] = 1;
				}
				else
				{
					RightID_OK = 0;
				}
			}
			/*�ڶ���λ��ȷ������*/

		/*�����������*/
		}

		if (FGRP.GetPositionR == 9)
		{
			RightID_OK = 1;//��־λ��1
		}
		else
		{
			RightID_OK = 0;
		}
	/*������������*/
	}
}



/*******************************************************************************
Function:ReadSudokuNumber
Description:��ȡ�Ź������֣�д�뵽�ṹ��SingleRect��Ա����NumClass
Calls:��
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR) 
Input:LOR�����ұ�־λ��L��ʾ�����ͼ��R��ʾ�����ͼ��
Output:��
Return:��
Others:���ݱ�ʶ������ͼ�����ʶ��CONFIDENCE��Ϊ���Ŷ��о�
       
	   ��Ҫ���£�
	   2016/11/25 ��д������ȡ���룬�������������ϸ����Ǳ�ڵ���ʶ��
	   2016/12/8 ����VarThreshold���ӣ������и�����
********************************************************************************/
bool BigMark::ReadSudokuNumber(char LOR)
{
	HTuple  WindowHandle1, WindowHandle2;
	ReadNum = 0;
	/*���ݱ�ʶ��������Ӧͼ��*/
	if (LOR == 'L')
	{
		//FRSN.RegionSudoku = SDP.RegionIntersectionL;
		//FRSN.ImageInvert = SDP.ImageInvertL;
		FRSN.GrayImage = SDP.GrayImageL;
		/*����ѭ��*/
		for (FRSN.it = LeftSingleRect.begin(); FRSN.it != LeftSingleRect.end(); ++FRSN.it)
		{
			/*�и��ʶ�������������α߿�*/
			GenRectangle1(&FRSN.Rectangle, (HTuple)(FRSN.it->Row1 + 5.0), (HTuple)(FRSN.it->Column1 + 5.0), (HTuple)(FRSN.it->Row2 - 5.0), (HTuple)(FRSN.it->Column2 - 5.0));
			ReduceDomain(FRSN.GrayImage, FRSN.Rectangle, &FRSN.ImageReduced);
			/*����ָ�и���������ɸ������*/
			VarThreshold(FRSN.ImageReduced, &FRSN.ImageRegion, 69, 105, 0.2, 2, "dark");
			Connection(FRSN.ImageRegion, &FRSN.ConnectedRegion);
			SelectShape(FRSN.ConnectedRegion, &FRSN.SelectedRegion1, "width", "and", 10, 100);
			SelectShape(FRSN.SelectedRegion1, &FRSN.NumRegion, "height", "and", 30, 100);

			/*SetWindowAttr("background_color", "black");
			OpenWindow(0, 0, 300, 200, 0, "", "", &WindowHandle1);
			OpenWindow(0, 0, 300, 200, 0, "", "", &WindowHandle2);
			DispRegion(FRSN.NumRegion, WindowHandle1);
			DispImage(FRSN.GrayImage, WindowHandle2);
			HDevWindowStack::Push(WindowHandle1);
			HDevWindowStack::Push(WindowHandle2);*/

			/*��ȡ����*/
			DoOcrSingleClassMlp(FRSN.NumRegion, FRSN.GrayImage, OCRStructure.OCRHandlerSudoku, 1, &FRSN.NumClass, &FRSN.Confidence);
			FRSN.it->NumClass = atoi(FRSN.NumClass.S());
			FRSN.it->Confidence = (float)FRSN.Confidence.D();
			cout << FRSN.it->NumClass << endl;
			cout << FRSN.it->Confidence << endl;
			/*���Ŷ��ж�*/
			if (FRSN.it->Confidence > CONFIDENCE)
			{
				ReadNum++;
			}
		}
	}
	/*���ݱ�ʶ��������Ӧͼ��*/
	else if (LOR == 'R')
	{
		//FRSN.RegionSudoku = SDP.RegionIntersectionR;
		//FRSN.ImageInvert = SDP.ImageInvertR;
		FRSN.GrayImage = SDP.GrayImageR;
		/*����ѭ��*/
		for (FRSN.it = RightSingleRect.begin(); FRSN.it != RightSingleRect.end(); ++FRSN.it)
		{
			/*�и��ʶ�������������α߿�*/
			GenRectangle1(&FRSN.Rectangle, (HTuple)(FRSN.it->Row1 + 5.0), (HTuple)(FRSN.it->Column1 + 5.0), (HTuple)(FRSN.it->Row2 - 5.0), (HTuple)(FRSN.it->Column2 - 5.0));
			ReduceDomain(FRSN.GrayImage, FRSN.Rectangle, &FRSN.ImageReduced);
			/*����ָ�и���������ɸ������*/
			VarThreshold(FRSN.ImageReduced, &FRSN.ImageRegion, 69, 105, 0.2, 2, "dark");
			Connection(FRSN.ImageRegion, &FRSN.ConnectedRegion);
			SelectShape(FRSN.ConnectedRegion, &FRSN.SelectedRegion1, "width", "and", 10, 100);
			SelectShape(FRSN.SelectedRegion1, &FRSN.NumRegion, "height", "and", 30, 100);

			/*SetWindowAttr("background_color", "black");
			OpenWindow(0, 0, 300, 200, 0, "", "", &WindowHandle1);
			OpenWindow(0, 0, 300, 200, 0, "", "", &WindowHandle2);
			DispRegion(FRSN.NumRegion, WindowHandle1);
			DispImage(FRSN.GrayImage, WindowHandle2);
			HDevWindowStack::Push(WindowHandle1);
			HDevWindowStack::Push(WindowHandle2);*/

			/*��ȡ����*/
			DoOcrSingleClassMlp(FRSN.NumRegion, FRSN.GrayImage, OCRStructure.OCRHandlerSudoku, 1, &FRSN.NumClass, &FRSN.Confidence);
			FRSN.it->NumClass = atoi(FRSN.NumClass.S());
			FRSN.it->Confidence = (float)FRSN.Confidence.D();
			cout << FRSN.it->NumClass << endl;
			cout << FRSN.it->Confidence << endl;
			/*���Ŷ��ж�*/
			if (FRSN.it->Confidence > CONFIDENCE)
			{
				ReadNum++;
			}
		}
	}
	else
	{
		cout << "ReadSudokuNumber,Wrong Parameter!" << endl;
		return false;
	}



	/*�ж��Ƿ�ȫ��ʶ��*/
	if (ReadNum == 9)
	{
		ReadNum = 0;
		return true;
	}
	else
	{
		ReadNum = 0;
		cout << "Read Sudoku Error!" << endl;
		return false;
	}

}



/*******************************************************************************
Function:SudokuMatched
Description:�Ź���˫Ŀƥ��Ͳ��
Calls:
      GetRectPosition()������������
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR)
Input:��
Output:��
Return:��
Others:���ҽ���LeftID_OK=1��RightID_OK=1ʱ���ô˺���
********************************************************************************/
void BigMark::SudokuMatched(void)
{
	/************************����IDƥ������ʼ***************************/
	for (FSM.SortedCounterL = LeftSingleRect.begin(); FSM.SortedCounterL != LeftSingleRect.end(); ++FSM.SortedCounterL)
	{
		for (FSM.SortedCounterR = RightSingleRect.begin(); FSM.SortedCounterR != RightSingleRect.end(); ++FSM.SortedCounterR)
		{
			/*IDƥ��*/
			if (FSM.SortedCounterL->RectID == FSM.SortedCounterR->RectID)
			{
				///*���NumClass����ͬ��ȡConfidence�ߵ�Ϊʵ��ֵ*/
				//if ((int) (FSM.SortedCounterL->NumClass) != (int) (FSM.SortedCounterR->RectID))
				//{
				//	if ((double)(FSM.SortedCounterL->Confidence) >= (double)(FSM.SortedCounterR->Confidence))
				//	{
				//		FSM.SortedCounterR->NumClass = FSM.SortedCounterL->NumClass;
				//	}
				//	else
				//	{
				//		FSM.SortedCounterL->NumClass = FSM.SortedCounterR->NumClass;
				//	}
				//}

				/*FSMTempRect.Area = FSM.SortedCounterL->Area;
				FSMTempRect.CenterColumn = FSM.SortedCounterL->CenterColumn;
				FSMTempRect.CenterRow = FSM.SortedCounterL->CenterRow;
				FSMTempRect.Row1 = FSM.SortedCounterL->Row1;
				FSMTempRect.Row2 = FSM.SortedCounterL->Row2;
				FSMTempRect.Column1 = FSM.SortedCounterL->Column1;
				FSMTempRect.Column2 = FSM.SortedCounterL->Column2;
				FSMTempRect.Width = FSM.SortedCounterL->Width;
				FSMTempRect.Height = FSM.SortedCounterL->Height;
				FSMTempRect.NumClass = FSM.SortedCounterL->NumClass;
				FSMTempRect.Confidence = FSM.SortedCounterL->Confidence;
				FSMTempRect.RectID = FSM.SortedCounterL->RectID;
				SortedLeftSingleRect.push_back(FSMTempRect);*/


				FSMTempRectL = *FSM.SortedCounterL;
				SortedLeftSingleRect.push_back(FSMTempRectL);
				FSMTempRectR = *FSM.SortedCounterR;
				SortedRightSingleRect.push_back(FSMTempRectR);

				//SortedLeftSingleRect.push_back(*FSM.SortedCounterL);
				//SortedRightSingleRect.push_back(*FSM.SortedCounterR);
			}
		}
	}
	/***********************����IDƥ���������**************************/

	/**************************3D��࿪ʼ*******************************/
	//GetRectPosition();
	/**************************3D������*******************************/
}



/*******************************************************************************
Function:GetRectPosition
Description:3D����ȡ��������
Calls:��
Called By:
      SudokuMatched(void)
Input:��
Output:��
Return:��
Others:���ɹ���WorldPositionMeasureд��1��һ��ʧ��WorldPositionMeasureд��0����������
********************************************************************************/
void BigMark::GetRectPosition(void)
{
	/*����������*/
	list<SingleRect>::iterator MatchedL;
	list<SingleRect>::iterator MatchedR;

	/*�м����*/
	HTuple XX, YY, ZZ, Dis;
	double X, Y, Z;
	double XT1, YT1, ZT1;
	double Angle_H, Angle_V, Distance;

	/*2������ͬʱ����*/
	for (MatchedL = SortedLeftSingleRect.begin(), MatchedR = SortedRightSingleRect.begin();
		MatchedL != SortedLeftSingleRect.end(), MatchedR != SortedRightSingleRect.end();
		MatchedL++, MatchedL++)
	{
		try
		{
			/*3D���,������任�������*/
			IntersectLinesOfSight(LInit.CamParamL, LInit.CamParamR, LInit.RelPose,
				MatchedL->CenterRow, MatchedL->CenterColumn, MatchedR->CenterRow, MatchedR->CenterColumn,
				&XX, &YY, &ZZ, &Dis);		
			/*������任����̨���*/
			X = (double)XX.D();
			Y = (double)YY.D();
			Z = (double)ZZ.D();
		    LArmorDetect.TransFCamToCT(X,Y,Z,&XT1,&YT1,&ZT1);
			LMathClass.CartesianCor2PolarCor(XT1, YT1, ZT1 ,&Angle_H, &Angle_V, &Distance);

			TempRectPosition.X = XT1;
			TempRectPosition.Y = YT1;
			TempRectPosition.Z = ZT1;
			TempRectPosition.Dis = Dis.D();
			TempRectPosition.AngleH = Angle_H;
			TempRectPosition.AngleV = Angle_V;
			TempRectPosition.Distance = Distance;
			TempRectPosition.ID = MatchedL->RectID;
			TempRectPosition.Num = MatchedL->NumClass;
			/*���浽����*/
			WorldRectPosition.push_back(TempRectPosition);
		}
		catch (...)
		{
			WorldPositionMeasure = 0;
			return;

		}
	}
	WorldPositionMeasure = 1;

}



/*�ָ���������*/
bool BigMark::PasswordDivition(const HObject& ImageL, const HObject& ImageR)
{
	return true;

}

/*��ȡ����*/
bool BigMark::ReadPassword(void)
{
	return true;
}

/*���봫��*/
void BigMark::PasswordTransmit(void)
{

}


/*******************************************************************************
Function:GetCellInformation
Description:����ID��ȡ������Ϣ
Calls:��
Called By:
      MatchedTransmitBigMark(void)
Input:
      ID���Ź���С����ID��
Output:
      X,Y,Z:��άֱ������
      Angle1��Angle2��DIS����ά������
Return:��
Others:����������������̨�ġ��˴���Ϊ����������δ����
********************************************************************************/
void BigMark::GetCellInformation(int ID, double *X, double *Y, double *Z, double *Angle1, double *Angle2, double *DIS)
{
	list<WorldRectPositionStructure>::iterator ii;
	for (ii = WorldRectPosition.begin(); ii != WorldRectPosition.end(); ++ii)
	{
		if (ii->ID == ID)
		{
			*X = (double)ii->X;
			*Y = (double)ii->Y;
			*Z = (double)ii->Z;
			*Angle1 = ii->AngleH;
			*Angle2 = ii->AngleV;
			*DIS = ii->Distance;
			return;
		}
	}
}



/*******************************************************************************
Function:MatchedTransmitBigMark
Description:˫Ŀ�����Ź������ݴ���
Calls:
      SingleTransmitBigMark(char LOR)
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR)
Input:ID����
Output:��
Return:��
Others:
���ҽ���LeftID_OK=1��RightID_OK=1ʱ���ô˺���
���ݴ�����ʽ��
       1.����˫Ŀ����MatchedSudoku�ؼ���
	   2.���������Ϣ������ΪX,Y,Z��Number��ID�Լ��ؼ���SudokuPrepare
	   3.�������ȷ��λBigSymbolShoot�ؼ���
���WorldPositionMeasure=0������SingleTransmitBigMark(char LOR)�����ú���������
********************************************************************************/
void BigMark::MatchedTransmitBigMark(void)
{

	/*�����һ��˫Ŀ���ʧ�ܣ����õ�Ŀ���ݴ���*/
	if (!WorldPositionMeasure)
	{
		SingleTransmitBigMark('L');
		SingleTransmitBigMark('R');
		return;
	}
	/*֪ͨ���ط���˫Ŀ����*/
	LSerialPort.SendData(0, 0, 0, 0, 0, MatchedSudoku);
	/*����*/
	for (FTBM.Read_WRPS = WorldRectPosition.begin(); FTBM.Read_WRPS != WorldRectPosition.end(); ++FTBM.Read_WRPS)
	{
		/*����ID�õ�������Ϣ*/
		/*GetCellInformation(FTBM.Read_WRPS->ID, &FTBM.TempX, &FTBM.TempY, &FTBM.TempZ, 
			&FTBM.H_Angle, &FTBM.V_Angle, &FTBM.DIS);*/
		FTBM.TempX = (double)FTBM.Read_WRPS->X;
		FTBM.TempY = (double)FTBM.Read_WRPS->Y;
		FTBM.TempZ = (double)FTBM.Read_WRPS->Z;
		FTBM.Number = FTBM.Read_WRPS->Num;
		FTBM.RectangleID = FTBM.Read_WRPS->ID;
		/*��������*/
		//LSerialPort.SendData(0, 0, 0, 0, 0, SudokuPrepare);
		LSerialPort.SendData((float)FTBM.TempX, (float)FTBM.TempY, (float)FTBM.TempZ,
			(float)FTBM.RectangleID, (float)FTBM.Number, SudokuPrepare);
	}

	LSerialPort.SendData(0, 0, 0, 0, 0, BigSymbolShoot);
}



/*******************************************************************************
Function:SingleTransmitBigMark
Description:��Ŀ�����Ź������ݴ���
Calls:��
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR)
      MatchedTransmitBigMark(void)
Input:
      LOR�����ұ�־λ��L��ʾ���������R��ʾ���������
Output:��
Return:��
Others:
���ݴ�����ʽ��
       1.���͵�Ŀ����SingleSudoku�ؼ���
       2.���������Ϣ������Ϊ0,0,0��Number��ID�Լ��ؼ���SudokuPrepare
       3.�������ȷ��λBigSymbolShoot�ؼ���
��˫Ŀ���ʧ�ܺ���MatchedTransmitBigMark�������ô˺���
��LeftID_OK��RightID_OK��ȫΪ1ʱ��ShootMark(const HObject& ImageL, const HObject& ImageR)�����е��ô˺���
********************************************************************************/
void BigMark::SingleTransmitBigMark(char LOR)
{
	list<SingleRect>::iterator ReadList;
	float ID;
	float NumClass;
	/*֪ͨ���ط��͵�Ŀ����*/
	LSerialPort.SendData(0, 0, 0, 0, 0, SingleSudoku);
	Sleep(20);
	/*���ݱ�־��ѡ��*/
	if (LOR == 'L')
	{
		for (ReadList = LeftSingleRect.begin(); ReadList != LeftSingleRect.end(); ReadList++)
		{
			ID = (float)ReadList->RectID;
			NumClass = (float)ReadList->NumClass;
			LSerialPort.SendData(0, 0, 0, ID, NumClass, SudokuPrepare);
			Sleep(500);
		}
		Sleep(500);
		LSerialPort.SendData(0, 0, 0, 0, 0, BigSymbolShoot);
	}
	else if (LOR == 'R')
	{
		for (ReadList = RightSingleRect.begin(); ReadList != RightSingleRect.end(); ReadList++)
		{
			ID = (float)ReadList->RectID;
			NumClass = (float)ReadList->NumClass;
			LSerialPort.SendData(0, 0, 0, ID, NumClass, SudokuPrepare);
			Sleep(500);
		}
		Sleep(500);
		LSerialPort.SendData(0, 0, 0, 0, 0, BigSymbolShoot);
	}
}










