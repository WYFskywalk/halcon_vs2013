#include "ArmorDetect.h"
#include "Threads.h"
#include "VisionInit.h"
#include "MathHandle.h"
#include "SerialPort.h"
#include<HalconCpp.h>
#include<HDevThread.h>
using namespace std;
using namespace HalconCpp;


extern Threads LThreads;
extern VisionInit LInit;
extern MathClass LMathClass;
extern CSerialPort LSerialPort;
extern bool ROB;

ArmorDetect::ArmorDetect(void)
{
	CGV.WorldX = ((((HTuple(-135).Append(135)).Append(135)).Append(-135)).Append(-135)) / 2000.0;
	CGV.WorldY = ((((HTuple(60).Append(60)).Append(-60)).Append(-60)).Append(60)) / 2000.0;
	CGV.WorldXS = ((((HTuple(-225).Append(225)).Append(225)).Append(-225)).Append(-225)) / 2000.0;
	CGV.WorldYS = ((((HTuple(60).Append(60)).Append(-60)).Append(-60)).Append(60)) / 2000.0;
	ShootingEnemy = { 0, 0, 998, 998, 998, 998 };
	//SaveTheDates.open("Date.txt"); 
}


ArmorDetect::~ArmorDetect(void)
{
}



/*******************************************************************************
Function:ShootInit(void)
Description:射击初始化
Calls:无
Called By:MainThread(void)
Input:无
Output:无
Return:无
Others:
********************************************************************************/
void ArmorDetect::ShootInit(void)
{
	/*获得右相机到左相机的转换矩阵*/
	PoseToHomMat3d(LInit.RelPose, &SIP.RToL3D);
	/*获得左相机到右相机的转换矩阵*/
	HomMat3dInvert(SIP.RToL3D, &SIP.LToR3D);

	/*保存左相机到云台的转换矩阵，需手动测试*/
	SIP.PoseCamL.Clear();
	SIP.PoseCamL[0] = -0.185;
	SIP.PoseCamL[1] = 0.085;
	SIP.PoseCamL[2] = 0.16;
	SIP.PoseCamL[3] = 0;
	SIP.PoseCamL[4] = 0;
	SIP.PoseCamL[5] = 0;
	SIP.PoseCamL[6] = 0;

	/*得到左相机到云台转换矩阵*/
	PoseToHomMat3d(SIP.PoseCamL, &SIP.HomCamLToGun);

}



/*******************************************************************************
Function:Shooting(const HObject &ImageL, const HObject &ImageR)
Description:主线程射击函数
Calls:ListUpdate()
      EnemyDistingguish(const HObject& Image, string LOR)
	  SingleCameraForCol(char Single)
	  SelectArmor()
	  RegionMatch()
	  BinaryMeasure()
	  SelectObject()
	  TransFCamToCT(double& XC, double& YC, double& ZC, double* XT, double* YT, double* ZT)
	  void CSerialPort::SendData(float Data0, float Data1, float Data2, float Data3, int Data4, int DataId)
	  void MathClass::LowPassFilterFun(double* new_value)
Called By:MainThread(void)
Input:
      ImageL：左相机图像
	  ImageR：右相机图像
Output:无
Return:无
Others:装甲板射击主函数
********************************************************************************/
void ArmorDetect::Shooting(const HObject &ImageL, const HObject &ImageR)
{
	/*链表更新*/
	ListUpdate();
	/**************************目标检测开始***************************/
	if (EnemyDistingguish(ImageL, "L") && EnemyDistingguish(ImageR, "R"))
	{
		/*单目测量*/
		try
		{
			SingleCameraForCol('L');
			SingleCameraForCol('R');
		}
		catch (...)
		{
			cout << "单目测量错误..." << endl;
		}

		/*删除面积过小的装甲*/
		SelectArmor();
		/*左右相机目标匹配*/
		if (RegionMatch())
		{
			/* 目标测量&&结果存储 */
			if (!BinaryMeasure())
			{
				cout << "Binary Measure Error..." << endl;
			}
		}

		/*选择最近射击目标*/
		if (SelectObject())
		{
			/* 目标坐标转换 */
			TransFCamToCT(ShootingEnemy.X, ShootingEnemy.Y, ShootingEnemy.Z, &CGV.XGP, &CGV.YGP, &CGV.ZGP);
			EnemyID = Shoot;
		}
		else
		{
			EnemyID = PauseShoot;
		}
	/**************************目标检测结束***************************/
	}
	else
	{
		cout << "Function EnemyDistingguish: No Body" << endl;
		EnemyID = PauseShoot;
	}

	try
	{
		if (EnemyID == Shoot)
		{
			QueryPerformanceCounter(&EndTime);
			//time = (double)(EndTime.QuadPart - BeginTime.QuadPart) / (double)Freq.QuadPart;
			time = (int)((double)(EndTime.QuadPart) / (double)Freq.QuadPart * 1000);
			//QueryPerformanceCounter(&BeginTime);
			cout << "主线程总时间" << time << endl;

			LMathClass.LowPassFilterFun(&CGV.XGP);
			LSerialPort.SendData((float)CGV.XGP, (float)CGV.YGP, (float)CGV.ZGP, 0, (float)time, EnemyID);
			//SaveTheDates << (float)CGV.XGP << " " << (float)CGV.YGP << " " << (float)CGV.ZGP << " " << (int)time << endl;
			cout << (float)CGV.XGP << " " << (float)CGV.YGP << " " << (float)CGV.ZGP << " " << EnemyID << endl;
		}
		else
		{
			LSerialPort.SendData(0, 0, 0, 0, 0, EnemyID);
		}
	}
	catch (...){
		LSerialPort.SerialPortTest();
	}
	/*调试用代码，可注释*/
	list<Armor>::iterator LCam;
	try
	{
		HDevWindowStack::SetActive(LInit.WindowHandleL);
		ClearWindow(HDevWindowStack::GetActive());
		for (LCam = Left_ArmorList.begin(); LCam != Left_ArmorList.end(); ++LCam)
		{
			if (HDevWindowStack::IsOpen())
			{
				SetColor(HDevWindowStack::GetActive(), "green");
				DispObj(LCam->Polygon, HDevWindowStack::GetActive());
			}
		}
		//DispCircle(LInit.WindowHandleL, ShootingEnemy.Row, ShootingEnemy.Column, 10);
		//vector<ArmorMatched>::iterator its;
		//for (its = ArmorList.begin(); its != ArmorList.end(); ++its)
		//{
		//	SetTposition(LInit.WindowHandleL, its->Row, its->Column);
		//	WriteString(LInit.WindowHandleL, LCam->ID);
		//}
		SetColor(HDevWindowStack::GetActive(), "red");
	}
	catch (...)
	{
		cout << "Camera Left Draw Pane Error" << endl;
	}
	list<Armor>::iterator RCam;
	try
	{
		HDevWindowStack::SetActive(LInit.WindowHandleR);
		ClearWindow(HDevWindowStack::GetActive());
		for (RCam = Right_ArmorList.begin(); RCam != Right_ArmorList.end(); ++RCam)
		{
			if (HDevWindowStack::IsOpen())
			{
				SetColor(HDevWindowStack::GetActive(), "green");
				DispObj(RCam->Polygon, HDevWindowStack::GetActive());
			}
		}
		SetColor(HDevWindowStack::GetActive(), "red");
	}
	catch (...)
	{
		cout << "Camera Right Draw Pane Error" << endl;
	}



}



/*******************************************************************************
Function:EnemyDistingguish(const HObject& Image, string LOR)
Description:装甲板检测，向LEDList，Armor结构体写入数据
Calls:
       LampMatch(string Single)
Called By:
       Shooting(const HObject &ImageL, const HObject &ImageR)
Input:
       Image：传入的图像
	   LOR：左右标识符，L代表左相机，R代表右相机
Output:无
Return:
       识别成功返回TRUE，否则返回FALSE
Others:灯条识别算法说明见函数内部,向LEDList，Armor结构体写入数据
       重要更新：
	   2016/12/8 修改经测试后的阈值化和膨胀系数，减少膨胀次数
********************************************************************************/
bool ArmorDetect::EnemyDistingguish(const HObject& Image, string LOR)
{
	Decompose3(Image, &EDP.Red, &EDP.Green, &EDP.Blue);
	Rgb3ToGray(EDP.Red, EDP.Green, EDP.Blue, &EDP.ImageGray);
	/*算法思路：将灯条拆分为两个部分，一个是中心高亮部分，另一个是周边颜色分量部分
	*通过阈值化和面积筛选先选出高亮部分膨胀，再用SubImage和intersec算子得到周边单色光部分膨胀
	*将两部分取交集后得到目标灯条附近单色光区域，再进行膨胀，椭圆拟合，便于进一步筛选
	*/
	if (ROB == BLUE)//蓝色
	{
		/*拆分白光区域*/
		Threshold(EDP.ImageGray, &EDP.ImageGrayRegion, 180, 255);
		Connection(EDP.ImageGrayRegion, &EDP.ImageGrayConnectedRegion);
		DilationCircle(EDP.ImageGrayConnectedRegion, &EDP.ImageGrayDilation,1.5);
		SelectShape(EDP.ImageGrayDilation, &EDP.ImageGraySelectedRegion, "area", "and", 40, 6000);
		//DilationCircle(EDP.ImageGraySelectedRegion, &EDP.ImageGrayRegionDilation, 1.5);

		/*拆分蓝色高分量区域*/
		SubImage(EDP.Blue, EDP.Red, &EDP.ImageTemp1,1.0,0);
		SubImage(EDP.Blue, EDP.Green, &EDP.ImageTemp2, 1.0, 0);
		Threshold(EDP.ImageTemp1, &EDP.ImageTempRegion1, 30, 255);
		Threshold(EDP.ImageTemp2, &EDP.ImageTempRegion2, 30, 255);
		Intersection(EDP.ImageTempRegion1, EDP.ImageTempRegion2, &EDP.ImageIntersections);
		DilationCircle(EDP.ImageIntersections, &EDP.ImageIntersectionsDilation, 2.5);
		FillUp(EDP.ImageIntersectionsDilation,&EDP.ImageIntersectionsFillUp);

		/*取交集，获取灯条边缘单色分量*/
		Intersection(EDP.ImageGrayDilation, EDP.ImageIntersectionsFillUp, &EDP.LightEdge);
		//DilationCircle(EDP.LightEdge, &EDP.LightEdgeDilation,3.5);
        
		/*第二次筛选，清除反光区域*/
		Connection(EDP.LightEdge, &EDP.LightEdgeConnectedRegions);
		SelectShape(EDP.LightEdgeConnectedRegions, &EDP.LightEdgeSelectedRegion, "area", "and", 20, 6000);
	}
	else if (ROB == RED)//红色
	{
		/*拆分白光区域*/
		Threshold(EDP.ImageGray, &EDP.ImageGrayRegion, 180, 255);
		Connection(EDP.ImageGrayRegion, &EDP.ImageGrayConnectedRegion);
		DilationCircle(EDP.ImageGrayConnectedRegion, &EDP.ImageGrayDilation, 1.5);
		SelectShape(EDP.ImageGrayDilation, &EDP.ImageGraySelectedRegion, "area", "and", 40, 6000);
		//DilationCircle(EDP.ImageGraySelectedRegion, &EDP.ImageGrayRegionDilation, 1.5);

		/*拆分蓝色高分量区域*/
		SubImage(EDP.Red, EDP.Blue, &EDP.ImageTemp1, 1.0, 0);
		SubImage(EDP.Red, EDP.Green, &EDP.ImageTemp2, 1.0, 0);
		Threshold(EDP.ImageTemp1, &EDP.ImageTempRegion1, 30, 255);
		Threshold(EDP.ImageTemp2, &EDP.ImageTempRegion2, 30, 255);
		Intersection(EDP.ImageTempRegion1, EDP.ImageTempRegion2, &EDP.ImageIntersections);
		DilationCircle(EDP.ImageIntersections, &EDP.ImageIntersectionsDilation, 2.5);
		FillUp(EDP.ImageIntersectionsDilation, &EDP.ImageIntersectionsFillUp);

		/*取交集，获取灯条边缘单色分量*/
		Intersection(EDP.ImageGrayDilation, EDP.ImageIntersectionsFillUp, &EDP.LightEdge);
		//DilationCircle(EDP.LightEdge, &EDP.LightEdgeDilation,3.5);

		/*第二次筛选，清除反光区域*/
		Connection(EDP.LightEdge, &EDP.LightEdgeConnectedRegions);
		SelectShape(EDP.LightEdgeConnectedRegions, &EDP.LightEdgeSelectedRegion, "area", "and", 20, 6000);	
	}

	CountObj(EDP.LightEdgeSelectedRegion, &EDP.Num);

	/*************************显示灯条、调试用，可注释**********************/
	SetColor(HDevWindowStack::GetActive(), "red");
	if (LOR == "L")
	{
		HDevWindowStack::SetActive(LInit.WindowHandleL);
		if (HDevWindowStack::IsOpen())
		{
			ClearWindow(HDevWindowStack::GetActive());
			DispObj(EDP.LightEdgeSelectedRegion, HDevWindowStack::GetActive());
		}
	}
	else
	{
		HDevWindowStack::SetActive(LInit.WindowHandleR);
		if (HDevWindowStack::IsOpen())
		{
			ClearWindow(HDevWindowStack::GetActive());
			DispObj(EDP.LightEdgeSelectedRegion, HDevWindowStack::GetActive());
		}
	}
	/*************************************************************************/

	if (EDP.Num <= 1)
	{
		return false;
	}

	HObject FUCK;
	GenEmptyObj(&FUCK);

	/*********************************灯条形态筛选**********************************/
	for (int i = 0; i != EDP.Num; ++i)
	{
		SelectObj(EDP.LightEdgeSelectedRegion, &EDP.LedRegion, i + 1);//选中待处理区域
		//AreaCenter(EDP.LedRegion, &EDP.Area, &EDP.Row, &EDP.Column);//获取面积和中心点坐标
					
			try
			{
				ShapeTrans(EDP.LedRegion, &EDP.LightShapeTrans,"ellipse");
				AreaCenter(EDP.LightShapeTrans,&EDP.RegionArea,&EDP.RegionRow,&EDP.RegionColumn);
				GenContourRegionXld(EDP.LedRegion, &EDP.Contours, "border");
   				//ShapeTransXld(EDP.Contours, &EDP.XLDtrans, "ellipse");
				//AreaCenterXld(EDP.XLDtrans, &EDP.XldArea, &EDP.XldRow, &EDP.XldColumn, &EDP.XldPointOrder);
				FitEllipseContourXld(EDP.Contours, "fitzgibbon", -1, 0, 0, 200, 3, 2, &EDP.Row,
					&EDP.Column, &EDP.Phi, &EDP.Radius1, &EDP.Radius2, &EDP.StartPhi, &EDP.EndPhi,
					&EDP.PointOrder);//XLD椭圆拟合				
				//cout << abs(1.57 - (double)EDP.Phi) << "  " << (double)(EDP.Radius1 / EDP.Radius2) << endl;
				/*方向角，椭圆形态,椭圆面积判断*/
				if ((abs(1.57 - (double)EDP.Phi) < PHI_THRESHOLD)
					&& (((double)(EDP.Radius1 / EDP.Radius2)) > RA_RATIO_RB)
					//&&EDP.XldArea<=XLD_MAX_AREA
					//&&EDP.XldArea>=XLD_MIN_AREA
					&& EDP.RegionArea <= REGION_MAX_AREA
					&& EDP.RegionArea >= REGION_MIN_AREA
					)
				{
					ConcatObj(FUCK, EDP.LedRegion, &FUCK);
					/*数据写入*/
					EDP.LEDtemp.Row = (double)EDP.Row;
					EDP.LEDtemp.Column = (double)EDP.Column;
					EDP.LEDtemp.Phi = (double)EDP.Phi;
					EDP.LEDtemp.Radius1 = (double)EDP.Radius1;
					EDP.LEDtemp.Radius2 = (double)EDP.Radius2;
					EDP.LEDtemp.Length = (double)(EDP.Radius1 * 2);
					EDP.LEDtemp.Area = (double)EDP.RegionArea;
					EDP.LEDtemp.High = (EDP.Radius1 > EDP.Radius2 ? EDP.Radius1 : EDP.Radius2) * 2 *
						sin((double)EDP.Phi);
					/*保存到链表*/
					LEDList.push_back(EDP.LEDtemp);
				}
			 }
			catch (...)
			{
				continue;
			}
	}
	/****************************显示筛选结果***********************************/
	SetColor(HDevWindowStack::GetActive(), "red");
	if (LOR == "L")
	{
		HDevWindowStack::SetActive(LInit.WindowHandleL);
		if (HDevWindowStack::IsOpen())
		{
			//ClearWindow(HDevWindowStack::GetActive());
			DispObj(FUCK, HDevWindowStack::GetActive());
		}
	}
	else
	{
		HDevWindowStack::SetActive(LInit.WindowHandleR);
		if (HDevWindowStack::IsOpen())
		{
			//ClearWindow(HDevWindowStack::GetActive());
			DispObj(FUCK, HDevWindowStack::GetActive());
		}
	}

	/************************************匹配**********************************/
	if (LOR == "L")
		LampMatch("L");
	else if (LOR == "R")
		LampMatch("R");
	else
	{
		cout << "Function EnemyDistingguish: Param Error" << endl;
		return false;
	}
	LEDList.clear();//清空列表
	/*************************计算并保存装甲板顶点信息**************************/

	//int j = 1;
	if (LOR == "L")
	{
		for (EDP.tempL = Left_ArmorList.begin(); EDP.tempL != Left_ArmorList.end(); ++EDP.tempL)
		{
			EDP.tempL->Rowa = (double)EDP.tempL->LeftLED_Row 
				- sin((double)EDP.tempL->LeftLED_Phi)*(double)EDP.tempL->LeftLED_Ra;			
			EDP.tempL->Rowd = (double)EDP.tempL->LeftLED_Row 
				+ sin((double)EDP.tempL->LeftLED_Phi)*(double)EDP.tempL->LeftLED_Ra;			
			EDP.tempL->Columna = (double)EDP.tempL->LeftLED_Column 
				+ cos((double)EDP.tempL->LeftLED_Phi)*(double)EDP.tempL->LeftLED_Ra;
			EDP.tempL->Columnd = (double)EDP.tempL->LeftLED_Column 
				- cos((double)EDP.tempL->LeftLED_Phi)*(double)EDP.tempL->LeftLED_Ra;
			EDP.tempL->Rowb = (double)EDP.tempL->RightLED_Row 
				- sin((double)EDP.tempL->RightLED_Phi)*(double)EDP.tempL->RightLED_Ra;
			EDP.tempL->Rowc = (double)EDP.tempL->RightLED_Row 
				+ sin((double)EDP.tempL->RightLED_Phi)*(double)EDP.tempL->RightLED_Ra;		
			EDP.tempL->Columnb = (double)EDP.tempL->RightLED_Column 
				+ cos((double)EDP.tempL->RightLED_Phi)*(double)EDP.tempL->RightLED_Ra;			
			EDP.tempL->Columnc = (double)EDP.tempL->RightLED_Column 
				- cos((double)EDP.tempL->RightLED_Phi)*(double)EDP.tempL->RightLED_Ra;

			/* 判断左右灯条是否搞反 */
			if ((EDP.tempL->Columna) >= (EDP.tempL->Columnb))
			{//如果左右灯条搞反，a、b交换，c、d交换
				EDP.temp = EDP.tempL->Rowa;
				EDP.tempL->Rowa = EDP.tempL->Rowb;
				EDP.tempL->Rowb = EDP.temp;

				EDP.temp = EDP.tempL->Rowc;
				EDP.tempL->Rowc = EDP.tempL->Rowd;
				EDP.tempL->Rowd = EDP.temp;

				EDP.temp = EDP.tempL->Columna;
				EDP.tempL->Columna = EDP.tempL->Columnb;
				EDP.tempL->Columnb = EDP.temp;

				EDP.temp = EDP.tempL->Columnc;
				EDP.tempL->Columnc = EDP.tempL->Columnd;
				EDP.tempL->Columnd = EDP.temp;
			}

			EDP.PloygonRow.Clear();
			EDP.PloygonRow.Append(EDP.tempL->Rowa);
			EDP.PloygonRow.Append(EDP.tempL->Rowb);
			EDP.PloygonRow.Append(EDP.tempL->Rowc);
			EDP.PloygonRow.Append(EDP.tempL->Rowd);
			EDP.PloygonRow.Append(EDP.tempL->Rowa);
			EDP.PloygonColumn.Clear();
			EDP.PloygonColumn.Append(EDP.tempL->Columna);
			EDP.PloygonColumn.Append(EDP.tempL->Columnb);
			EDP.PloygonColumn.Append(EDP.tempL->Columnc);
			EDP.PloygonColumn.Append(EDP.tempL->Columnd);
			EDP.PloygonColumn.Append(EDP.tempL->Columna);
			
			EDP.tempL->Rows = EDP.PloygonRow;
			EDP.tempL->Columns = EDP.PloygonColumn;

			GenContourPolygonXld(&EDP.Polygons, EDP.PloygonRow, EDP.PloygonColumn);
			EDP.tempL->Polygon = EDP.Polygons;

			EDP.tempL->FlagForSingleCamMeasure = 0;
			EDP.tempL->ID = GetNewIDS('L');
			//EDP.tempL->ID =j++;
			
		}
	}
	else if (LOR == "R")
	{
		for (EDP.tempR = Right_ArmorList.begin(); EDP.tempR != Right_ArmorList.end(); ++EDP.tempR)
		{
			EDP.tempR->Rowa = (double)EDP.tempR->LeftLED_Row 
				- sin((double)EDP.tempR->LeftLED_Phi)*(double)EDP.tempR->LeftLED_Ra;			
			EDP.tempR->Rowd = (double)EDP.tempR->LeftLED_Row 
				+ sin((double)EDP.tempR->LeftLED_Phi)*(double)EDP.tempR->LeftLED_Ra;		
			EDP.tempR->Columna = (double)EDP.tempR->LeftLED_Column 
				+ cos((double)EDP.tempR->LeftLED_Phi)*(double)EDP.tempR->LeftLED_Ra;			
			EDP.tempR->Columnd = (double)EDP.tempR->LeftLED_Column 
				- cos((double)EDP.tempR->LeftLED_Phi)*(double)EDP.tempR->LeftLED_Ra;
			EDP.tempR->Rowb = (double)EDP.tempR->RightLED_Row 
				- sin((double)EDP.tempR->RightLED_Phi)*(double)EDP.tempR->RightLED_Ra;			
			EDP.tempR->Rowc = (double)EDP.tempR->RightLED_Row 
				+ sin((double)EDP.tempR->RightLED_Phi)*(double)EDP.tempR->RightLED_Ra;			
			EDP.tempR->Columnb = (double)EDP.tempR->RightLED_Column 
				+ cos((double)EDP.tempR->RightLED_Phi)*(double)EDP.tempR->RightLED_Ra;			
			EDP.tempR->Columnc = (double)EDP.tempR->RightLED_Column 
				- cos((double)EDP.tempR->RightLED_Phi)*(double)EDP.tempR->RightLED_Ra;
			/* 判断左右灯条是否搞反 */
			if ((EDP.tempR->Columna) >= (EDP.tempR->Columnb))
			{//如果左右灯条搞反，a、b交换，c、d交换
				EDP.temp = EDP.tempR->Rowa;
				EDP.tempR->Rowa = EDP.tempR->Rowb;
				EDP.tempR->Rowb = EDP.temp;

				EDP.temp = EDP.tempR->Rowc;
				EDP.tempR->Rowc = EDP.tempR->Rowd;
				EDP.tempR->Rowd = EDP.temp;

				EDP.temp = EDP.tempR->Columna;
				EDP.tempR->Columna = EDP.tempR->Columnb;
				EDP.tempR->Columnb = EDP.temp;

				EDP.temp = EDP.tempR->Columnc;
				EDP.tempR->Columnc = EDP.tempR->Columnd;
				EDP.tempR->Columnd = EDP.temp;
			}

			EDP.PloygonRow.Clear();
			EDP.PloygonRow.Append(EDP.tempR->Rowa);
			EDP.PloygonRow.Append(EDP.tempR->Rowb);
			EDP.PloygonRow.Append(EDP.tempR->Rowc);
			EDP.PloygonRow.Append(EDP.tempR->Rowd);
			EDP.PloygonRow.Append(EDP.tempR->Rowa);
			EDP.PloygonColumn.Clear();
			EDP.PloygonColumn.Append(EDP.tempR->Columna);
			EDP.PloygonColumn.Append(EDP.tempR->Columnb);
			EDP.PloygonColumn.Append(EDP.tempR->Columnc);
			EDP.PloygonColumn.Append(EDP.tempR->Columnd);
			EDP.PloygonColumn.Append(EDP.tempR->Columna);

			EDP.tempR->Rows = EDP.PloygonRow;
			EDP.tempR->Columns = EDP.PloygonColumn;

			GenContourPolygonXld(&EDP.Polygons, EDP.PloygonRow, EDP.PloygonColumn);
			EDP.tempR->Polygon = EDP.Polygons;

			EDP.tempR->FlagForSingleCamMeasure = 0;
			EDP.tempR->ID = GetNewIDS('R');
			//EDP.tempL->ID =j++;
			
		}
	}
	return true;
}



/*******************************************************************************
Function:LampMatch(string Single)
Description:灯条匹配装甲，
Calls:无
Called By:
      EnemyDistingguish(const HObject& Image, string LOR)
Input:
      Single：L表示左相机装甲链表，R表示右相机装甲链表
Output:无
Return:无
Others:双重遍历，根据自定义误差参数来匹配灯条.向Armor写入左右灯条坐标和形态特征
       重要更新：
	   2016/12/8 根据测试确定部分误差系数
********************************************************************************/
void ArmorDetect::LampMatch(string Single)
{
	/*******************************双重循环*************************************/
	LMP.CompareError = 500;
	LMP.TempError = 500;
	/*没有足够的灯条匹配，直接返回*/
	if (LEDList.size() < 2)
	{
		return;
	}
	/**********************左灯条循环开始******************************/
	for (LMP.LEDOne = LEDList.begin(); LMP.LEDOne != LEDList.end(); ++LMP.LEDOne)
	{
		/**********************右灯条循环开始******************************/
		for (LMP.LEDTwo = ++LMP.LEDOne; LMP.LEDTwo != LEDList.end(); ++LMP.LEDTwo)
		{
			/*误差计算*/
			LMP.TempErrorLength = abs(LMP.LEDOne->Length - LMP.LEDTwo->Length)* LENGTH_WEIGHT; //长度误差，最大容忍度20，权重20*1=20
			LMP.TempErrorPhi = abs(LMP.LEDOne->Phi - LMP.LEDTwo->Phi) * PHI_WEIGHT;   //角度误差，最大容忍度0.12，权重0.12*333=40
			LMP.TempErrorRow = abs(LMP.LEDOne->Row - LMP.LEDTwo->Row)  * ROW_WEIGHT;  //高度误差，最大容忍度15，权重15*2.67=40
			LMP.TempError = LMP.TempErrorLength + LMP.TempErrorPhi + LMP.TempErrorRow;
			cout << "长度误差" << abs(LMP.LEDOne->Length - LMP.LEDTwo->Length)* LENGTH_WEIGHT << endl;
			cout << "角度误差" << abs(LMP.LEDOne->Phi - LMP.LEDTwo->Phi) * PHI_WEIGHT << endl;
			cout << "水平坐标误差" << abs(LMP.LEDOne->Row - LMP.LEDTwo->Row) / (LMP.LEDOne->Row + LMP.LEDTwo->Row) * ROW_WEIGHT<< endl;
			cout << LMP.TempError << endl;

			if (LMP.TempErrorLength <= LIGHT_ERROR_LENGTH
				&&LMP.TempErrorPhi <= LIGHT_ERROR_PHI
				&&LMP.TempErrorRow <= LIGHT_ERROR_Row
				&&LMP.TempError <= LMP.CompareError)
			{
				LMP.CompareError = LMP.TempError;
				LMP.LEDTempTwo = LMP.LEDTwo;

				LMP.tempRow1 = LMP.LEDOne->Row;
				LMP.tempColumn1 = LMP.LEDOne->Column;
				LMP.tempRow2 = LMP.LEDTwo->Row;
				LMP.tempColumn2 = LMP.LEDTwo->Column;
				LMP.tempPhi1 = LMP.LEDOne->Phi;
				LMP.tempPhi2 = LMP.LEDTwo->Phi;
				LMP.tempRadius11 = LMP.LEDOne->Radius1;
				LMP.tempRadius12 = LMP.LEDTwo->Radius1;
				LMP.tempHigh = LMP.LEDOne->High;
			}
			else
			{//误差大于最小者
				continue;
			}			
		/**************************右灯条循环完成*********************************/
		}
		if (LMP.CompareError < 10)
		{
			LMP.Armortemp.LeftLED_Row = LMP.tempRow1;
			LMP.Armortemp.LeftLED_Column = LMP.tempColumn1;
			LMP.Armortemp.RightLED_Row = LMP.tempRow2;
			LMP.Armortemp.RightLED_Column = LMP.tempColumn2;
			LMP.Armortemp.LeftLED_Phi = LMP.tempPhi1;
			LMP.Armortemp.RightLED_Phi = LMP.tempPhi2;
			LMP.Armortemp.LeftLED_Ra = LMP.tempRadius11;
			LMP.Armortemp.RightLED_Ra = LMP.tempRadius12;
			LMP.Armortemp.Z_High = LMP.tempHigh;

			if (Single == "L")
			{
				Left_ArmorList.push_back(LMP.Armortemp);
			}
			else if (Single == "R")
			{
				Right_ArmorList.push_back(LMP.Armortemp);
			}
			else
			{
				cout << "Function LampMatch Error: Parameter Error!" << endl;
			}
		}
	/*************************左灯条循环完成*********************************/
	}

}



/*******************************************************************************
Function:SingleCameraForCol(char Single)
Description:左右相机单目求3D坐标，获取装甲板中心点坐标
Calls:
      MathClass::PolarCor2CartesianCor(const double& Distance, const HTuple& AngleX,
	       const HTuple& AngleY, double* X, double* Y, double* Z)
      MathClass::VRadiaHandleCL(const double& yy, double* angle)

Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:
       Single：L表示左相机装甲链表，R表示右相机装甲链表
Output:无
Return:无
Others:
********************************************************************************/
void ArmorDetect::SingleCameraForCol(char Single)
{
	if (Single == 'L')
	{
		for (SCFCP.it = Left_ArmorList.begin(); SCFCP.it != Left_ArmorList.end(); ++SCFCP.it)
		{
			try
			{   
				/* 由多边形轮廓生成区域 */
				GenRegionPolygon(&SCFCP.Region, SCFCP.it->Rows, SCFCP.it->Columns);
				/* 计算中心点 */
				AreaCenter(SCFCP.Region, &SCFCP.Area, &SCFCP.Row, &SCFCP.Column);
			}
			catch (...)
			{
				cout << "GenRegionPolygon Error！" << endl;
				continue;
			}
			SCFCP.it->Area = (double)SCFCP.Area;
			SCFCP.it->CenterRow = (double)SCFCP.Row;
			SCFCP.it->CenterColumn = (double)SCFCP.Column;

			SCFCP.it->spin_Z_angle = acos(abs(SCFCP.it->LeftLED_Column - SCFCP.it->RightLED_Column) / (SCFCP.it->Z_High * REALITY_ARMOR_HIGH)) * 180 / PI;

			LMathClass.VRadiaHandleCL((double)SCFCP.Row, &SCFCP.it->AngleV);
			LMathClass.HRadiaHandleCL((double)SCFCP.Column, &SCFCP.it->AngleH);

			SCFCP.it->Distance = (REALITY_LIGHT_LENGTH*CAMERALFOCUS) / ((SCFCP.it->Z_High) * PIXEL_SIZE);
			LMathClass.PolarCor2CartesianCor(SCFCP.it->Distance, SCFCP.it->AngleH, SCFCP.it->AngleV, &(SCFCP.it->SCX), &(SCFCP.it->SCY), &(SCFCP.it->SCZ));
			//cout << "left" << (double)(SCFCP.it->SCX) << "  " << (double)(SCFCP.it->SCY) << "  " << (double)(SCFCP.it->SCZ) << endl;
		}
	}
	else
	{
		for (SCFCP.it = Right_ArmorList.begin(); SCFCP.it != Right_ArmorList.end(); ++SCFCP.it)
		{
			try
			{
				/* 由多边形轮廓生成区域 */
				GenRegionPolygon(&SCFCP.Region, SCFCP.it->Rows, SCFCP.it->Columns);
				/* 计算中心点 */
				AreaCenter(SCFCP.Region, &SCFCP.Area, &SCFCP.Row, &SCFCP.Column);
			}
			catch (...)
			{
				continue;
			}
			SCFCP.it->Area = (double)SCFCP.Area;
			SCFCP.it->CenterRow = (double)SCFCP.Row;
			SCFCP.it->CenterColumn = (double)SCFCP.Column;

			SCFCP.it->spin_Z_angle = acos(abs(SCFCP.it->LeftLED_Column - SCFCP.it->RightLED_Column) / (SCFCP.it->Z_High * REALITY_ARMOR_HIGH)) * 180 / PI;

			LMathClass.VRadiaHandleCL((double)SCFCP.Row, &SCFCP.it->AngleV);
			LMathClass.HRadiaHandleCL((double)SCFCP.Column, &SCFCP.it->AngleH);

			SCFCP.it->Distance = (REALITY_LIGHT_LENGTH*CAMERARFOCUS) / ((SCFCP.it->Z_High) * PIXEL_SIZE);
			LMathClass.PolarCor2CartesianCor(SCFCP.it->Distance, SCFCP.it->AngleH, SCFCP.it->AngleV, &(SCFCP.it->SCX), &(SCFCP.it->SCY), &(SCFCP.it->SCZ));
			//cout << "left" << (double)(SCFCP.it->SCX) << "  " << (double)(SCFCP.it->SCY) << "  " << (double)(SCFCP.it->SCZ) << endl;
		}
	}
}



/*******************************************************************************
Function:SelectArmor(void)
Description:删除掉面积小的装甲
Calls:无
Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:无
Output:无
Return:无
Others:
       重要更新：
	   2016/11/18 加入装甲长宽比尺寸约束，修正在某些距离较近的情况下装甲误匹配的BUG
********************************************************************************/
void ArmorDetect::SelectArmor(void)
{
	/*链表迭代器*/
	list<Armor>::iterator it;
	/*装甲板长宽定义*/
	double ArmorWidth, ArmorHigh;
	double Rate;
	for (it = Left_ArmorList.begin(); it != Left_ArmorList.end();)
	{
		/*计算装甲长宽比*/
		ArmorWidth = abs(it->RightLED_Column - it->LeftLED_Column);
		ArmorHigh = (it->LeftLED_Ra + it->RightLED_Ra)/2;
		Rate = ArmorWidth / ArmorHigh;

		if ((it->Area < ARMOR_MIN_AREA) || Rate >= 5 || Rate<=1.0)
		{
			Left_ArmorList.erase(it++);
		}
		else
			++it;
	}

	for (it = Right_ArmorList.begin(); it != Right_ArmorList.end();)
	{
		/*计算装甲长宽比*/
		ArmorWidth = abs(it->RightLED_Column - it->LeftLED_Column);
		ArmorHigh = (it->LeftLED_Ra + it->RightLED_Ra)/2;
		Rate = ArmorWidth / ArmorHigh;
		
		if ((it->Area <ARMOR_MIN_AREA) || Rate >= 5 || Rate <= 1.0)
		{
			Right_ArmorList.erase(it++);
		}
		else
			++it;
	}
}



/*******************************************************************************
Function:RegionMatch(void)
Description:左右相机匹配
Calls:无
Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:无
Output:无
Return:匹配成功返回TRUE,否则返回FALSE
Others:
********************************************************************************/
bool ArmorDetect::RegionMatch(void)
{
	Left_Match.clear();
	Right_Match.clear();
    /********************************左相机循环开始***********************************/
	for (RMP.tempL = Left_ArmorList.begin(); RMP.tempL != Left_ArmorList.end(); ++RMP.tempL)
	{
		if (RMP.tempL->FlagForSingleCamMeasure == 1)
		{
			continue;
		}
		try
		{		
			/* 将坐标变换到右相机下 */
			AffineTransPoint3d(SIP.LToR3D, RMP.tempL->SCX, RMP.tempL->SCY, RMP.tempL->SCZ, &RMP.XRTL, &RMP.YRTL, &RMP.ZRTL);
		}
		catch (...)
		{
			cout << "AffineTransPoint3d Error！" << endl;
			continue;
		}
		RMP.MinDisXX = 1000;
		/**********************************右相机循环开始*******************************/
		for (RMP.tempR = Right_ArmorList.begin(); RMP.tempR != Right_ArmorList.end(); ++RMP.tempR)
		{
			if (RMP.tempR->FlagForSingleCamMeasure == 1)
			{
				continue;
			}

			/** 一定深度、竖直范围内 */
			if ((abs((double)RMP.YRTL - RMP.tempR->SCY) < X_DEEP_THRESHOLD) && (abs((double)RMP.ZRTL - RMP.tempR->SCZ) < Z_HIGH_THRESHOLD))
			{
				/* 计算水平坐标差距 */
				RMP.tempDis = abs((double)RMP.XRTL - (RMP.tempR->SCX));
				if (RMP.tempDis < RMP.MinDisXX)
				{//目标差距小于
					RMP.MinDisXX = RMP.tempDis;
					RMP.IDR = RMP.tempR->ID;
				}
			}
		/***********************************右相机循环结束*******************************/
		}
		if (RMP.MinDisXX < MAXMATCHPP)
		{//最近目标满足要求
			/* 存储左右相机ID */
			int IDtemp = RMP.tempL->ID;
			Left_Match.push_back(IDtemp);
			Right_Match.push_back(RMP.IDR);
			//cout << RMP.tempL->ID << "  " << RMP.IDR << endl;
		}
	/********************************左相机循环结束***********************************/
	}

	if ((!Left_Match.empty()) && (!Right_Match.empty()))
		return true;
	else
		return false;

}



/*******************************************************************************
Function:RegionMatch(void)
Description:双目测量
Calls:无
Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:无
Output:无
Return:测量成功返回TRUE,否则返回FALSE
Others:数据保存到ArmorMatched的容器中
********************************************************************************/
bool ArmorDetect::BinaryMeasure(void)
{
	/*错误判断*/
	if (Left_Match.size() != Right_Match.size())
	{
		cout << "Match Error: MatchedL unequal MatchedR..." << endl;
		return false;
	}
	if (Left_Match.empty() | Right_Match.empty())
	{
		cout << "Match Error: No Armor Be Matched..." << endl;
		return false;
	}
	/*链表循环*/
	for (BMP.IDL = Left_Match.begin(), BMP.IDR = Right_Match.begin(); BMP.IDL != Left_Match.end(); ++BMP.IDL, ++BMP.IDR)
	{
		if (GetArmor('L', *BMP.IDL, &BMP.AreaL, &BMP.RowL, &BMP.ColumnL, &BMP.AngleL) && GetArmor('R', *BMP.IDR, &BMP.AreaR, &BMP.RowR, &BMP.ColumnR, &BMP.AngleR))
		{
			try
			{
				/*双目摄像机求3D坐标*/
				IntersectLinesOfSight(LInit.CamParamL, LInit.CamParamR, LInit.RelPose, 
					BMP.RowL, BMP.ColumnL, BMP.RowR, BMP.ColumnR,
					&BMP.X, &BMP.Y, &BMP.Z, &BMP.Dist);
			}
			catch (...)
			{
				cout << "Binary Measure Error: Count Three-Dimensional Coordinates Error..." << endl;
				continue;
			}
			/*保存数据到装甲链表*/
			SaveData(BMP.X, BMP.Y, BMP.Z, BMP.AngleL, BMP.AreaL, BMP.RowL, BMP.ColumnL);

			/*检查是否遍历到链表结尾，如果否继续循环。是则直接返回*/
			if (((++BMP.IDL) != Left_Match.end()) && ((++BMP.IDR) != Right_Match.end()))
			{
				BMP.IDL--;
				BMP.IDR--;
			}
			else
			{
				return true;
			}

		}
		else
		{
			cout << "Binary Measure Error: Get Armor Pointer Error..." << endl;
			continue;
		}
	}
	return true;

}



/*******************************************************************************
Function:SelectObject(void)
Description:选择射击目标
Calls:无
Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:无
Output:无
Return:当找到射击目标且距离小于阈值时返回TRUE,否则返回FALSE
Others:代码分2种情况执行，1种是上次找到射击目标，另一种是上次未找到射击目标，通过static变量标识
********************************************************************************/
bool ArmorDetect::SelectObject(void)
{
	vector<ArmorMatched>::iterator it;
	/*标志位，用于辨别上次是否锁定射击目标.0:上次未识别到目标 1：上次识别到目标*/
	static int LastEnemyFlag = 0;
	//static double LastDistance;//上次射击距离
	double ShortestDistance = 1000.0;//离车体最短射击距离
	double DistanceBetweenEnemys ;//当前所有目标距离上次射击目标的距离（临时变量）
	double ShortestDistanceEnemys = 1000.0;//当前所有目标距离上次射击目标最近的距离
	
	/************首次调用和上次调用未识别到装甲板时执行下面代码**************/
	if (!LastEnemyFlag)
	{
		for (it = ArmorList.begin(); it != ArmorList.end(); ++it)
		{
			/*寻找距离车体最近的目标*/
			if (it->Distance < ShortestDistance)
			{
				ShortestDistance = it->Distance;
				EasistEnemy = *it;
			}
		}
		
		/*距离太远，直接返回false*/
		if (EasistEnemy.Distance > 5)
		{
			return false;
		}
		
		/*找到射击目标*/
		else
		{
			LastEnemyFlag = 1;//标志位置位
			LastEnemy = EasistEnemy;
			ShootingEnemy= EasistEnemy;//复制结构体
			return true;
		}
	}

	/********************上次调用识别到目标执行下列代码***********************/
	for (it = ArmorList.begin(); it != ArmorList.end(); ++it)
	{
		/*寻找距离车体最近射击目标*/
		if (it->Distance < ShortestDistance)
		{
			ShortestDistance = it->Distance;
			EasistEnemy = *it;
		}
		
		/*寻找距离上一次射击目标最近的目标*/
		LMathClass.Distance3DPp(it->X, it->Y, it->Z, LastEnemy.X, LastEnemy.Y, LastEnemy.Z, &DistanceBetweenEnemys);
		if (DistanceBetweenEnemys < ShortestDistanceEnemys)
		{
			ShortestDistanceEnemys = DistanceBetweenEnemys;
			TempEnemy = *it;
		}
	}
	
	/*如果TempEnemy和EasistEnemy差值不大于0.5米，选择TempEnemy为ShootingEnemy，否则选择EasistEnemy*/
	if ((TempEnemy.Distance - EasistEnemy.Distance) < 0.5)
	{
		ShootingEnemy = TempEnemy;
	}
	else
	{
		ShootingEnemy = EasistEnemy;
	}
	
	/*判断射击距离,若距离过长清空标志位*/
	if (ShootingEnemy.Distance < 5)
	{
		LastEnemy = ShootingEnemy;
		return true;
	}
	else
	{
		LastEnemyFlag = 0;
		return false;
	}
}



/*******************************************************************************
Function:ListUpdate(void)
Description:链表更新，清空各种链表
Calls:无
Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:无
Output:无
Return:无
Others:
********************************************************************************/
void ArmorDetect::ListUpdate(void)
{
	LEDList.clear();
	Left_ArmorList.clear();
	Right_ArmorList.clear();
	Left_Match.clear();
	Right_Match.clear();
	ArmorList.clear();
}



/*******************************************************************************
Function:GetNewIDS(char LOR)
Description:得到一个新的灯板ID
Calls:无
Called By:
      EnemyDistingguish(const HObject & Image, string LOR) (ArmorDetect)
Input:
      LOR:L表示左相机链表，R表示右相机链表
Output:无
Return:灯板ID
Others:
********************************************************************************/
int ArmorDetect::GetNewIDS(char LOR)
{
	int NewID = 1;
	list<Armor>::iterator it;
	if (LOR == 'L')
	{
		for (it = Left_ArmorList.begin(); it != Left_ArmorList.end(); ++it)
		{
			if (it->ID == NewID)
			{
				NewID++;
				it = Left_ArmorList.begin();
			}
		}
		return NewID;
	}
	else if (LOR == 'R')
	{
		for (it = Right_ArmorList.begin(); it != Right_ArmorList.end(); ++it)
		{
			if (it->ID == NewID)
			{
				NewID++;
				it = Right_ArmorList.begin();
			}
		}
		return NewID;
	}
	else
	{
		cout << "Function GetNewID Input Error..." << endl;
		return 0;
	}
}



/*******************************************************************************
Function:SaveData(const HTuple& X, const HTuple& Y, const HTuple& Z, const HTuple& angle,
	const double& Area, const HTuple RowL, const HTuple ColumnL)
Description:保存匹配好的装甲信息，坐标、距离、角度等信息
Calls:无
Called By:
      BinaryMeasure(void) 
Input:
      X,Y,Z,angle,Area,RowL, ColumnL:待保存的数据
Output:无
Return:无
Others:数据写入到ArmorList的容器中
********************************************************************************/
void ArmorDetect::SaveData(const HTuple& X, const HTuple& Y, const HTuple& Z, const HTuple& angle,
	const double& Area, const HTuple RowL, const HTuple ColumnL)
{
	ArmorMatched Temp;
	double Distance;
	LMathClass.DistanceToCamera(X, Y, Z, &Distance);
	Temp.X = X;
	Temp.Y = Y;
	Temp.Z = Z;
	Temp.Distance = Distance;
	Temp.Row = RowL;
	Temp.Column = ColumnL;
	ArmorList.push_back(Temp);
}



/*******************************************************************************
Function:GetArmor(char LOR, int ID, double* Area, HTuple* Row, HTuple* Column, double* Angle)
Description:获得装甲信息
Calls:无
Called By:
      BinaryMeasure(void) 
Input:
      LOR：左右标志位，L表示左相机链表，R表示右相机链表
Output:
      Area，Row，Column，Column，Angle：装甲相关信息
Return:无
Others:当出现非法LOR参数时返回FALSE,其余返回TRUE
********************************************************************************/
bool ArmorDetect::GetArmor(char LOR, int ID, double* Area, HTuple* Row, HTuple* Column, double* Angle)
{
	list<Armor>::iterator it;
	if (LOR == 'L')
	{
		/*遍历链表*/
		for (it = Left_ArmorList.begin(); it != Left_ArmorList.end(); ++it)
		{
			if (it->ID == ID)//查找到相同ID
			{
				*Area = it->Area;
				*Row = it->CenterRow;
				*Column = it->CenterColumn;
				*Angle = it->spin_Z_angle;
			}
		}
		return true;
	}
	else if (LOR == 'R')
	{
			/*遍历链表*/
		for (it = Right_ArmorList.begin(); it != Right_ArmorList.end(); ++it)
		 {
			  if (it->ID == ID)//查找到相同ID
			  {
				  *Area = it->Area;
				  *Row = it->CenterRow;
				  *Column = it->CenterColumn;
				  *Angle = it->spin_Z_angle;
			  }
		  }
		  return true;
	}
	else
	{
		cout << "Function GetArmor Input Error..." << endl;
		return false;
	}
}



/*******************************************************************************
Function:TransFCamToCT(double& XC, double& YC, double& ZC, double* XT, double* YT, double* ZT)
Description:从相机坐标变换到云台坐标
Calls:无
Called By:
      Shooting(const HObject & ImageL, const HObject & ImageR) (ArmorDetect)
Input:
      XC,YC,ZC：相机三维坐标
Output:
      XT,YT,ZT：云台三维坐标
Return:无
Others:
********************************************************************************/
void ArmorDetect::TransFCamToCT(double& XC, double& YC, double& ZC, double* XT, double* YT, double* ZT)
{
	HTuple tempX, tempY, tempZ;

	AffineTransPoint3d(SIP.HomCamLToGun, XC, YC, ZC, &tempX, &tempY, &tempZ);

	*XT = (double)tempX;
	*YT = (double)tempY;
	*ZT = (double)tempZ;
}



