/************************************************************************************
*	�����
*
*	�����⼰����
*
*************************************************************************************/


#ifndef MARK_H
#define MARK_H

#include "stdafx.h"
/*�������궨��,1:�������޸ģ�0������δ�޸�*/
#define UNCHANGED_PASSWORD 0
#define CHANGED_PASSWORD 1

/*����ָ����궨��,����ͼ��������*/
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

/*�Ҷ�ͼ��ֵ������*/
#define SUDOKU_MIN_THRESHOLD 50
#define SUDOKU_MAX_THRESHOLD 255

/*��ֵ�˲���ֵ*/
#define SUDOKU_IMAGE_MEAN 40

/*��ͨ�˲���ֵ*/
#define SUDOKU_DYN_THRESHOLD_OFFSET 10

/*�Ź���������ɸѡ*/
/*LENGTH1��ʾ�ǳ��ᣬLENGTH2��ʾ����*/
#define RECT_MIN_REGION_AREA 3000
#define RECT_MAX_REGION_AREA 8000
#define RECT_MIN_LENGTH1 85
#define RECT_MAX_LENGTH1 115
#define RECT_MIN_LENGTH2 40
#define RECT_MAX_LENGTH2 70

/*�Ź��������λ��ȷ���궨��*/
#define RECT_ROW_DIS 50
#define RECT_COLUMN_DIS 100

/*ʶ����Ŷ�*/
#define CONFIDENCE 0.6

class BigMark
{
public:
	BigMark(void);
	~BigMark(void);

	/* ��ʱ */
	LARGE_INTEGER FreqMark;
	LARGE_INTEGER BeginTimeMark;
	LARGE_INTEGER EndTimeMark;
	/** ��֮֡��ʱ��� */
	double time;
	HTuple ii;

	/*OCR�Ķ����ṹ��*/
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

	/*�Ź�����ṹ��*/
	typedef struct SingleRect
	{
		int RectID;//С����ID�ţ�������ʶ����������
		/*�Ź������ĵ�����*/
		float CenterRow;
		float CenterColumn;

		/*�Ź��񶥵�����*/
		float Row1;
		float Column1;
		float Row2;
		float Column2;

		/*�Ź���ߴ�*/
		float Width;
		float Height;

		/*�Ź������*/
		float Area;

		/*ʶ��Ľ��*/
		int NumClass;

		/*���Ŷ�*/
		float Confidence;

	}SingleRect;
	
	/*�Ź���ָ���ṹ��*/
	SingleRect TempRect;
	
	/*˫Ŀƥ�亯���ṹ��*/
	SingleRect FSMTempRectL;
	SingleRect FSMTempRectR;

	/*��������Ź�������*/
	list<SingleRect> LeftSingleRect;
	list<SingleRect> RightSingleRect;

	/*�����������ID����*/
	list<SingleRect> SortedLeftSingleRect;
	list<SingleRect> SortedRightSingleRect;


	/*3D����ṹ��*/
	typedef struct WorldRectPositionStructure
	{
		/*3Dֱ������*/
		double X;
		double Y;
		double Z;
		double Dis;

		/*3D������*/
		double AngleH;
		double AngleV;
		double Distance;

		/*����*/
		int Num;
		int ID;
	}WorldRectPositionStructure;

	/*˫Ŀ���*/
	WorldRectPositionStructure TempRectPosition;
	
	/*˫Ŀ�������*/
	list<WorldRectPositionStructure> WorldRectPosition;

	/*ƥ������*/
	//list<WorldRectPosition>

	/* ȫ�ֲ����ṹ�� */
	typedef struct OverallParam
	{
		HObject Chambers;        //��Ԫ���������
		HTuple Wide, High;

		int ReadyCount;     //׼���׶μ���
		bool StartShoot;     //׼����������ʼ��
		bool NewStart;


		int Target1;      //����Ŀ��һ
		int Target2;      //����Ŀ���

		int PresentID;      //��ǰ֡Ŀ��ID
		int LastID;       //��һ֡ID

		float AngleX, AngleY, Distance;   //���ɱ���
		double X, Y, Z;    //���ɱ���
		double XT, YT, ZT;     //���ɱ���

	}OverallParam;

	OverallParam OAP;


	/*����ṹ��*/
	typedef struct FunReadPassword
	{
		int LastPassword[5];
		int PreaestPassword[5];

		/*������ı�־λ.1�������޸� 0������δ�޸�*/
		bool ChangePassword;
	}FunReadPassword;

	FunReadPassword Password;

	/*�ָ�Ź������ṹ��*/
	typedef struct FunSudokuDivitionParam
	{
		/*�Ҷ�ͼ*/
		HObject GrayImageL, GrayImageR;
		/*�Ҷ�ͼת��*/
		HObject ImageResultL, ImageResultR;
		/*ROI����*/
		HObject ImageROI_L, ImageROI_R;
		/*�����С���ͼ��*/
		HObject ImageReducedL, ImageReducedR;
		/*�Ҷ�ͼ��ת*/
		HObject ImageInvertL, ImageInvertR;
		/*��ֵ���ָ�ͼ��*/
		HObject RegionL, RegionR;
		/*��ֵ�˲�*/
		HObject ImageMeanL, ImageMeanR;
		/*��ͨ�˲�*/
		HObject RegionDynThreshL, RegionDynThreshR;
		/*����ָ�*/
		HObject RegionClippedL, RegionClippedR;
		/*���*/
		HObject RegionFillUpL, RegionFillUpR;
		/*������ͨ*/
		HObject ConnectedRegionsL, ConnectedRegionsR;
		/*����ɸѡ*/
		HObject SelectedRegionsL1, SelectedRegionsR1;
		HObject SelectedRegionsL2, SelectedRegionsR2;
		HObject SelectedRegionsL3, SelectedRegionsR3;
		HObject SelectedRegionsL4, SelectedRegionsR4;
		/*����*/
		HObject RegionIntersectionL, RegionIntersectionR;
		/*��������*/
		HObject SortedRegionL, SortedRegionR;
		/*����ѡ����ʱ����*/
		HObject ObjectSelected;
		HTuple Row, Column, Phi, Length1, Length2;
		HTuple Area,CenterRow, CenterColumn, Row1, Column1, Row2, Column2;

	}FunSudokuDivitionParam;

	FunSudokuDivitionParam SDP;

	

	/*��ȡ���λ�û��ID�ṹ��*/
	typedef struct FunGetRelativePosition
	{
		/*�����������*/
		list<SingleRect>::iterator it1;
		list<SingleRect>::iterator it2;
		/*�����ĵ�����*/
		double MidRow, MidColumn;
		/*IDд�������*/
		int GetPositionL;
		int GetPositionR;

	}FunGetRelativePosition;

	FunGetRelativePosition FGRP;

	/*��ȡ�Ź��������ֽṹ��*/
	typedef struct FunReadSudokuNumber
	{
		/*������ͼ��*/
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

		/*�����������*/
		list<SingleRect>::iterator it;

	}FunReadSudokuNumber;

	FunReadSudokuNumber FRSN;

	/*�Ź���˫Ŀƥ�亯���ṹ��*/
	typedef struct FunSudokuMatched
	{
		/*�����������*/
		list<SingleRect>::iterator SortedCounterL;
		list<SingleRect>::iterator SortedCounterR;
	}FunSudokuMatched;

	FunSudokuMatched FSM;

	/*���ݴ���ṹ��*/
	typedef struct FunTransmitBigMark
	{
		/*����������*/
		list<WorldRectPositionStructure>::iterator Read_WRPS;
		/*�м����*/
		double TempX, TempY, TempZ;
		double H_Angle;
		double V_Angle;
		double DIS;
		int Number;
		int RectangleID;

	}FunTransmitBigMark;

	FunTransmitBigMark FTBM;

	/*���������*/
	int LeftCounter;
	int RightCounter;
	/*ʶ�������*/
	int ReadNum;
	/*�������IDд��ɹ���־λ*/
	bool LeftID_OK;
	bool RightID_OK;
	/*˫Ŀ���ɹ���־λ*/
	bool WorldPositionMeasure;
	
public:

	/*������*/
	void ShootMark(const HObject& ImageL, const HObject& ImageR);

private:
	/*OCR�Ķ�����ʼ��*/
	void OCR_Ready(void);

	/*�ر�OCR�Ķ���*/
	void OCR_Close(void);

	/*�ָ�Ź�������*/
	bool SudokuDivition(const HObject& ImageL, const HObject& ImageR);

	/*�ָ���������*/
	bool PasswordDivition(const HObject& ImageL, const HObject& ImageR);

	/*����9��С�����������*/
	void GetRectPosition(void);

	/*����Ź������λ��*/
	void GetRelativePosition(void);

	/*��ȡ�Ź�������*/
	bool ReadSudokuNumber(char LOR);

	/*��ȡ����*/
	bool ReadPassword(void);

	/*�Ź���˫Ŀƥ��*/
	void SudokuMatched(void);

	/* ���ݸ�����ID���ظ�ID��Ԫ�����Ϣ */
	void GetCellInformation(int ID,double *X,double *Y,double *Z,
		double *Angle1,double *Angle2,double *DIS);

	/*�������*/
	void ClearList(void);

	/*����������*/
	void PasswordTransmit(void);

	/*˫Ŀ�Ź������ݴ���*/
	void MatchedTransmitBigMark(void);

	/*��Ŀ�Ź������ݴ���*/
	void SingleTransmitBigMark(char LOR);
};



#endif