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
	/*变量清0*/
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

/*主线程大符射击函数*/
void BigMark::ShootMark(const HObject& ImageL, const HObject& ImageR)
{
	bool ReadSudokuFlagL = false;
	bool ReadSudokuFlagR = false;
	/*全局变量初始化*/
	ClearList();
	
	/*OCR阅读器初始化*/
	OCR_Ready();

	/*分割密码区,如果失败报错返回*/
	if (!PasswordDivition(ImageL, ImageR))
	{
		LSerialPort.SendData(0, 0, 0, 0, 0, PasswordRegionNotFind);
		cout << "未能分割密码区" << endl;
		return;
	}
	/*密码区识别，如果失败报错返回*/
	if (!ReadPassword())
	{
		LSerialPort.SendData(0, 0, 0, 0, 0, PasswordNumNotFind);
		cout << "未能识别密码" << endl;
		return;
	}

	/*密码传输*/
	PasswordTransmit();

	/*分割九宫格区域，如果失败报错返回*/
	if (!SudokuDivition(ImageL, ImageR))
	{
		LSerialPort.SendData(0, 0, 0, 0, 0, SudokuRegionNotFind);
		cout << "未能分割九宫格区域" << endl;
		return;
	}

	/*读取九宫格数字*/
	if (LeftID_OK)
	{
		ReadSudokuFlagL = ReadSudokuNumber('L');
	}
	if (RightID_OK)
	{
		ReadSudokuFlagR = ReadSudokuNumber('R');
	}

	/*双目匹配&&3D测距*/
	if (ReadSudokuFlagL && ReadSudokuFlagR)
	{
		SudokuMatched();			
		/*大符数据传输*/
		MatchedTransmitBigMark();
		cout << "双目数据传输成功..." << endl;
	}
	/*单目直接传输ID，不传输3D坐标*/
	/*左相机单目*/
	else if (ReadSudokuFlagL && !ReadSudokuFlagR)
	{
		SingleTransmitBigMark('L');
		cout << "单目数据传输成功..." << endl;
	}
	/*右相机单目*/
	else if (!ReadSudokuFlagL && ReadSudokuFlagR)
	{
		SingleTransmitBigMark('R');
		cout << "单目数据传输成功..." << endl;
	}
	/*双目相机读取失败*/
	else
	{
		LSerialPort.SendData(0, 0, 0, 0, 0, SudokuNumNotFind);
		cout << "未能读取九宫格数字..." << endl;
	}

	/*关闭OCR阅读器*/
	OCR_Close();
}



/*******************************************************************************
Function:ClearList 
Description:清空链表，初始化全局变量
Calls:无
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR)
Input:无
Output:无
Return:无
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
Description:初始化OCR
Calls:无
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR) 
Input:无
Output:无
Return:无
Others:OCR文件需加载到工程目录下，文件名枚举在函数内部写出
********************************************************************************/
void BigMark::OCR_Ready(void)
{
	/*所有可选的文件名*/
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
Description:关闭OCR文件
Calls:无
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR) 
Input:无
Output:无
Return:无
Others:打开OCR后必须需要调用此函数关闭文件
********************************************************************************/
void BigMark::OCR_Close(void)
{
	ClearOcrClassMlp(OCRStructure.OCRHandlerSudoku);
}



/*******************************************************************************
Function:SudokuDivition 
Description:分割九宫格，获取九宫格除数字外的数据，写入结构体LeftSingleRect，RightSingleRect坐标信息
Calls:
      GetRelativePosition(void)
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR) 
Input:
      ImageL：左相机图像
      ImageR：左相机图像
Output:无
Return:分割成功返回TRUE,否则返回FALSE
Others:当成功分割出九宫格位置（LeftCounter == 9 || RightCounter == 9）才能调用GetRelativePosition()函数，
       否则直接返回报错

	   重要更新：
	   2016/11/24 提前对图形进行缩减
	   2016/12/8 简化处理步骤 
********************************************************************************/
bool BigMark::SudokuDivition(const HObject& ImageL, const HObject& ImageR)
{
	/*计数器临时变量*/
	HTuple TempNumL, TempNumR;
	
	//LeftCounter = 0;
	//RightCounter = 0;
	/*转化为灰度图像*/
	Rgb1ToGray(ImageL, &SDP.GrayImageL);
	Rgb1ToGray(ImageR, &SDP.GrayImageR);

	/*圈定ROI区域*/
	GenRectangle1(&SDP.ImageROI_L, 164, 523, 770, 1517);
	GenRectangle1(&SDP.ImageROI_R, 57, 391, 613, 1302);

	/*图像域缩减*/
	ReduceDomain(SDP.GrayImageL, SDP.ImageROI_L, &SDP.ImageReducedL);
	ReduceDomain(SDP.GrayImageR, SDP.ImageROI_R, &SDP.ImageReducedR);

	/*灰度图转换*/
	//GrayRangeRect(SDP.ImageReducedL, &SDP.ImageResultL, 3, 3);
	//GrayRangeRect(SDP.ImageReducedR, &SDP.ImageResultR, 3, 3);
	
	/*灰度值反转*/
	//InvertImage(SDP.ImageResultL, &SDP.ImageInvertL);
	//InvertImage(SDP.ImageResultR, &SDP.ImageInvertR);

	/*阈值化分割*/
	//Threshold(SDP.ImageResultL, &SDP.RegionL, 50, 255);
	//Threshold(SDP.ImageResultR, &SDP.RegionR, 50, 255);

	/*均值滤波*/
	MeanImage(SDP.ImageReducedL, &SDP.ImageMeanL, 40, 40);
	MeanImage(SDP.ImageReducedR, &SDP.ImageMeanR, 40, 40);

	/*高通滤波*/
	DynThreshold(SDP.ImageReducedL, SDP.ImageMeanL, &SDP.RegionDynThreshL, 10, "light");
	DynThreshold(SDP.ImageReducedR, SDP.ImageMeanR, &SDP.RegionDynThreshR, 10, "light");


	/*区域填充*/
	FillUp(SDP.RegionDynThreshL, &SDP.RegionFillUpL);
	FillUp(SDP.RegionDynThreshR, &SDP.RegionFillUpR);

	/*区域连通*/
	Connection(SDP.RegionFillUpL, &SDP.ConnectedRegionsL);
	Connection(SDP.RegionFillUpR, &SDP.ConnectedRegionsR);

	/*区域筛选*/
	SelectShape(SDP.ConnectedRegionsL, &SDP.SelectedRegionsL1, "area", "and", 10000, 35000);
	SelectShape(SDP.ConnectedRegionsR, &SDP.SelectedRegionsR1, "area", "and", 10000, 35000);

	SelectShape(SDP.SelectedRegionsL1, &SDP.SelectedRegionsL2, "rectangularity", "and", 0.8, 1);
	SelectShape(SDP.SelectedRegionsR1, &SDP.SelectedRegionsR2, "rectangularity", "and", 0.8, 1);

	/*取交集还原九宫格内容*/
	//Intersection(SDP.RegionL, SDP.SelectedRegionsL2, &SDP.RegionIntersectionL);
	//Intersection(SDP.RegionR, SDP.SelectedRegionsR2, &SDP.RegionIntersectionR);

	/*筛选后区域个数*/
	CountObj(SDP.SelectedRegionsL2, &TempNumL);
	CountObj(SDP.SelectedRegionsR2, &TempNumR);

	cout << TempNumL.I() << endl;
	cout << TempNumR.I() << endl;
	/*左相机区域循环*/
	for (int i = 0; i != TempNumL.I(); ++i)
	{
		try
		{
			/*选中目标，获取最小包围矩形长度*/
			SelectObj(SDP.SelectedRegionsL2, &SDP.ObjectSelected, i + 1);
			SmallestRectangle2(SDP.ObjectSelected, &SDP.Row, &SDP.Column,
				&SDP.Phi, &SDP.Length1, &SDP.Length2);
			/*计算面积，中心点和边缘点坐标*/
			AreaCenter(SDP.ObjectSelected, &SDP.Area, &SDP.CenterRow, &SDP.CenterColumn);
			SmallestRectangle1(SDP.ObjectSelected, &SDP.Row1, &SDP.Column1, &SDP.Row2, &SDP.Column2);
		}
		catch (...)
		{
			continue;
		}

		/*筛选要求*/
		if (SDP.Length1 >= RECT_MIN_LENGTH1
			&&SDP.Length1 <= RECT_MAX_LENGTH1
			&&SDP.Length2 >= RECT_MIN_LENGTH2
			&&SDP.Length2 <= RECT_MAX_LENGTH2)
		{		
			/*保存数据到链表*/
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
	
	/*右相机区域循环*/
	for (int j = 0; j!= TempNumR.I(); ++j)
	{
		try
		{
			/*选中目标，获取最小包围矩形长度*/
			SelectObj(SDP.SelectedRegionsR2, &SDP.ObjectSelected, j + 1);
			SmallestRectangle2(SDP.ObjectSelected, &SDP.Row, &SDP.Column,
				&SDP.Phi, &SDP.Length1, &SDP.Length2);
			/*计算面积，中心点和边缘点坐标*/
			AreaCenter(SDP.ObjectSelected, &SDP.Area, &SDP.CenterRow, &SDP.CenterColumn);
			SmallestRectangle1(SDP.ObjectSelected, &SDP.Row1, &SDP.Column1, &SDP.Row2, &SDP.Column2);
		}
		catch (...)
		{
			continue;
		}
		/*筛选要求*/
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

	/*找到足够的矩形框*/
	/*note:放宽条件，当两个摄像头有一个识别出9个矩形框时即可进入相关函数，保证
	至少有一个能正常工作*/
	if (LeftCounter == 9 || RightCounter == 9)
	{
		/*获取相对位置和ID*/
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
Description:获取九宫格相对位置和ID，写入到结构体SingleRect成员变量RectID
Calls:无
Called By:
      SudokuDivition(const HObject& ImageL, const HObject& ImageR)
Input:无
Output:无
Return:无
Others:当左相机图像ID写入成功9次后LeftID_OK写1，右相机图像ID写入成功9次后RightID_OK写1
********************************************************************************/
void BigMark::GetRelativePosition(void)
{
    
	/*************ID命名*************/
	/*1--------------2-------------3*/


	/*4--------------5-------------6*/


	/*7--------------8-------------9*/

	/*变量清空*/
	FGRP.GetPositionL = 0;
	FGRP.GetPositionR = 0;
	FGRP.MidColumn = 0;
	FGRP.MidRow = 0;

	/*控制数组定义*/
	int LeftID_Flag[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0};
	int RightID_Flag[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0};
	/*****************************左相机处理开始***************************/
	if (LeftCounter == 9)
	{
		for (FGRP.it1 = LeftSingleRect.begin(); FGRP.it1 != LeftSingleRect.end(); ++FGRP.it1)
		{
			FGRP.MidColumn += (double)FGRP.it1->CenterColumn;
			FGRP.MidRow += (double)FGRP.it1->CenterRow;
		}
		/*获取中心点大致坐标*/
		FGRP.MidColumn /= 9.0;
		FGRP.MidRow /= 9.0;
		/*************************相对位置判断，写入ID****************************/
		/*链表遍历开始*/
		for (FGRP.it2 = LeftSingleRect.begin(); FGRP.it2 != LeftSingleRect.end(); ++FGRP.it2)
		{
			/*第三行位置确定开始*/
			if ((double)FGRP.it2->CenterRow - FGRP.MidRow >= RECT_ROW_DIS)
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && LeftID_Flag[9] == 0)//右下角
				{				
					FGRP.it2->RectID = 9;//写入ID
					FGRP.GetPositionL++;//计数器累加
					LeftID_Flag[9] = 1;//控制数组相应ID位置位
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && LeftID_Flag[7] == 0)//左下角
				{					
					FGRP.it2->RectID = 7;
					FGRP.GetPositionL++;
					LeftID_Flag[7] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn)<RECT_COLUMN_DIS && LeftID_Flag[8] == 0)//正下方
				{
					FGRP.it2->RectID = 8;
					FGRP.GetPositionL++;
					LeftID_Flag[8] = 1;
				}
				else//ID已被占用，写入失败
				{
					LeftID_OK = 0;
					//return;
				}
			}
			/*第三行位置确定结束*/

			/*第一行位置确定开始*/
			else if (FGRP.MidRow - (double)FGRP.it2->CenterRow >= RECT_ROW_DIS)
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && LeftID_Flag[3]==0)//右上角
				{
					FGRP.it2->RectID = 3;
					FGRP.GetPositionL++;
					LeftID_Flag[3] = 1;
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && LeftID_Flag[1] == 0)//左上角
				{
					FGRP.it2->RectID = 1;
					FGRP.GetPositionL++;
					LeftID_Flag[1] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn)<RECT_COLUMN_DIS && LeftID_Flag[2] == 0)//正上方
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
			/*第一行位置确定结束*/

			/*第二行位置确定开始*/
			else
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && LeftID_Flag[6] == 0)//正右方
				{
					FGRP.it2->RectID = 6;
					FGRP.GetPositionL++;
					LeftID_Flag[6] = 1;
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && LeftID_Flag[4] == 0)//正左方
				{
					FGRP.it2->RectID = 4;
					FGRP.GetPositionL++;
					LeftID_Flag[4] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn < RECT_COLUMN_DIS) && LeftID_Flag[5] == 0)//中心
				{
					FGRP.it2->RectID = 5;//中心
					FGRP.GetPositionL++;
					LeftID_Flag[5] = 1;
				}
				else
				{
					LeftID_OK = 0;
					//return;
				}
			}
			/*第二行位置确定结束*/
        
		/*链表遍历结束*/
		}

		/*ID全部写入成功*/
		if (FGRP.GetPositionL == 9)
		{
			LeftID_OK = 1;//标志位置1
		}

		else
		{
			LeftID_OK = 0;
		}
	/*左相机处理结束*/
	}

	/**************************右相机处理开始***************************/
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
		/*************************相对位置判断，写入ID****************************/
		/*链表遍历开始*/
		for (FGRP.it2 = RightSingleRect.begin(); FGRP.it2 != RightSingleRect.end(); ++FGRP.it2)
		{
			/*第三行位置确定开始*/
			if ((double)FGRP.it2->CenterRow - FGRP.MidRow >= RECT_ROW_DIS)
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && RightID_Flag[9] == 0)//右下角
				{
					FGRP.it2->RectID = 9;
					FGRP.GetPositionR++;
					RightID_Flag[9] = 1;
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && RightID_Flag[7] == 0)//左下角
				{
					FGRP.it2->RectID = 7;
					FGRP.GetPositionR++;
					RightID_Flag[7] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn) < RECT_COLUMN_DIS && RightID_Flag[8] == 0)//正下方
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
			/*第三行位置确定结束*/

			/*第一行位置确定开始*/
			else if (FGRP.MidRow - (double)FGRP.it2->CenterRow >= RECT_ROW_DIS)
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && RightID_Flag[3] == 0)//右上角
				{
					FGRP.it2->RectID = 3;
					FGRP.GetPositionR++;
					RightID_Flag[3] = 1;
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && RightID_Flag[1] == 0)//左上角
				{
					FGRP.it2->RectID = 1;
					FGRP.GetPositionR++;
					RightID_Flag[1] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn) < RECT_COLUMN_DIS && RightID_Flag[2] == 0)//正上方
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
			/*第一行位置确定结束*/

			/*第二行位置确定开始*/
			else
			{
				if ((double)FGRP.it2->CenterColumn - FGRP.MidColumn >= RECT_COLUMN_DIS && RightID_Flag[6] == 0)//正右方
				{
					FGRP.it2->RectID = 6;
					FGRP.GetPositionR++;
					RightID_Flag[6] = 1;
				}
				else if (FGRP.MidColumn - (double)FGRP.it2->CenterColumn >= RECT_COLUMN_DIS && RightID_Flag[4] == 0)//正左方
				{
					FGRP.it2->RectID = 4;
					FGRP.GetPositionR++;
					RightID_Flag[4] = 1;
				}
				else if (abs((double)FGRP.it2->CenterColumn - FGRP.MidColumn) < RECT_COLUMN_DIS && RightID_Flag[5] == 0)//中心
				{
					FGRP.it2->RectID = 5;//中心
					FGRP.GetPositionR++;
					RightID_Flag[5] = 1;
				}
				else
				{
					RightID_OK = 0;
				}
			}
			/*第二行位置确定结束*/

		/*链表遍历结束*/
		}

		if (FGRP.GetPositionR == 9)
		{
			RightID_OK = 1;//标志位置1
		}
		else
		{
			RightID_OK = 0;
		}
	/*右相机处理结束*/
	}
}



/*******************************************************************************
Function:ReadSudokuNumber
Description:读取九宫格数字，写入到结构体SingleRect成员变量NumClass
Calls:无
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR) 
Input:LOR：左右标志位，L表示左相机图像，R表示右相机图像
Output:无
Return:无
Others:根据标识符加载图像进行识别，CONFIDENCE作为可信度判据
       
	   重要更新：
	   2016/11/25 重写数字提取代码，避免字体自身粗细带来潜在的误识别
	   2016/12/8 换用VarThreshold算子，快速切割数字
********************************************************************************/
bool BigMark::ReadSudokuNumber(char LOR)
{
	HTuple  WindowHandle1, WindowHandle2;
	ReadNum = 0;
	/*根据标识符加载相应图像*/
	if (LOR == 'L')
	{
		//FRSN.RegionSudoku = SDP.RegionIntersectionL;
		//FRSN.ImageInvert = SDP.ImageInvertL;
		FRSN.GrayImage = SDP.GrayImageL;
		/*链表循环*/
		for (FRSN.it = LeftSingleRect.begin(); FRSN.it != LeftSingleRect.end(); ++FRSN.it)
		{
			/*切割待识别数字区域，屏蔽边框*/
			GenRectangle1(&FRSN.Rectangle, (HTuple)(FRSN.it->Row1 + 5.0), (HTuple)(FRSN.it->Column1 + 5.0), (HTuple)(FRSN.it->Row2 - 5.0), (HTuple)(FRSN.it->Column2 - 5.0));
			ReduceDomain(FRSN.GrayImage, FRSN.Rectangle, &FRSN.ImageReduced);
			/*区域分割，切割数字区域并筛除噪声*/
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

			/*读取数字*/
			DoOcrSingleClassMlp(FRSN.NumRegion, FRSN.GrayImage, OCRStructure.OCRHandlerSudoku, 1, &FRSN.NumClass, &FRSN.Confidence);
			FRSN.it->NumClass = atoi(FRSN.NumClass.S());
			FRSN.it->Confidence = (float)FRSN.Confidence.D();
			cout << FRSN.it->NumClass << endl;
			cout << FRSN.it->Confidence << endl;
			/*可信度判断*/
			if (FRSN.it->Confidence > CONFIDENCE)
			{
				ReadNum++;
			}
		}
	}
	/*根据标识符加载相应图像*/
	else if (LOR == 'R')
	{
		//FRSN.RegionSudoku = SDP.RegionIntersectionR;
		//FRSN.ImageInvert = SDP.ImageInvertR;
		FRSN.GrayImage = SDP.GrayImageR;
		/*链表循环*/
		for (FRSN.it = RightSingleRect.begin(); FRSN.it != RightSingleRect.end(); ++FRSN.it)
		{
			/*切割待识别数字区域，屏蔽边框*/
			GenRectangle1(&FRSN.Rectangle, (HTuple)(FRSN.it->Row1 + 5.0), (HTuple)(FRSN.it->Column1 + 5.0), (HTuple)(FRSN.it->Row2 - 5.0), (HTuple)(FRSN.it->Column2 - 5.0));
			ReduceDomain(FRSN.GrayImage, FRSN.Rectangle, &FRSN.ImageReduced);
			/*区域分割，切割数字区域并筛除噪声*/
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

			/*读取数字*/
			DoOcrSingleClassMlp(FRSN.NumRegion, FRSN.GrayImage, OCRStructure.OCRHandlerSudoku, 1, &FRSN.NumClass, &FRSN.Confidence);
			FRSN.it->NumClass = atoi(FRSN.NumClass.S());
			FRSN.it->Confidence = (float)FRSN.Confidence.D();
			cout << FRSN.it->NumClass << endl;
			cout << FRSN.it->Confidence << endl;
			/*可信度判断*/
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



	/*判断是否全部识别*/
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
Description:九宫格双目匹配和测距
Calls:
      GetRectPosition()测量世界坐标
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR)
Input:无
Output:无
Return:无
Others:当且仅当LeftID_OK=1，RightID_OK=1时调用此函数
********************************************************************************/
void BigMark::SudokuMatched(void)
{
	/************************链表ID匹配排序开始***************************/
	for (FSM.SortedCounterL = LeftSingleRect.begin(); FSM.SortedCounterL != LeftSingleRect.end(); ++FSM.SortedCounterL)
	{
		for (FSM.SortedCounterR = RightSingleRect.begin(); FSM.SortedCounterR != RightSingleRect.end(); ++FSM.SortedCounterR)
		{
			/*ID匹配*/
			if (FSM.SortedCounterL->RectID == FSM.SortedCounterR->RectID)
			{
				///*如果NumClass不相同，取Confidence高的为实际值*/
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
	/***********************链表ID匹配排序结束**************************/

	/**************************3D测距开始*******************************/
	//GetRectPosition();
	/**************************3D测距结束*******************************/
}



/*******************************************************************************
Function:GetRectPosition
Description:3D测距获取世界坐标
Calls:无
Called By:
      SudokuMatched(void)
Input:无
Output:无
Return:无
Others:测距成功后WorldPositionMeasure写入1，一旦失败WorldPositionMeasure写入0，立即返回
********************************************************************************/
void BigMark::GetRectPosition(void)
{
	/*迭代器声明*/
	list<SingleRect>::iterator MatchedL;
	list<SingleRect>::iterator MatchedR;

	/*中间变量*/
	HTuple XX, YY, ZZ, Dis;
	double X, Y, Z;
	double XT1, YT1, ZT1;
	double Angle_H, Angle_V, Distance;

	/*2个链表同时遍历*/
	for (MatchedL = SortedLeftSingleRect.begin(), MatchedR = SortedRightSingleRect.begin();
		MatchedL != SortedLeftSingleRect.end(), MatchedR != SortedRightSingleRect.end();
		MatchedL++, MatchedL++)
	{
		try
		{
			/*3D测距,右相机变换到左相机*/
			IntersectLinesOfSight(LInit.CamParamL, LInit.CamParamR, LInit.RelPose,
				MatchedL->CenterRow, MatchedL->CenterColumn, MatchedR->CenterRow, MatchedR->CenterColumn,
				&XX, &YY, &ZZ, &Dis);		
			/*左相机变换到云台相机*/
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
			/*保存到链表*/
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



/*分割密码区域*/
bool BigMark::PasswordDivition(const HObject& ImageL, const HObject& ImageR)
{
	return true;

}

/*读取密码*/
bool BigMark::ReadPassword(void)
{
	return true;
}

/*密码传输*/
void BigMark::PasswordTransmit(void)
{

}


/*******************************************************************************
Function:GetCellInformation
Description:根据ID获取坐标信息
Calls:无
Called By:
      MatchedTransmitBigMark(void)
Input:
      ID：九宫格小矩形ID号
Output:
      X,Y,Z:三维直角坐标
      Angle1，Angle2，DIS：三维极坐标
Return:无
Others:所有坐标均是相对云台的。此代码为保留代码暂未调用
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
Description:双目测量九宫格数据传输
Calls:
      SingleTransmitBigMark(char LOR)
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR)
Input:ID：无
Output:无
Return:无
Others:
当且仅当LeftID_OK=1，RightID_OK=1时调用此函数
数据传输样式：
       1.传送双目数据MatchedSudoku关键字
	   2.传送相关信息，依次为X,Y,Z，Number，ID以及关键字SudokuPrepare
	   3.传送射击确认位BigSymbolShoot关键字
如果WorldPositionMeasure=0，调用SingleTransmitBigMark(char LOR)，调用后立即返回
********************************************************************************/
void BigMark::MatchedTransmitBigMark(void)
{

	/*如果上一步双目测距失败，采用单目数据传输*/
	if (!WorldPositionMeasure)
	{
		SingleTransmitBigMark('L');
		SingleTransmitBigMark('R');
		return;
	}
	/*通知主控发送双目数据*/
	LSerialPort.SendData(0, 0, 0, 0, 0, MatchedSudoku);
	/*遍历*/
	for (FTBM.Read_WRPS = WorldRectPosition.begin(); FTBM.Read_WRPS != WorldRectPosition.end(); ++FTBM.Read_WRPS)
	{
		/*根据ID得到坐标信息*/
		/*GetCellInformation(FTBM.Read_WRPS->ID, &FTBM.TempX, &FTBM.TempY, &FTBM.TempZ, 
			&FTBM.H_Angle, &FTBM.V_Angle, &FTBM.DIS);*/
		FTBM.TempX = (double)FTBM.Read_WRPS->X;
		FTBM.TempY = (double)FTBM.Read_WRPS->Y;
		FTBM.TempZ = (double)FTBM.Read_WRPS->Z;
		FTBM.Number = FTBM.Read_WRPS->Num;
		FTBM.RectangleID = FTBM.Read_WRPS->ID;
		/*发送数据*/
		//LSerialPort.SendData(0, 0, 0, 0, 0, SudokuPrepare);
		LSerialPort.SendData((float)FTBM.TempX, (float)FTBM.TempY, (float)FTBM.TempZ,
			(float)FTBM.RectangleID, (float)FTBM.Number, SudokuPrepare);
	}

	LSerialPort.SendData(0, 0, 0, 0, 0, BigSymbolShoot);
}



/*******************************************************************************
Function:SingleTransmitBigMark
Description:单目测量九宫格数据传输
Calls:无
Called By:
      ShootMark(const HObject& ImageL, const HObject& ImageR)
      MatchedTransmitBigMark(void)
Input:
      LOR：左右标志位，L表示左相机链表，R表示右相机链表
Output:无
Return:无
Others:
数据传输样式：
       1.传送单目数据SingleSudoku关键字
       2.传送相关信息，依次为0,0,0，Number，ID以及关键字SudokuPrepare
       3.传送射击确认位BigSymbolShoot关键字
当双目测距失败后在MatchedTransmitBigMark（）调用此函数
当LeftID_OK，RightID_OK不全为1时在ShootMark(const HObject& ImageL, const HObject& ImageR)函数中调用此函数
********************************************************************************/
void BigMark::SingleTransmitBigMark(char LOR)
{
	list<SingleRect>::iterator ReadList;
	float ID;
	float NumClass;
	/*通知主控发送单目数据*/
	LSerialPort.SendData(0, 0, 0, 0, 0, SingleSudoku);
	Sleep(20);
	/*根据标志符选择*/
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










