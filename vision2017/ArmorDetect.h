/************************************************************************************
*                                  ����ģʽ
* ���̣�
*      װ�װ�ʶ����ʶ�������Ȼ�󽫵�������ƥ��Ϊһ��װ�װ�
*	   ��Ŀ���������������ṹ�������Խ�������жϣ������������һ������Ŀ�꣬
*                ��Ŀ�����ɸѡ������任�����͸�Ŀ������
*	   �������Ŀ��ƥ�䣺���ݵ�Ŀ������������������������任�����������������
*                ������������������ƥ��Ϊͬһ��װ��
*	   ����ǰ����֡�ľ�����Ϣ��װ�׵�ID����ƥ�䣬���ڸ���
*      ������̬��������Ϣ��ÿ��װ�׵�����ѶȽ��������������������ѡ�����Ŀ��
*
*************************************************************************************/

#ifndef ARMORDETECT_H
#define ARMORDETECT_H

#include "stdafx.h"

/*������ʶ��*/
#define BLUE 1
#define RED 0

/*������ͼ�������ֵ*/
#define XLD_MAX_AREA 8000
#define XLD_MIN_AREA 500

/*���������ֵ*/
#define REGION_MAX_AREA 7000
#define REGION_MIN_AREA 100

/*װ�������ֵ*/
#define ARMOR_MIN_AREA 200

/*ʵ��װ�׿�Ȼ�������*/
#define REALITY_ARMOR_HIGH 2.455
/*����ʵ�ʳ���*/
#define REALITY_LIGHT_LENGTH 0.06
/*�������ֵ*/
#define PHI_THRESHOLD 0.27
/*��������̬��ֵ*/
#define RA_RATIO_RB 1.4

/*����Ȩ��*/
#define LENGTH_WEIGHT 1
/*�����Ȩ��*/
#define PHI_WEIGHT 333
/*�߶�Ȩ��*/
#define ROW_WEIGHT 2.67

/*���ƥ����ֵ*/
#define X_DEEP_THRESHOLD 0.3
#define Z_HIGH_THRESHOLD 0.6


/*����ƥ����ֵ*/
#define LIGHT_ERROR_LENGTH 20
#define LIGHT_ERROR_PHI 40
#define LIGHT_ERROR_Row 40
class ArmorDetect
{
public:
	ArmorDetect(void);
	~ArmorDetect(void);

	/*���ڷ���Ŀ��״̬ID*/
	int EnemyID;

	/*LARGE_INTEGER�����壬����32λ��64λ*/
	/*��ʱ*/
	LARGE_INTEGER Freq;
	LARGE_INTEGER BeginTime;
	LARGE_INTEGER EndTime;

	/*��֮֡��ʱ���*/

	int time;

	/*�ļ�д�룬��������*/
	ofstream SaveTheData;

private:
	/*������Ϣ�ṹ��*/
	typedef struct LED
	{
		/*��������*/
		HObject Region;

		/*���ĵ�����*/
		double Row;
		double Column;

		/*XID�����ؼ���Բ�������*/
		/*����*/
		double Phi;

		/*������*/
		double Radius1;

		/*�̰���*/
		double Radius2;

		/*��������*/
		double Length;

		double High;

		/*�������*/
		double Area;

	}LED;
	/*��������*/
	list<LED> LEDList;

	/*װ����Ϣ�ṹ��*/
	typedef struct Armor
	{
		/*װ�ױ��*/
		int ID;

		/*����������Ϣ*/
		double LeftLED_Ra, LeftLED_Phi, LeftLED_Row, LeftLED_Column;
        
		/*�ҵ��������Ϣ*/
		double RightLED_Ra, RightLED_Phi, RightLED_Row, RightLED_Column;

		/*װ�װ嶥������*/
		double Rowa, Rowb, Rowc, Rowd;
		double Columna, Columnb, Columnc, Columnd;

		/*��Ϻ���ı���Region*/
		HObject Polygon;

		/*װ�װ����*/
		double Area;

		/*װ�װ����ĵ�����*/
		double CenterRow;
		double CenterColumn;

		/*װ�װ���̬��*/
		HTuple Rows;
		HTuple Columns;

		/*�����ز���*/
		/*������������Ϣ(�����ֱ꣬������)*/
		double Distance;
		double AngleH, AngleV;

		double SCX;
		double SCY;
		double SCZ;

		/*Z����ת��*/
		double spin_Z_angle;

		/*Z��߶�*/
		double Z_High;

		/*��Ŀ������־λ*/
		bool FlagForSingleCamMeasure;
	}Armor;

	/*�������װ������*/
	list<Armor> Left_ArmorList;
	list<Armor> Right_ArmorList;

	/*ƥ����ɺ������*/
	list<int> Left_Match;
	list<int> Right_Match;

	/*װ��ƥ����ɽṹ��*/
	typedef struct ArmorMatched
	{
		/*����*/
		double X;
		double Y;
		double Z;

		/*����*/
		double Distance;

		/*�Ƕ�*/
		double Row;
		double Column;
	}ArmorMatched;

	/* �������Ŀ�� */
	ArmorMatched ShootingEnemy;
	/* ��һʱ�����Ŀ�� */
	ArmorMatched LastEnemy;
	/* ����Ѷ����Ŀ�� */
	ArmorMatched EasistEnemy;
	/* ����Ŀ���ǵ���ʱĿ�� */
	ArmorMatched TempEnemy;

	/* �洢���Ŀ����Ϣ */
	vector<ArmorMatched> ArmorList;

	/*ȫ�ֹ��ɱ���*/
	typedef struct ClassGlobalVariable
	{

		double XCP, YCP, ZCP, Diff;
		double XGP, YGP, ZGP;
		double AngleH, AngleV, Distence;

		HTuple WorldX;
		HTuple WorldY;
		HTuple WorldXS;
		HTuple WorldYS;

	}ClassGlobalVariable;

	ClassGlobalVariable CGV;

	/*ͼ�����ۺϽṹ��*/
	typedef struct PictureHandle
	{ 
		/*Halcon���ӱ�Ҫ����*/
		HObject Red, Green, Blue;//RGB��ͨ������ͼ
		/*�Ҷ�ͼ��ر���*/
		HObject ImageGray, ImageGrayRegion, ImageGrayConnectedRegion, ImageGrayDilation;
		HObject ImageGraySelectedRegion,ImageGrayRegionDilation;
		/*��ɫ������ر���*/
		HObject ImageTemp1, ImageTemp2;
		HObject ImageIntersections;
		HObject ImageTempRegion1, ImageTempRegion2;
		HObject ImageIntersectionsDilation;
		HObject ImageIntersectionsFillUp;

		/*������Ե��ر���*/
		HObject LightEdge, LightEdgeDilation, LightEdgeSelectedRegion;
		HObject LightEdgeConnectedRegions;
		HObject LightShapeTrans;

		/*������Բ��ϱ���*/
		HTuple RegionArea, RegionRow, RegionColumn;

		/*XLD�����ؼ���Բ�����ر���*/
		HObject Contours, ContEllipse, LedRegion;
		HObject RegionEllipse, EllipseMinu;
		HObject Polygons;
		HObject XLDtrans;
		/*��ѡ������Ŀ*/
		HTuple Num;
		/*������̬�ж���ر���*/
		HTuple XldRow, XldColumn;
		HTuple XldArea;
		HTuple XldPointOrder;

		HTuple Row, Column, Phi, Radius1, Radius2, StartPhi, EndPhi, PointOrder;
		HTuple Rowa, Rowb, Rowc, Rowd;
		HTuple Columna, Columnb, Columnc, Columnd;
		HTuple PloygonRow, PloygonColumn;
		HTuple AreaRegion, AreaEllipse, tempRow, tempCol;

		double temp;
		double AreaRete;

		LED LEDtemp;

		list<Armor>::iterator tempL;
		list<Armor>::iterator tempR;
	}PictureHandle;

	PictureHandle EDP;

	/*����ƥ���ۺϽṹ��*/

	typedef struct FunLampMatchParam
	{
		/*��ʱ���������ڱȽ�*/
		Armor Armortemp;

		double CompareError;
		double TempErrorLength, TempErrorPhi, TempErrorRow;
		double TempError;
		double tempRow1, tempRow2;
		double tempColumn1, tempColumn2;
		double tempPhi1, tempPhi2;
		double tempLength1, tempLength2;
		double tempRadius11, tempRadius12;
		double tempHigh;
		bool hero;

		/*LED�����洢��Ϣ*/
		list<LED>::iterator LEDOne;
		list<LED>::iterator LEDTwo;

		list<LED>::iterator LEDTempOne;
		list<LED>::iterator LEDTempTwo;
	}FunLampMatchParam;

	FunLampMatchParam LMP;

	/*����ƥ���ۺϽṹ��*/
	typedef struct FunRegionMatchParam
	{
		/*���������ز���*/
		HTuple XR, YR, ZR;
		HTuple XL, YL, ZL;
		HTuple XRTL, YRTL, ZRTL;
		/*��ʱ����*/
		double MinDisXX;
		double tempDis;
		int IDR;
		int i;
		/*�������װ�׽ṹ��*/
		list<Armor>::iterator tempL;
		list<Armor>::iterator tempR;
	}FunRegionMatchParam;

	FunRegionMatchParam RMP;

	/*˫Ŀ�����ۺϽṹ��*/
	typedef struct FunBinaryMeasureParam
	{
		/*���������ز���*/
		HTuple X, Y, Z, Dist;
		HTuple RowL, RowR, ColumnL, ColumnR;
		double AreaL, AreaR;
		double AngleL, AngleR;

		/*ID����*/
		list<int>::iterator IDL;
		list<int>::iterator IDR;

	}FunBinaryMeasureParam;

	FunBinaryMeasureParam BMP;

	/*��Ŀ���3D�����ۺϽṹ��*/
	typedef struct FunSingleCameraForColParam
	{

		/* ������Ӣ��װ�׳��� */
		HTuple Heigh, HeighS;
		HTuple Weight, WeightS;

		/* halcon���ӱ�Ҫ���� */
		HTuple RectRows, RectColumns;
		HTuple Pose, Quality;

		HObject Region;
		HTuple Area;
		HTuple Row, Column;

		list<Armor>::iterator it;

	}FunSingleCameraForColParam;

	FunSingleCameraForColParam SCFCP;

	/* �����ʼ���ṹ�� */
	typedef struct FunShootInitParam
	{
		/* ����������������������������ת�� */
		HTuple LToR3D, RToL3D;

		/* ����������̨����̬ */
		HTuple PoseCamL;

		/* ���������̨������ת�� */
		HTuple HomCamLToGun;

	}FunShootInitParam;

	FunShootInitParam SIP;

public:
	/*********************************���ģʽ��ʼ��********************************/
	void ShootInit(void);


	/*******************************���߳��������**********************************/
	void Shooting(const HObject& ImageL, const HObject& ImageR);


	/********************************װ�װ���*************************************/
	bool EnemyDistingguish(const HObject& Image, string LOR);


	/**************************���������任����̨����********************************/
	void TransFCamToCT(double& XC, double& YC, double& ZC, double* XT, double* YT, double* ZT);

private:

	/********************************����ƥ��װ��************************************/
	void LampMatch(string Single);


	/*********************************˫Ŀ����****************************************/
	bool BinaryMeasure(void);


	/********************************���������Ŀ��3D����****************************/
	void SingleCameraForCol(char Single);


	/********************************ɾ�������С��װ��******************************/
	void SelectArmor(void);

	/********************************�������ƥ��*************************************/
	bool RegionMatch(void);


	/*****************************�õ�һ���µĵư�ID***********************************/
	int GetNewIDS(char LOR);

	/*******************************���װ����Ϣ***************************************/
	bool GetArmor(char LOR, int ID, double* Area, HTuple* Row, HTuple* Column, double* Angle);

	/**********************************�������***************************************/
	void ListUpdate(void);


	/*******************����ƥ��õ�װ����Ϣ�����ꡢ���롢�Ƕȵ���Ϣ*******************/
	void SaveData(const HTuple& X, const HTuple& Y, const HTuple& Z, const HTuple& angle, const double& Area, const HTuple RowL,
		const HTuple ColumnL);

	/**********************************ѡ�����Ŀ��***********************************/
	bool SelectObject(void);


};

#endif