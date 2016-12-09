/************************************************************************************
*                                  攻击模式
* 流程：
*      装甲板识别：先识别灯条，然后将灯条两两匹配为一个装甲板
*	   单目测量，将结果存入结构体链表，对结果进行判断，如果近距离有一个或多个目标，
*                对目标进行筛选，坐标变换，发送该目标坐标
*	   左右相机目标匹配：根据单目测量结果，将左相机测量结果变换到右相机，结合右相机
*                测量结果，距离最近的匹配为同一个装甲
*	   根据前后两帧的距离信息对装甲的ID进行匹配，用于跟踪
*      根据姿态，距离信息对每块装甲的设计难度进行评估，根据评估结果选择设计目标
*
*************************************************************************************/

#ifndef ARMORDETECT_H
#define ARMORDETECT_H

#include "stdafx.h"

/*红蓝标识符*/
#define BLUE 1
#define RED 0

/*亚像素图像面积阈值*/
#define XLD_MAX_AREA 8000
#define XLD_MIN_AREA 500

/*区域面积阈值*/
#define REGION_MAX_AREA 7000
#define REGION_MIN_AREA 100

/*装甲面积阈值*/
#define ARMOR_MIN_AREA 200

/*实际装甲宽度换算因子*/
#define REALITY_ARMOR_HIGH 2.455
/*灯条实际长度*/
#define REALITY_LIGHT_LENGTH 0.06
/*方向角阈值*/
#define PHI_THRESHOLD 0.27
/*离心率形态阈值*/
#define RA_RATIO_RB 1.4

/*长度权重*/
#define LENGTH_WEIGHT 1
/*方向角权重*/
#define PHI_WEIGHT 333
/*高度权重*/
#define ROW_WEIGHT 2.67

/*相机匹配阈值*/
#define X_DEEP_THRESHOLD 0.3
#define Z_HIGH_THRESHOLD 0.6


/*灯条匹配阈值*/
#define LIGHT_ERROR_LENGTH 20
#define LIGHT_ERROR_PHI 40
#define LIGHT_ERROR_Row 40
class ArmorDetect
{
public:
	ArmorDetect(void);
	~ArmorDetect(void);

	/*串口发送目标状态ID*/
	int EnemyID;

	/*LARGE_INTEGER共用体，兼容32位和64位*/
	/*计时*/
	LARGE_INTEGER Freq;
	LARGE_INTEGER BeginTime;
	LARGE_INTEGER EndTime;

	/*两帧之间时间差*/

	int time;

	/*文件写入，保存数据*/
	ofstream SaveTheData;

private:
	/*灯条信息结构体*/
	typedef struct LED
	{
		/*灯条区域*/
		HObject Region;

		/*中心点坐标*/
		double Row;
		double Column;

		/*XID亚像素级椭圆拟合数据*/
		/*方向*/
		double Phi;

		/*长半轴*/
		double Radius1;

		/*短半轴*/
		double Radius2;

		/*灯条长宽*/
		double Length;

		double High;

		/*灯条面积*/
		double Area;

	}LED;
	/*灯条链表*/
	list<LED> LEDList;

	/*装甲信息结构体*/
	typedef struct Armor
	{
		/*装甲编号*/
		int ID;

		/*左灯条相关信息*/
		double LeftLED_Ra, LeftLED_Phi, LeftLED_Row, LeftLED_Column;
        
		/*右灯条相关信息*/
		double RightLED_Ra, RightLED_Phi, RightLED_Row, RightLED_Column;

		/*装甲板顶点坐标*/
		double Rowa, Rowb, Rowc, Rowd;
		double Columna, Columnb, Columnc, Columnd;

		/*拟合后的四边形Region*/
		HObject Polygon;

		/*装甲板面积*/
		double Area;

		/*装甲板中心点坐标*/
		double CenterRow;
		double CenterColumn;

		/*装甲板姿态点*/
		HTuple Rows;
		HTuple Columns;

		/*相机相关参数*/
		/*相对相机坐标信息(极坐标，直角坐标)*/
		double Distance;
		double AngleH, AngleV;

		double SCX;
		double SCY;
		double SCZ;

		/*Z轴旋转角*/
		double spin_Z_angle;

		/*Z轴高度*/
		double Z_High;

		/*单目测量标志位*/
		bool FlagForSingleCamMeasure;
	}Armor;

	/*左右相机装甲链表*/
	list<Armor> Left_ArmorList;
	list<Armor> Right_ArmorList;

	/*匹配完成后的链表*/
	list<int> Left_Match;
	list<int> Right_Match;

	/*装甲匹配完成结构体*/
	typedef struct ArmorMatched
	{
		/*坐标*/
		double X;
		double Y;
		double Z;

		/*距离*/
		double Distance;

		/*角度*/
		double Row;
		double Column;
	}ArmorMatched;

	/* 锁定射击目标 */
	ArmorMatched ShootingEnemy;
	/* 上一时刻射击目标 */
	ArmorMatched LastEnemy;
	/* 射击难度最低目标 */
	ArmorMatched EasistEnemy;
	/* 查找目标是的零时目标 */
	ArmorMatched TempEnemy;

	/* 存储射击目标信息 */
	vector<ArmorMatched> ArmorList;

	/*全局过渡变量*/
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

	/*图像处理综合结构体*/
	typedef struct PictureHandle
	{ 
		/*Halcon算子必要变量*/
		HObject Red, Green, Blue;//RGB单通道分量图
		/*灰度图相关变量*/
		HObject ImageGray, ImageGrayRegion, ImageGrayConnectedRegion, ImageGrayDilation;
		HObject ImageGraySelectedRegion,ImageGrayRegionDilation;
		/*单色分量相关变量*/
		HObject ImageTemp1, ImageTemp2;
		HObject ImageIntersections;
		HObject ImageTempRegion1, ImageTempRegion2;
		HObject ImageIntersectionsDilation;
		HObject ImageIntersectionsFillUp;

		/*灯条边缘相关变量*/
		HObject LightEdge, LightEdgeDilation, LightEdgeSelectedRegion;
		HObject LightEdgeConnectedRegions;
		HObject LightShapeTrans;

		/*常规椭圆拟合变量*/
		HTuple RegionArea, RegionRow, RegionColumn;

		/*XLD亚像素级椭圆拟合相关变量*/
		HObject Contours, ContEllipse, LedRegion;
		HObject RegionEllipse, EllipseMinu;
		HObject Polygons;
		HObject XLDtrans;
		/*待选区域数目*/
		HTuple Num;
		/*灯条形态判断相关变量*/
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

	/*灯条匹配综合结构体*/

	typedef struct FunLampMatchParam
	{
		/*临时变量，用于比较*/
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

		/*LED链表，存储信息*/
		list<LED>::iterator LEDOne;
		list<LED>::iterator LEDTwo;

		list<LED>::iterator LEDTempOne;
		list<LED>::iterator LEDTempTwo;
	}FunLampMatchParam;

	FunLampMatchParam LMP;

	/*区域匹配综合结构体*/
	typedef struct FunRegionMatchParam
	{
		/*左右相机相关参数*/
		HTuple XR, YR, ZR;
		HTuple XL, YL, ZL;
		HTuple XRTL, YRTL, ZRTL;
		/*临时变量*/
		double MinDisXX;
		double tempDis;
		int IDR;
		int i;
		/*左右相机装甲结构体*/
		list<Armor>::iterator tempL;
		list<Armor>::iterator tempR;
	}FunRegionMatchParam;

	FunRegionMatchParam RMP;

	/*双目测量综合结构体*/
	typedef struct FunBinaryMeasureParam
	{
		/*左右相机相关参数*/
		HTuple X, Y, Z, Dist;
		HTuple RowL, RowR, ColumnL, ColumnR;
		double AreaL, AreaR;
		double AngleL, AngleR;

		/*ID链表*/
		list<int>::iterator IDL;
		list<int>::iterator IDR;

	}FunBinaryMeasureParam;

	FunBinaryMeasureParam BMP;

	/*单目求解3D坐标综合结构体*/
	typedef struct FunSingleCameraForColParam
	{

		/* 步兵、英雄装甲长宽 */
		HTuple Heigh, HeighS;
		HTuple Weight, WeightS;

		/* halcon算子必要参数 */
		HTuple RectRows, RectColumns;
		HTuple Pose, Quality;

		HObject Region;
		HTuple Area;
		HTuple Row, Column;

		list<Armor>::iterator it;

	}FunSingleCameraForColParam;

	FunSingleCameraForColParam SCFCP;

	/* 射击初始化结构体 */
	typedef struct FunShootInitParam
	{
		/* 左相机到右相机和右相机到左相机的转换 */
		HTuple LToR3D, RToL3D;

		/* 相机相对于云台的姿态 */
		HTuple PoseCamL;

		/* 从相机到云台的坐标转换 */
		HTuple HomCamLToGun;

	}FunShootInitParam;

	FunShootInitParam SIP;

public:
	/*********************************射击模式初始化********************************/
	void ShootInit(void);


	/*******************************主线程射击函数**********************************/
	void Shooting(const HObject& ImageL, const HObject& ImageR);


	/********************************装甲板检测*************************************/
	bool EnemyDistingguish(const HObject& Image, string LOR);


	/**************************从相机坐标变换到云台坐标********************************/
	void TransFCamToCT(double& XC, double& YC, double& ZC, double* XT, double* YT, double* ZT);

private:

	/********************************灯条匹配装甲************************************/
	void LampMatch(string Single);


	/*********************************双目测量****************************************/
	bool BinaryMeasure(void);


	/********************************左右相机单目求3D坐标****************************/
	void SingleCameraForCol(char Single);


	/********************************删除掉面积小的装甲******************************/
	void SelectArmor(void);

	/********************************左右相机匹配*************************************/
	bool RegionMatch(void);


	/*****************************得到一个新的灯板ID***********************************/
	int GetNewIDS(char LOR);

	/*******************************获得装甲信息***************************************/
	bool GetArmor(char LOR, int ID, double* Area, HTuple* Row, HTuple* Column, double* Angle);

	/**********************************链表更新***************************************/
	void ListUpdate(void);


	/*******************保存匹配好的装甲信息，坐标、距离、角度等信息*******************/
	void SaveData(const HTuple& X, const HTuple& Y, const HTuple& Z, const HTuple& angle, const double& Area, const HTuple RowL,
		const HTuple ColumnL);

	/**********************************选择射击目标***********************************/
	bool SelectObject(void);


};

#endif