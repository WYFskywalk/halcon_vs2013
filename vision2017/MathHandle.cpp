#include "stdafx.h"
#include "mathhandle.h"
#include "VisionInit.h"


MathClass::MathClass()
{
	a = 0.95;
	b = 0.95;
	LastValue = 0.1;
	h_LastValue = 0.1;
}


MathClass::~MathClass()
{
}

/* 左相机列坐标转换为角度*/
void MathClass::HRadiaHandleCL(const HTuple& xx, HTuple* angle)
{
	double temp = xx;
	double theta = (temp - CameraLColumnCenter) / WIDTH;
	*angle = (HTuple)(theta*THETAH);
}

void MathClass::HRadiaHandleCL(const double& xx, double* angle)
{
	double theta = (xx - CameraLColumnCenter) / WIDTH;
	*angle = (HTuple)(theta*THETAH);
}


/* 左相机行坐标转换为角度*/
void MathClass::VRadiaHandleCL(const HTuple& yy, HTuple* angle)
{
	double temp = yy;
	double theta = (temp - CameraLRowCenter) / HIGHT;
	*angle = (HTuple)(theta*THETAV);
}

void MathClass::VRadiaHandleCL(const double& yy, double* angle)
{
	double theta = (yy - CameraLRowCenter) / HIGHT;
	*angle = (HTuple)(theta*THETAV);
}


/* 右相机列坐标转换为角度*/
void MathClass::HRadiaHandleCR(const HTuple& xx, HTuple* angle)
{
	double temp = xx;
	double theta = (temp - CameraRColumnCenter) / WIDTH;
	*angle = (HTuple)(theta*THETAH);
}
void MathClass::HRadiaHandleCR(const double& xx, double* angle)
{
	double theta = (xx - CameraRColumnCenter) / WIDTH;
	*angle = (HTuple)(theta*THETAH);
}

/* 右相机行坐标转换为角度*/
void MathClass::VRadiaHandleCR(const HTuple& yy, HTuple* angle)
{
	double temp = yy;
	double theta = (temp - CameraRRowCenter) / HIGHT;
	*angle = (HTuple)(theta*THETAV);
}
void MathClass::VRadiaHandleCR(const double& yy, double* angle)
{
	double theta = (yy - CameraRRowCenter) / HIGHT;
	*angle = (HTuple)(theta*THETAV);
}

/* 左后相机列坐标转换为角度 */
void MathClass::HRadiaHandleCS(const HTuple& xx, HTuple* angle)
{
	double temp = xx;
	double theta = (WIDTH - temp) / WIDTH;
	*angle = (HTuple)(-theta * 180);
}



/** 3D点距离相机距离 */
void MathClass::DistanceToCamera(const HTuple& X, const HTuple& Y, const HTuple& Z, double* distance)
{
	DTCP.X = X;
	DTCP.Y = Y;
	DTCP.Z = Z;

	*distance = sqrt(DTCP.X*DTCP.X + DTCP.Y*DTCP.Y + DTCP.Z*DTCP.Z);
}


/** 3D点测距 */
void MathClass::Distance3DPp(const HTuple& X1, const HTuple& Y1, const HTuple&Z1,
	const HTuple& X2, const HTuple& Y2, const HTuple& Z2, HTuple* distance)
{
	DPPP.XL = X1;
	DPPP.YL = Y1;
	DPPP.ZL = Z1;
	DPPP.XR = X2;
	DPPP.YR = Y2;
	DPPP.ZR = Z2;
	*distance = sqrt((DPPP.XL - DPPP.XR)*(DPPP.XL - DPPP.XR) + (DPPP.YL - DPPP.YR)*
		(DPPP.YL - DPPP.YR) + (DPPP.ZL - DPPP.ZR)*(DPPP.ZL - DPPP.ZR));
}

void MathClass::Distance3DPp(const double& X1, const double& Y1, const double&Z1,
	const double& X2, const double& Y2, const double& Z2, double* distance)
{
	*distance = sqrt((X1 - X2)*(X1 - X2) + (Y1 - Y2) * (Y1 - Y2) + (Z1 - Z2)*(Z1 - Z2));
}



/** 极坐标转换到直角坐标 */
void MathClass::PolarCor2CartesianCor(const double& Distance, const HTuple& AngleX,
	const HTuple& AngleY, double* X, double* Y, double* Z)
{
	double AngleH = AngleX;
	double AngleV = AngleY;
	AngleH = AngleH * PI / 180;
	AngleV = AngleV * PI / 180;
	*X = Distance*cos(AngleV)*sin(AngleH);
	*Y = Distance*sin(AngleV)*cos(AngleH);
	*Z = Distance*cos(AngleV)*cos(AngleH);
}

/** 直角坐标转换到极坐标 */
void MathClass::CartesianCor2PolarCor(const HTuple& x, const HTuple& y, const HTuple& z,
	HTuple* angleh_input, HTuple* anglev_input, HTuple* distence_input)
{
	CCPCP.X = x;
	CCPCP.Y = y;
	CCPCP.Z = z;

	*angleh_input = (HTuple)atan(CCPCP.X / CCPCP.Z) * 180 / PI;
	*anglev_input = (HTuple)atan(-CCPCP.Y / CCPCP.Z) * 180 / PI;
	*distence_input = (HTuple)sqrt(CCPCP.X * CCPCP.X + CCPCP.Y * CCPCP.Y + CCPCP.Z * CCPCP.Z);
}

/** 直角坐标转换到极坐标 */
void MathClass::CartesianCor2PolarCor(const double& x, const double& y, const double& z,
	double* angleh_input, double* anglev_input, double* distence_input)
{
	*angleh_input = atan(x / z) * 180 / PI;
	*anglev_input = atan(-y / z) * 180 / PI;
	*distence_input = sqrt(x*x + y*y + z*z);
}



/** 低通滤波 */
void MathClass::LowPassFilterFun(HTuple* new_value)
{
	HTuple Output = ((*new_value)*b + h_LastValue*(1 - b));
	h_LastValue = Output;
	*new_value = Output;
}
void MathClass::LowPassFilterFun(double* new_value)
{
	double Output = ((*new_value)*b + h_LastValue*(1 - b));
	h_LastValue = Output;
	*new_value = Output;
}
void MathClass::LowPassFilterFun(float* new_value)
{
	float Output = ((*new_value)*b + h_LastValue*(1 - b));
	h_LastValue = Output;
	*new_value = Output;
}


/** 卡尔曼滤波 */
void MathClass::CalmanFilter::CalmanFilterFun(double* value)
{
	double Z_K = *value;
	double X_P = X_Last + *value;
	double P_Temp = P_Last + Q;
	*value = X_P + Kg*(Z_K - X_P);
	Kg = P_Temp / (P_Temp + R);
	P_Last = (1 - Kg)*P_Temp;

}


/** 二阶低通滤波 */
void MathClass::TwoLeverLowPassFilter::TwoLeverLowPassFilterFun(double* value)
{
	double Output = a*(*value) + b*(Value_Last)+c*(Value_Last_Last);
	Value_Last_Last = Value_Last;
	Value_Last = Output;
	*value = Output;
}









#ifdef I_WONT_USE_IT

/*角度反变换，测试用*/
void FHRadiaHandle(float* xx, float angle)
{
	double temp = (float)angle;
	double lemda = tan(temp*PI / 180) / (2 * tan(THETAH / 2)) + 0.5;
	*xx = (float)(lemda * 1280);
}

void FVRadiaHandle(float* yy, float angle)
{
	double temp = (float)angle;
	double lemda = 1 - tan(temp*PI / 180) / (2 * tan(THETAV / 2)) + 0.5;
	*yy = (float)(lemda * 720);
}

float MathClass::XYPlatToCurve(int column1, int row1, int column2, int row2)
{
	float length;
	//	float lamdax1, lamday1, lamdax2, lamday2;
	float thetax1, thetay1, thetax2, thetay2;
	float betax1, betay1, betax2, betay2;
	float cx1, cy1, cx2, cy2;
	float x1 = (float)column1;
	float y1 = (float)row1;
	float x2 = (float)column2;
	float y2 = (float)row2;
	HRadiaHandle(x1, &betax1);
	HRadiaHandle(y1, &betay1);
	HRadiaHandle(x2, &betax2);
	HRadiaHandle(y2, &betay2);
	//	lamdax1 = column1 / WIDTH;
	//	lamday1 = row1 / HIGHT;
	//	lamdax2 = column2 / WIDTH;
	//	lamday2 = row2 / HIGHT;
	thetax1 = (float)(0.5 + betax1 / THETAH);
	thetay1 = (float)(0.5 + betay1 / THETAV);
	thetax2 = (float)(0.5 + betax2 / THETAH);
	thetay2 = (float)(0.5 + betay2 / THETAV);
	cx1 = thetax1*WIDTH;
	cy1 = thetay1*HIGHT;
	cx2 = thetax2*WIDTH;
	cy2 = thetay2*HIGHT;
	float temp = sqrt((cx1 - cx2)*(cx1 - cx2) + (cy1 - cy2)*(cy1 - cy2));
	length = (float)(156.963 / temp);
	return length;
}


/** 水平角
*
*  返回角度
*/
void MathClass::HRadiaHandle(const HTuple& xx, HTuple* angle)
{
	double temp = xx;
	double lamda = temp / 1280;
	double totalangle = tan(THETAH*PI / 360);
	*angle = (HTuple)(atan((2 * lamda - 1) * totalangle) * 180 / PI);
}


/** 竖直角
*
*  返回角度
*/
void MathClass::VRadiaHandle(const HTuple& yy, HTuple* angle)
{
	double temp = yy;
	double lamda = (720 - temp) / 720;
	double totalangle = tan(PI * THETAV / 360.0);
	*angle = (HTuple)(atan((2 * lamda - 1) * totalangle) * 180 / PI);
}
#endif