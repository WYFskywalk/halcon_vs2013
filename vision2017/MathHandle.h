#ifndef MATHHANDLE_H
#define MATHHANDLE_H


/** ˮƽ�ӽ� */
#define THETAH 90

/** ��ֱ�ӽ� */
#define THETAV 60

/** ˮƽ���� */
#define WIDTH 1280

/** ��ֱ���� */
#define HIGHT 720



class MathClass
{
public:
	MathClass();
	~MathClass();


private:
	/* DistanceToCamera���������ṹ�� */
	typedef struct FunDistanceToCameraParam
	{
		double X;
		double Y;
		double Z;
	}FunDistanceToCameraParam;

	FunDistanceToCameraParam DTCP;


	/* Distance3DPp���������ṹ�� */
	typedef struct FunDistance3DPpParam
	{
		double XL;
		double YL;
		double ZL;
		double XR;
		double YR;
		double ZR;
	}FunDistance3DPpParam;

	FunDistance3DPpParam DPPP;


	/* CartesianCor2PolarCor���������ṹ�� */
	typedef struct FunCartesianCor2PolarCorParam
	{
		double X;
		double Y;
		double Z;
	}FunCartesianCor2PolarCorParam;

	FunCartesianCor2PolarCorParam CCPCP;


public:

	/** �����������ת��Ϊ�Ƕ� */
	void HRadiaHandleCL(const HTuple& xx, HTuple* angle);
	void HRadiaHandleCL(const double& xx, double* angle);


	/** �����������ת��Ϊ�Ƕ� */
	void VRadiaHandleCL(const HTuple& yy, HTuple* angle);
	void VRadiaHandleCL(const double& yy, double* angle);


	/** �����������ת��Ϊ�Ƕ� */
	void HRadiaHandleCR(const HTuple& xx, HTuple* angle);
	void HRadiaHandleCR(const double& xx, double* angle);


	/** �����������ת��Ϊ�Ƕ� */
	void VRadiaHandleCR(const HTuple& yy, HTuple* angle);
	void VRadiaHandleCR(const double& yy, double* angle);


	/** �����������ת��Ϊ�Ƕ� */
	void HRadiaHandleCS(const HTuple& xx, HTuple* angle);
	void HRadiaHandleCS(const double& xx, double* angle);


	/** �����ԭ����� */
	void DistanceToCamera(const HTuple& X, const HTuple& Y, const HTuple& Z, double* distance);


	/** ������3D��֮����� */
	void Distance3DPp(const HTuple& X1, const HTuple& Y1, const HTuple&Z1,
		const HTuple& X2, const HTuple& Y2, const HTuple& Z2, HTuple* distance);
	void Distance3DPp(const double& X1, const double& Y1, const double&Z1,
		const double& X2, const double& Y2, const double& Z2, double* distance);

	/** ������תֱ������ */
	void PolarCor2CartesianCor(const double& Distance, const HTuple& AngleX,
		const HTuple& AngleY, double* X, double* Y, double* Z);


	/** ֱ������ת������ */
	void CartesianCor2PolarCor(const HTuple& x, const HTuple& y, const HTuple& z,
		HTuple* angleh_input, HTuple* anglev_input, HTuple* distence_input);

	void CartesianCor2PolarCor(const double& x, const double& y, const double& z,
		double* angleh_input, double* anglev_input, double* distence_input);


	/** ��������תͼ������ */
	float XYPlatToCurve(int column1, int row1, int column2, int row2);



	///** ��ͨ�˲� */
	//class LowPassFilter
	//{
	//public:
	double a;
	HTuple b;
	double LastValue;
	HTuple h_LastValue;

	void LowPassFilterFun(double* new_value);
	void LowPassFilterFun(float* new_value);
	void LowPassFilterFun(HTuple* new_value);

	//}LowPassFilter;

	/** �������˲� */
	class CalmanFilter
	{
	public:
		double Q = 0;
		double R = 0;

		double X_Last = 0;
		double P_Last = 0;
		double Kg = 0;

		void CalmanFilterFun(double* value);

	}CalmanFilter;

	/** ���׵�ͨ�˲� */
	class TwoLeverLowPassFilter
	{
	public:
		double a = 0.2;
		double b = 0.3;
		double c = 0.5;
		double Value_Last = 0;
		double Value_Last_Last = 0;

		void TwoLeverLowPassFilterFun(double* value);
	}TwoLeverLowPassFilter;

};


#endif