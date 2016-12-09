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
Description:�����ʼ��
Calls:��
Called By:MainThread(void)
Input:��
Output:��
Return:��
Others:
********************************************************************************/
void ArmorDetect::ShootInit(void)
{
	/*�����������������ת������*/
	PoseToHomMat3d(LInit.RelPose, &SIP.RToL3D);
	/*�����������������ת������*/
	HomMat3dInvert(SIP.RToL3D, &SIP.LToR3D);

	/*�������������̨��ת���������ֶ�����*/
	SIP.PoseCamL.Clear();
	SIP.PoseCamL[0] = -0.185;
	SIP.PoseCamL[1] = 0.085;
	SIP.PoseCamL[2] = 0.16;
	SIP.PoseCamL[3] = 0;
	SIP.PoseCamL[4] = 0;
	SIP.PoseCamL[5] = 0;
	SIP.PoseCamL[6] = 0;

	/*�õ����������̨ת������*/
	PoseToHomMat3d(SIP.PoseCamL, &SIP.HomCamLToGun);

}



/*******************************************************************************
Function:Shooting(const HObject &ImageL, const HObject &ImageR)
Description:���߳��������
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
      ImageL�������ͼ��
	  ImageR�������ͼ��
Output:��
Return:��
Others:װ�װ����������
********************************************************************************/
void ArmorDetect::Shooting(const HObject &ImageL, const HObject &ImageR)
{
	/*�������*/
	ListUpdate();
	/**************************Ŀ���⿪ʼ***************************/
	if (EnemyDistingguish(ImageL, "L") && EnemyDistingguish(ImageR, "R"))
	{
		/*��Ŀ����*/
		try
		{
			SingleCameraForCol('L');
			SingleCameraForCol('R');
		}
		catch (...)
		{
			cout << "��Ŀ��������..." << endl;
		}

		/*ɾ�������С��װ��*/
		SelectArmor();
		/*�������Ŀ��ƥ��*/
		if (RegionMatch())
		{
			/* Ŀ�����&&����洢 */
			if (!BinaryMeasure())
			{
				cout << "Binary Measure Error..." << endl;
			}
		}

		/*ѡ��������Ŀ��*/
		if (SelectObject())
		{
			/* Ŀ������ת�� */
			TransFCamToCT(ShootingEnemy.X, ShootingEnemy.Y, ShootingEnemy.Z, &CGV.XGP, &CGV.YGP, &CGV.ZGP);
			EnemyID = Shoot;
		}
		else
		{
			EnemyID = PauseShoot;
		}
	/**************************Ŀ�������***************************/
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
			cout << "���߳���ʱ��" << time << endl;

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
	/*�����ô��룬��ע��*/
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
Description:װ�װ��⣬��LEDList��Armor�ṹ��д������
Calls:
       LampMatch(string Single)
Called By:
       Shooting(const HObject &ImageL, const HObject &ImageR)
Input:
       Image�������ͼ��
	   LOR�����ұ�ʶ����L�����������R���������
Output:��
Return:
       ʶ��ɹ�����TRUE�����򷵻�FALSE
Others:����ʶ���㷨˵���������ڲ�,��LEDList��Armor�ṹ��д������
       ��Ҫ���£�
	   2016/12/8 �޸ľ����Ժ����ֵ��������ϵ�����������ʹ���
********************************************************************************/
bool ArmorDetect::EnemyDistingguish(const HObject& Image, string LOR)
{
	Decompose3(Image, &EDP.Red, &EDP.Green, &EDP.Blue);
	Rgb3ToGray(EDP.Red, EDP.Green, EDP.Blue, &EDP.ImageGray);
	/*�㷨˼·�����������Ϊ�������֣�һ�������ĸ������֣���һ�����ܱ���ɫ��������
	*ͨ����ֵ�������ɸѡ��ѡ�������������ͣ�����SubImage��intersec���ӵõ��ܱߵ�ɫ�ⲿ������
	*��������ȡ������õ�Ŀ�����������ɫ�������ٽ������ͣ���Բ��ϣ����ڽ�һ��ɸѡ
	*/
	if (ROB == BLUE)//��ɫ
	{
		/*��ְ׹�����*/
		Threshold(EDP.ImageGray, &EDP.ImageGrayRegion, 180, 255);
		Connection(EDP.ImageGrayRegion, &EDP.ImageGrayConnectedRegion);
		DilationCircle(EDP.ImageGrayConnectedRegion, &EDP.ImageGrayDilation,1.5);
		SelectShape(EDP.ImageGrayDilation, &EDP.ImageGraySelectedRegion, "area", "and", 40, 6000);
		//DilationCircle(EDP.ImageGraySelectedRegion, &EDP.ImageGrayRegionDilation, 1.5);

		/*�����ɫ�߷�������*/
		SubImage(EDP.Blue, EDP.Red, &EDP.ImageTemp1,1.0,0);
		SubImage(EDP.Blue, EDP.Green, &EDP.ImageTemp2, 1.0, 0);
		Threshold(EDP.ImageTemp1, &EDP.ImageTempRegion1, 30, 255);
		Threshold(EDP.ImageTemp2, &EDP.ImageTempRegion2, 30, 255);
		Intersection(EDP.ImageTempRegion1, EDP.ImageTempRegion2, &EDP.ImageIntersections);
		DilationCircle(EDP.ImageIntersections, &EDP.ImageIntersectionsDilation, 2.5);
		FillUp(EDP.ImageIntersectionsDilation,&EDP.ImageIntersectionsFillUp);

		/*ȡ��������ȡ������Ե��ɫ����*/
		Intersection(EDP.ImageGrayDilation, EDP.ImageIntersectionsFillUp, &EDP.LightEdge);
		//DilationCircle(EDP.LightEdge, &EDP.LightEdgeDilation,3.5);
        
		/*�ڶ���ɸѡ�������������*/
		Connection(EDP.LightEdge, &EDP.LightEdgeConnectedRegions);
		SelectShape(EDP.LightEdgeConnectedRegions, &EDP.LightEdgeSelectedRegion, "area", "and", 20, 6000);
	}
	else if (ROB == RED)//��ɫ
	{
		/*��ְ׹�����*/
		Threshold(EDP.ImageGray, &EDP.ImageGrayRegion, 180, 255);
		Connection(EDP.ImageGrayRegion, &EDP.ImageGrayConnectedRegion);
		DilationCircle(EDP.ImageGrayConnectedRegion, &EDP.ImageGrayDilation, 1.5);
		SelectShape(EDP.ImageGrayDilation, &EDP.ImageGraySelectedRegion, "area", "and", 40, 6000);
		//DilationCircle(EDP.ImageGraySelectedRegion, &EDP.ImageGrayRegionDilation, 1.5);

		/*�����ɫ�߷�������*/
		SubImage(EDP.Red, EDP.Blue, &EDP.ImageTemp1, 1.0, 0);
		SubImage(EDP.Red, EDP.Green, &EDP.ImageTemp2, 1.0, 0);
		Threshold(EDP.ImageTemp1, &EDP.ImageTempRegion1, 30, 255);
		Threshold(EDP.ImageTemp2, &EDP.ImageTempRegion2, 30, 255);
		Intersection(EDP.ImageTempRegion1, EDP.ImageTempRegion2, &EDP.ImageIntersections);
		DilationCircle(EDP.ImageIntersections, &EDP.ImageIntersectionsDilation, 2.5);
		FillUp(EDP.ImageIntersectionsDilation, &EDP.ImageIntersectionsFillUp);

		/*ȡ��������ȡ������Ե��ɫ����*/
		Intersection(EDP.ImageGrayDilation, EDP.ImageIntersectionsFillUp, &EDP.LightEdge);
		//DilationCircle(EDP.LightEdge, &EDP.LightEdgeDilation,3.5);

		/*�ڶ���ɸѡ�������������*/
		Connection(EDP.LightEdge, &EDP.LightEdgeConnectedRegions);
		SelectShape(EDP.LightEdgeConnectedRegions, &EDP.LightEdgeSelectedRegion, "area", "and", 20, 6000);	
	}

	CountObj(EDP.LightEdgeSelectedRegion, &EDP.Num);

	/*************************��ʾ�����������ã���ע��**********************/
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

	/*********************************������̬ɸѡ**********************************/
	for (int i = 0; i != EDP.Num; ++i)
	{
		SelectObj(EDP.LightEdgeSelectedRegion, &EDP.LedRegion, i + 1);//ѡ�д���������
		//AreaCenter(EDP.LedRegion, &EDP.Area, &EDP.Row, &EDP.Column);//��ȡ��������ĵ�����
					
			try
			{
				ShapeTrans(EDP.LedRegion, &EDP.LightShapeTrans,"ellipse");
				AreaCenter(EDP.LightShapeTrans,&EDP.RegionArea,&EDP.RegionRow,&EDP.RegionColumn);
				GenContourRegionXld(EDP.LedRegion, &EDP.Contours, "border");
   				//ShapeTransXld(EDP.Contours, &EDP.XLDtrans, "ellipse");
				//AreaCenterXld(EDP.XLDtrans, &EDP.XldArea, &EDP.XldRow, &EDP.XldColumn, &EDP.XldPointOrder);
				FitEllipseContourXld(EDP.Contours, "fitzgibbon", -1, 0, 0, 200, 3, 2, &EDP.Row,
					&EDP.Column, &EDP.Phi, &EDP.Radius1, &EDP.Radius2, &EDP.StartPhi, &EDP.EndPhi,
					&EDP.PointOrder);//XLD��Բ���				
				//cout << abs(1.57 - (double)EDP.Phi) << "  " << (double)(EDP.Radius1 / EDP.Radius2) << endl;
				/*����ǣ���Բ��̬,��Բ����ж�*/
				if ((abs(1.57 - (double)EDP.Phi) < PHI_THRESHOLD)
					&& (((double)(EDP.Radius1 / EDP.Radius2)) > RA_RATIO_RB)
					//&&EDP.XldArea<=XLD_MAX_AREA
					//&&EDP.XldArea>=XLD_MIN_AREA
					&& EDP.RegionArea <= REGION_MAX_AREA
					&& EDP.RegionArea >= REGION_MIN_AREA
					)
				{
					ConcatObj(FUCK, EDP.LedRegion, &FUCK);
					/*����д��*/
					EDP.LEDtemp.Row = (double)EDP.Row;
					EDP.LEDtemp.Column = (double)EDP.Column;
					EDP.LEDtemp.Phi = (double)EDP.Phi;
					EDP.LEDtemp.Radius1 = (double)EDP.Radius1;
					EDP.LEDtemp.Radius2 = (double)EDP.Radius2;
					EDP.LEDtemp.Length = (double)(EDP.Radius1 * 2);
					EDP.LEDtemp.Area = (double)EDP.RegionArea;
					EDP.LEDtemp.High = (EDP.Radius1 > EDP.Radius2 ? EDP.Radius1 : EDP.Radius2) * 2 *
						sin((double)EDP.Phi);
					/*���浽����*/
					LEDList.push_back(EDP.LEDtemp);
				}
			 }
			catch (...)
			{
				continue;
			}
	}
	/****************************��ʾɸѡ���***********************************/
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

	/************************************ƥ��**********************************/
	if (LOR == "L")
		LampMatch("L");
	else if (LOR == "R")
		LampMatch("R");
	else
	{
		cout << "Function EnemyDistingguish: Param Error" << endl;
		return false;
	}
	LEDList.clear();//����б�
	/*************************���㲢����װ�װ嶥����Ϣ**************************/

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

			/* �ж����ҵ����Ƿ�㷴 */
			if ((EDP.tempL->Columna) >= (EDP.tempL->Columnb))
			{//������ҵ����㷴��a��b������c��d����
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
			/* �ж����ҵ����Ƿ�㷴 */
			if ((EDP.tempR->Columna) >= (EDP.tempR->Columnb))
			{//������ҵ����㷴��a��b������c��d����
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
Description:����ƥ��װ�ף�
Calls:��
Called By:
      EnemyDistingguish(const HObject& Image, string LOR)
Input:
      Single��L��ʾ�����װ������R��ʾ�����װ������
Output:��
Return:��
Others:˫�ر����������Զ�����������ƥ�����.��Armorд�����ҵ����������̬����
       ��Ҫ���£�
	   2016/12/8 ���ݲ���ȷ���������ϵ��
********************************************************************************/
void ArmorDetect::LampMatch(string Single)
{
	/*******************************˫��ѭ��*************************************/
	LMP.CompareError = 500;
	LMP.TempError = 500;
	/*û���㹻�ĵ���ƥ�䣬ֱ�ӷ���*/
	if (LEDList.size() < 2)
	{
		return;
	}
	/**********************�����ѭ����ʼ******************************/
	for (LMP.LEDOne = LEDList.begin(); LMP.LEDOne != LEDList.end(); ++LMP.LEDOne)
	{
		/**********************�ҵ���ѭ����ʼ******************************/
		for (LMP.LEDTwo = ++LMP.LEDOne; LMP.LEDTwo != LEDList.end(); ++LMP.LEDTwo)
		{
			/*������*/
			LMP.TempErrorLength = abs(LMP.LEDOne->Length - LMP.LEDTwo->Length)* LENGTH_WEIGHT; //������������̶�20��Ȩ��20*1=20
			LMP.TempErrorPhi = abs(LMP.LEDOne->Phi - LMP.LEDTwo->Phi) * PHI_WEIGHT;   //�Ƕ���������̶�0.12��Ȩ��0.12*333=40
			LMP.TempErrorRow = abs(LMP.LEDOne->Row - LMP.LEDTwo->Row)  * ROW_WEIGHT;  //�߶���������̶�15��Ȩ��15*2.67=40
			LMP.TempError = LMP.TempErrorLength + LMP.TempErrorPhi + LMP.TempErrorRow;
			cout << "�������" << abs(LMP.LEDOne->Length - LMP.LEDTwo->Length)* LENGTH_WEIGHT << endl;
			cout << "�Ƕ����" << abs(LMP.LEDOne->Phi - LMP.LEDTwo->Phi) * PHI_WEIGHT << endl;
			cout << "ˮƽ�������" << abs(LMP.LEDOne->Row - LMP.LEDTwo->Row) / (LMP.LEDOne->Row + LMP.LEDTwo->Row) * ROW_WEIGHT<< endl;
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
			{//��������С��
				continue;
			}			
		/**************************�ҵ���ѭ�����*********************************/
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
	/*************************�����ѭ�����*********************************/
	}

}



/*******************************************************************************
Function:SingleCameraForCol(char Single)
Description:���������Ŀ��3D���꣬��ȡװ�װ����ĵ�����
Calls:
      MathClass::PolarCor2CartesianCor(const double& Distance, const HTuple& AngleX,
	       const HTuple& AngleY, double* X, double* Y, double* Z)
      MathClass::VRadiaHandleCL(const double& yy, double* angle)

Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:
       Single��L��ʾ�����װ������R��ʾ�����װ������
Output:��
Return:��
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
				/* �ɶ���������������� */
				GenRegionPolygon(&SCFCP.Region, SCFCP.it->Rows, SCFCP.it->Columns);
				/* �������ĵ� */
				AreaCenter(SCFCP.Region, &SCFCP.Area, &SCFCP.Row, &SCFCP.Column);
			}
			catch (...)
			{
				cout << "GenRegionPolygon Error��" << endl;
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
				/* �ɶ���������������� */
				GenRegionPolygon(&SCFCP.Region, SCFCP.it->Rows, SCFCP.it->Columns);
				/* �������ĵ� */
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
Description:ɾ�������С��װ��
Calls:��
Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:��
Output:��
Return:��
Others:
       ��Ҫ���£�
	   2016/11/18 ����װ�׳���ȳߴ�Լ����������ĳЩ����Ͻ��������װ����ƥ���BUG
********************************************************************************/
void ArmorDetect::SelectArmor(void)
{
	/*���������*/
	list<Armor>::iterator it;
	/*װ�װ峤����*/
	double ArmorWidth, ArmorHigh;
	double Rate;
	for (it = Left_ArmorList.begin(); it != Left_ArmorList.end();)
	{
		/*����װ�׳����*/
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
		/*����װ�׳����*/
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
Description:�������ƥ��
Calls:��
Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:��
Output:��
Return:ƥ��ɹ�����TRUE,���򷵻�FALSE
Others:
********************************************************************************/
bool ArmorDetect::RegionMatch(void)
{
	Left_Match.clear();
	Right_Match.clear();
    /********************************�����ѭ����ʼ***********************************/
	for (RMP.tempL = Left_ArmorList.begin(); RMP.tempL != Left_ArmorList.end(); ++RMP.tempL)
	{
		if (RMP.tempL->FlagForSingleCamMeasure == 1)
		{
			continue;
		}
		try
		{		
			/* ������任��������� */
			AffineTransPoint3d(SIP.LToR3D, RMP.tempL->SCX, RMP.tempL->SCY, RMP.tempL->SCZ, &RMP.XRTL, &RMP.YRTL, &RMP.ZRTL);
		}
		catch (...)
		{
			cout << "AffineTransPoint3d Error��" << endl;
			continue;
		}
		RMP.MinDisXX = 1000;
		/**********************************�����ѭ����ʼ*******************************/
		for (RMP.tempR = Right_ArmorList.begin(); RMP.tempR != Right_ArmorList.end(); ++RMP.tempR)
		{
			if (RMP.tempR->FlagForSingleCamMeasure == 1)
			{
				continue;
			}

			/** һ����ȡ���ֱ��Χ�� */
			if ((abs((double)RMP.YRTL - RMP.tempR->SCY) < X_DEEP_THRESHOLD) && (abs((double)RMP.ZRTL - RMP.tempR->SCZ) < Z_HIGH_THRESHOLD))
			{
				/* ����ˮƽ������ */
				RMP.tempDis = abs((double)RMP.XRTL - (RMP.tempR->SCX));
				if (RMP.tempDis < RMP.MinDisXX)
				{//Ŀ����С��
					RMP.MinDisXX = RMP.tempDis;
					RMP.IDR = RMP.tempR->ID;
				}
			}
		/***********************************�����ѭ������*******************************/
		}
		if (RMP.MinDisXX < MAXMATCHPP)
		{//���Ŀ������Ҫ��
			/* �洢�������ID */
			int IDtemp = RMP.tempL->ID;
			Left_Match.push_back(IDtemp);
			Right_Match.push_back(RMP.IDR);
			//cout << RMP.tempL->ID << "  " << RMP.IDR << endl;
		}
	/********************************�����ѭ������***********************************/
	}

	if ((!Left_Match.empty()) && (!Right_Match.empty()))
		return true;
	else
		return false;

}



/*******************************************************************************
Function:RegionMatch(void)
Description:˫Ŀ����
Calls:��
Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:��
Output:��
Return:�����ɹ�����TRUE,���򷵻�FALSE
Others:���ݱ��浽ArmorMatched��������
********************************************************************************/
bool ArmorDetect::BinaryMeasure(void)
{
	/*�����ж�*/
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
	/*����ѭ��*/
	for (BMP.IDL = Left_Match.begin(), BMP.IDR = Right_Match.begin(); BMP.IDL != Left_Match.end(); ++BMP.IDL, ++BMP.IDR)
	{
		if (GetArmor('L', *BMP.IDL, &BMP.AreaL, &BMP.RowL, &BMP.ColumnL, &BMP.AngleL) && GetArmor('R', *BMP.IDR, &BMP.AreaR, &BMP.RowR, &BMP.ColumnR, &BMP.AngleR))
		{
			try
			{
				/*˫Ŀ�������3D����*/
				IntersectLinesOfSight(LInit.CamParamL, LInit.CamParamR, LInit.RelPose, 
					BMP.RowL, BMP.ColumnL, BMP.RowR, BMP.ColumnR,
					&BMP.X, &BMP.Y, &BMP.Z, &BMP.Dist);
			}
			catch (...)
			{
				cout << "Binary Measure Error: Count Three-Dimensional Coordinates Error..." << endl;
				continue;
			}
			/*�������ݵ�װ������*/
			SaveData(BMP.X, BMP.Y, BMP.Z, BMP.AngleL, BMP.AreaL, BMP.RowL, BMP.ColumnL);

			/*����Ƿ�����������β����������ѭ��������ֱ�ӷ���*/
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
Description:ѡ�����Ŀ��
Calls:��
Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:��
Output:��
Return:���ҵ����Ŀ���Ҿ���С����ֵʱ����TRUE,���򷵻�FALSE
Others:�����2�����ִ�У�1�����ϴ��ҵ����Ŀ�꣬��һ�����ϴ�δ�ҵ����Ŀ�꣬ͨ��static������ʶ
********************************************************************************/
bool ArmorDetect::SelectObject(void)
{
	vector<ArmorMatched>::iterator it;
	/*��־λ�����ڱ���ϴ��Ƿ��������Ŀ��.0:�ϴ�δʶ��Ŀ�� 1���ϴ�ʶ��Ŀ��*/
	static int LastEnemyFlag = 0;
	//static double LastDistance;//�ϴ��������
	double ShortestDistance = 1000.0;//�복������������
	double DistanceBetweenEnemys ;//��ǰ����Ŀ������ϴ����Ŀ��ľ��루��ʱ������
	double ShortestDistanceEnemys = 1000.0;//��ǰ����Ŀ������ϴ����Ŀ������ľ���
	
	/************�״ε��ú��ϴε���δʶ��װ�װ�ʱִ���������**************/
	if (!LastEnemyFlag)
	{
		for (it = ArmorList.begin(); it != ArmorList.end(); ++it)
		{
			/*Ѱ�Ҿ��복�������Ŀ��*/
			if (it->Distance < ShortestDistance)
			{
				ShortestDistance = it->Distance;
				EasistEnemy = *it;
			}
		}
		
		/*����̫Զ��ֱ�ӷ���false*/
		if (EasistEnemy.Distance > 5)
		{
			return false;
		}
		
		/*�ҵ����Ŀ��*/
		else
		{
			LastEnemyFlag = 1;//��־λ��λ
			LastEnemy = EasistEnemy;
			ShootingEnemy= EasistEnemy;//���ƽṹ��
			return true;
		}
	}

	/********************�ϴε���ʶ��Ŀ��ִ�����д���***********************/
	for (it = ArmorList.begin(); it != ArmorList.end(); ++it)
	{
		/*Ѱ�Ҿ��복��������Ŀ��*/
		if (it->Distance < ShortestDistance)
		{
			ShortestDistance = it->Distance;
			EasistEnemy = *it;
		}
		
		/*Ѱ�Ҿ�����һ�����Ŀ�������Ŀ��*/
		LMathClass.Distance3DPp(it->X, it->Y, it->Z, LastEnemy.X, LastEnemy.Y, LastEnemy.Z, &DistanceBetweenEnemys);
		if (DistanceBetweenEnemys < ShortestDistanceEnemys)
		{
			ShortestDistanceEnemys = DistanceBetweenEnemys;
			TempEnemy = *it;
		}
	}
	
	/*���TempEnemy��EasistEnemy��ֵ������0.5�ף�ѡ��TempEnemyΪShootingEnemy������ѡ��EasistEnemy*/
	if ((TempEnemy.Distance - EasistEnemy.Distance) < 0.5)
	{
		ShootingEnemy = TempEnemy;
	}
	else
	{
		ShootingEnemy = EasistEnemy;
	}
	
	/*�ж��������,�����������ձ�־λ*/
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
Description:������£���ո�������
Calls:��
Called By:
      Shooting(const HObject &ImageL, const HObject &ImageR)
Input:��
Output:��
Return:��
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
Description:�õ�һ���µĵư�ID
Calls:��
Called By:
      EnemyDistingguish(const HObject & Image, string LOR) (ArmorDetect)
Input:
      LOR:L��ʾ���������R��ʾ���������
Output:��
Return:�ư�ID
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
Description:����ƥ��õ�װ����Ϣ�����ꡢ���롢�Ƕȵ���Ϣ
Calls:��
Called By:
      BinaryMeasure(void) 
Input:
      X,Y,Z,angle,Area,RowL, ColumnL:�����������
Output:��
Return:��
Others:����д�뵽ArmorList��������
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
Description:���װ����Ϣ
Calls:��
Called By:
      BinaryMeasure(void) 
Input:
      LOR�����ұ�־λ��L��ʾ���������R��ʾ���������
Output:
      Area��Row��Column��Column��Angle��װ�������Ϣ
Return:��
Others:�����ַǷ�LOR����ʱ����FALSE,���෵��TRUE
********************************************************************************/
bool ArmorDetect::GetArmor(char LOR, int ID, double* Area, HTuple* Row, HTuple* Column, double* Angle)
{
	list<Armor>::iterator it;
	if (LOR == 'L')
	{
		/*��������*/
		for (it = Left_ArmorList.begin(); it != Left_ArmorList.end(); ++it)
		{
			if (it->ID == ID)//���ҵ���ͬID
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
			/*��������*/
		for (it = Right_ArmorList.begin(); it != Right_ArmorList.end(); ++it)
		 {
			  if (it->ID == ID)//���ҵ���ͬID
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
Description:���������任����̨����
Calls:��
Called By:
      Shooting(const HObject & ImageL, const HObject & ImageR) (ArmorDetect)
Input:
      XC,YC,ZC�������ά����
Output:
      XT,YT,ZT����̨��ά����
Return:��
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



