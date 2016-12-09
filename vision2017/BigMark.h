/************************************************************************************
*	大符类
*
*	大符检测及测量
*
*************************************************************************************/


#ifndef MARK_H
#define MARK_H

#include "stdafx.h"
/*密码变更宏定义,1:密码已修改，0：密码未修改*/
#define UNCHANGED_PASSWORD 0
#define CHANGED_PASSWORD 1

/*区域分割坐标定义,左右图像有区分*/
#define SUDOKU_ROW1_L 88
#define SUDOKU_COLUMN1_L 472
#define SUDOKU_ROW2_L 420
#define SUDOKU_COLUMN2_L 1047

#define PASSWORD_ROW1_L 23
#define PASSWORD_COLUMN1_L 503
#define PASSWORD_ROW2_L 108
#define PASSWORD_COLUMN2_L 993

#define SUDOKU_ROW1_R 88
#define SUDOKU_COLUMN1_R 472
#define SUDOKU_ROW2_R 420
#define SUDOKU_COLUMN2_R 1047

#define PASSWORD_ROW1_R 23
#define PASSWORD_COLUMN1_R 503
#define PASSWORD_ROW2_R 108
#define PASSWORD_COLUMN2_R 993

/*灰度图阈值化参数*/
#define SUDOKU_MIN_THRESHOLD 50
#define SUDOKU_MAX_THRESHOLD 255

/*均值滤波阈值*/
#define SUDOKU_IMAGE_MEAN 40

/*高通滤波阈值*/
#define SUDOKU_DYN_THRESHOLD_OFFSET 10

/*九宫格单体区域筛选*/
/*LENGTH1表示是长轴，LENGTH2表示短轴*/
#define RECT_MIN_REGION_AREA 3000
#define RECT_MAX_REGION_AREA 8000
#define RECT_MIN_LENGTH1 85
#define RECT_MAX_LENGTH1 115
#define RECT_MIN_LENGTH2 40
#define RECT_MAX_LENGTH2 70

/*九宫格单体相对位置确定宏定义*/
#define RECT_ROW_DIS 50
#define RECT_COLUMN_DIS 100

/*识别可信度*/
#define CONFIDENCE 0.6

class BigMark
{
public:
	BigMark(void);
	~BigMark(void);

	/* 计时 */
	LARGE_INTEGER FreqMark;
	LARGE_INTEGER BeginTimeMark;
	LARGE_INTEGER EndTimeMark;
	/** 两帧之间时间差 */
	double time;
	HTuple ii;

	/*OCR阅读器结构体*/
	typedef struct OCRStructure
	{
		HTuple FileNameSudoku;
		HTuple OCRHandlerSudoku;
		HTuple FileNamePassword;
		HTuple OCRHandlerPassword;
		bool OCROpen;
	}OCRStructure;

	OCRStructure OCRStructure;


private:

	/*九宫格单体结构体*/
	typedef struct SingleRect
	{
		int RectID;//小矩形ID号，非数字识别结果！！！
		/*九宫格中心点坐标*/
		float CenterRow;
		float CenterColumn;

		/*九宫格顶点坐标*/
		float Row1;
		float Column1;
		float Row2;
		float Column2;

		/*九宫格尺寸*/
		float Width;
		float Height;

		/*九宫格面积*/
		float Area;

		/*识别的结果*/
		int NumClass;

		/*可信度*/
		float Confidence;

	}SingleRect;
	
	/*九宫格分割函数结构体*/
	SingleRect TempRect;
	
	/*双目匹配函数结构体*/
	SingleRect FSMTempRectL;
	SingleRect FSMTempRectR;

	/*左右相机九宫格链表*/
	list<SingleRect> LeftSingleRect;
	list<SingleRect> RightSingleRect;

	/*左右相机链表按ID排序*/
	list<SingleRect> SortedLeftSingleRect;
	list<SingleRect> SortedRightSingleRect;


	/*3D坐标结构体*/
	typedef struct WorldRectPositionStructure
	{
		/*3D直角坐标*/
		double X;
		double Y;
		double Z;
		double Dis;

		/*3D极坐标*/
		double AngleH;
		double AngleV;
		double Distance;

		/*数字*/
		int Num;
		int ID;
	}WorldRectPositionStructure;

	/*双目测距*/
	WorldRectPositionStructure TempRectPosition;
	
	/*双目测距链表*/
	list<WorldRectPositionStructure> WorldRectPosition;

	/*匹配链表*/
	//list<WorldRectPosition>

	/* 全局参数结构体 */
	typedef struct OverallParam
	{
		HObject Chambers;        //单元格区域参数
		HTuple Wide, High;

		int ReadyCount;     //准备阶段计数
		bool StartShoot;     //准备结束，开始打
		bool NewStart;


		int Target1;      //可能目标一
		int Target2;      //可能目标二

		int PresentID;      //当前帧目标ID
		int LastID;       //上一帧ID

		float AngleX, AngleY, Distance;   //过渡变量
		double X, Y, Z;    //过渡变量
		double XT, YT, ZT;     //过渡变量

	}OverallParam;

	OverallParam OAP;


	/*密码结构体*/
	typedef struct FunReadPassword
	{
		int LastPassword[5];
		int PreaestPassword[5];

		/*密码更改标志位.1：密码修改 0：密码未修改*/
		bool ChangePassword;
	}FunReadPassword;

	FunReadPassword Password;

	/*分割九宫格函数结构体*/
	typedef struct FunSudokuDivitionParam
	{
		/*灰度图*/
		HObject GrayImageL, GrayImageR;
		/*灰度图转化*/
		HObject ImageResultL, ImageResultR;
		/*ROI区域*/
		HObject ImageROI_L, ImageROI_R;
		/*区域减小后的图像*/
		HObject ImageReducedL, ImageReducedR;
		/*灰度图反转*/
		HObject ImageInvertL, ImageInvertR;
		/*阈值化分割图像*/
		HObject RegionL, RegionR;
		/*均值滤波*/
		HObject ImageMeanL, ImageMeanR;
		/*高通滤波*/
		HObject RegionDynThreshL, RegionDynThreshR;
		/*区域分割*/
		HObject RegionClippedL, RegionClippedR;
		/*填充*/
		HObject RegionFillUpL, RegionFillUpR;
		/*区域连通*/
		HObject ConnectedRegionsL, ConnectedRegionsR;
		/*区域筛选*/
		HObject SelectedRegionsL1, SelectedRegionsR1;
		HObject SelectedRegionsL2, SelectedRegionsR2;
		HObject SelectedRegionsL3, SelectedRegionsR3;
		HObject SelectedRegionsL4, SelectedRegionsR4;
		/*交集*/
		HObject RegionIntersectionL, RegionIntersectionR;
		/*区域排序*/
		HObject SortedRegionL, SortedRegionR;
		/*区域选中临时变量*/
		HObject ObjectSelected;
		HTuple Row, Column, Phi, Length1, Length2;
		HTuple Area,CenterRow, CenterColumn, Row1, Column1, Row2, Column2;

	}FunSudokuDivitionParam;

	FunSudokuDivitionParam SDP;

	

	/*读取相对位置获得ID结构体*/
	typedef struct FunGetRelativePosition
	{
		/*遍历链表变量*/
		list<SingleRect>::iterator it1;
		list<SingleRect>::iterator it2;
		/*求中心点坐标*/
		double MidRow, MidColumn;
		/*ID写入计数器*/
		int GetPositionL;
		int GetPositionR;

	}FunGetRelativePosition;

	FunGetRelativePosition FGRP;

	/*读取九宫格函数数字结构体*/
	typedef struct FunReadSudokuNumber
	{
		/*待处理图像*/
		HObject RegionSudoku;
		HObject GrayImage;
		HObject ImageInvert;
		
		HObject Rectangle;
		HObject ImageReduced;
		HObject ImageRegion;
		HTuple NumClass;
		HTuple Confidence;


		HObject ClippedRegion;
		HObject ConnectedRegion;
		HObject SelectedRegion1;
		HObject SelectedRegion2;
		HObject NumRegion;

		/*遍历链表变量*/
		list<SingleRect>::iterator it;

	}FunReadSudokuNumber;

	FunReadSudokuNumber FRSN;

	/*九宫格双目匹配函数结构体*/
	typedef struct FunSudokuMatched
	{
		/*遍历链表变量*/
		list<SingleRect>::iterator SortedCounterL;
		list<SingleRect>::iterator SortedCounterR;
	}FunSudokuMatched;

	FunSudokuMatched FSM;

	/*数据传输结构体*/
	typedef struct FunTransmitBigMark
	{
		/*迭代器变量*/
		list<WorldRectPositionStructure>::iterator Read_WRPS;
		/*中间变量*/
		double TempX, TempY, TempZ;
		double H_Angle;
		double V_Angle;
		double DIS;
		int Number;
		int RectangleID;

	}FunTransmitBigMark;

	FunTransmitBigMark FTBM;

	/*区域计数器*/
	int LeftCounter;
	int RightCounter;
	/*识别计数器*/
	int ReadNum;
	/*左右相机ID写入成功标志位*/
	bool LeftID_OK;
	bool RightID_OK;
	/*双目测距成功标志位*/
	bool WorldPositionMeasure;
	
public:

	/*射击大符*/
	void ShootMark(const HObject& ImageL, const HObject& ImageR);

private:
	/*OCR阅读器初始化*/
	void OCR_Ready(void);

	/*关闭OCR阅读器*/
	void OCR_Close(void);

	/*分割九宫格区域*/
	bool SudokuDivition(const HObject& ImageL, const HObject& ImageR);

	/*分割密码区域*/
	bool PasswordDivition(const HObject& ImageL, const HObject& ImageR);

	/*测量9个小格的世界坐标*/
	void GetRectPosition(void);

	/*计算九宫格相对位置*/
	void GetRelativePosition(void);

	/*读取九宫格数字*/
	bool ReadSudokuNumber(char LOR);

	/*读取密码*/
	bool ReadPassword(void);

	/*九宫格双目匹配*/
	void SudokuMatched(void);

	/* 根据给定的ID返回该ID单元格的信息 */
	void GetCellInformation(int ID,double *X,double *Y,double *Z,
		double *Angle1,double *Angle2,double *DIS);

	/*链表清空*/
	void ClearList(void);

	/*密码区传输*/
	void PasswordTransmit(void);

	/*双目九宫格数据传输*/
	void MatchedTransmitBigMark(void);

	/*单目九宫格数据传输*/
	void SingleTransmitBigMark(char LOR);
};



#endif