#include "VisionInit.h"
#include "SerialPort.h"



/* ���ڶ��� */
extern CSerialPort LSerialPort;


VisionInit::VisionInit()
{}

VisionInit::~VisionInit()
{}

/* �����ʼ������ */
void VisionInit::Init(void)
{
	OpenCameraAll();//�����
	CreatWindow();//��������
	CameraParamInit();//���������ʼ��
}


/******************��˫Ŀ���***********************/
void VisionInit::OpenCameraAll(void)
{
	for (;;)
	{
		try
		{
			CloseAllFramegrabbers();
			OpenFramegrabber("DirectShow", 1, 1, 0, 0, 0, 0, "default", 8, "rgb", -1, "false",
				"[0] mjpg (1920x1080)", "[0] HD USB Camera", 0, -1, &AcqHandleL);
			OpenFramegrabber("DirectShow", 1, 1, 0, 0, 0, 0, "default", 8, "rgb", -1, "false",
				"[0] mjpg (1920x1080)", "[1] HD USB Camera", 0, -1, &AcqHandleR);

			SetFramegrabberParam(AcqHandleL, "exposure", -7);//�ع��
			SetFramegrabberParam(AcqHandleL, "contrast", 64);//�Աȶ�
			SetFramegrabberParam(AcqHandleL, "hue", 0);//ɫ��
			SetFramegrabberParam(AcqHandleL, "saturation", 60);//���Ͷ�
			SetFramegrabberParam(AcqHandleL, "sharpness", 2);//������
			SetFramegrabberParam(AcqHandleL, "gamma", 72);//٤���
			SetFramegrabberParam(AcqHandleL, "brightness", -64);//����
			SetFramegrabberParam(AcqHandleL, "video_gain", 0);//��Ƶ����
			SetFramegrabberParam(AcqHandleL, "frame_rate", 30.0);//֡��

			SetFramegrabberParam(AcqHandleR, "exposure", -7);
			SetFramegrabberParam(AcqHandleR, "contrast", 64);
			SetFramegrabberParam(AcqHandleR, "hue", 0);
			SetFramegrabberParam(AcqHandleR, "saturation", 60);
			SetFramegrabberParam(AcqHandleR, "sharpness", 2);
			SetFramegrabberParam(AcqHandleR, "gamma", 72);
			SetFramegrabberParam(AcqHandleR, "brightness", -64);
			SetFramegrabberParam(AcqHandleR, "video_gain", 0);
			SetFramegrabberParam(AcqHandleR, "frame_rate", 30.0);
			break;
		}
		catch (...){
			continue;
		}
	}
}
/*****************��ʼ���������**********************/
void VisionInit::CameraParamInit(void)
{
	double ParamL[] = CAMERAL;
	double ParamR[] = CAMERAR;
	double RealPose[] = REALPOSE;
	ReadCamParamL(ParamL);
	ReadCamParamR(ParamR);
	ReadRealPose(RealPose);
	CamParamOutL = CamParamL;
	CamParamOutL[1] = 0;
	CamParamOutR = CamParamR;
	CamParamOutR[1] = 0;
}

/*****************����ͼ�񴰿�************************/
void VisionInit::CreatWindow(void)
{
	HTuple  Width, Hight;
	HObject ImageL, ImageR;

	GrabImage(&ImageL, AcqHandleL);
	GetImageSize(ImageL, &Width, &Hight);
	GrabImage(&ImageR, AcqHandleR);

	SetWindowAttr("background_color", "black");
	OpenWindow(0, 0, Width / 2, Hight / 2, 0, "", "", &WindowHandleL);
	DispImage(ImageL, WindowHandleL);
	HDevWindowStack::Push(WindowHandleL);

	SetWindowAttr("background_color", "black");
	OpenWindow(0, 0, Width / 2, Hight / 2, 0, "", "", &WindowHandleR);
	DispImage(ImageR, WindowHandleR);
	HDevWindowStack::Push(WindowHandleR);
}

/*******************��ȡ���������*******************/
void VisionInit::ReadCamParamL(double* InputCamParamL)
{
	CamParamL.Clear();
	for (int i = 0; i < 12; i++)
	{
		CamParamL[i] = InputCamParamL[i];
	}
}

/*******************��ȡ���������********************/
void VisionInit::ReadCamParamR(double* InputCamParamR)
{
	CamParamR.Clear();
	for (int i = 0; i < 12; i++)
	{
		CamParamR[i] = InputCamParamR[i];
	}
}

/******************��ȡ������λ�ù�ϵ****************/
void VisionInit::ReadRealPose(double* InputRelPose)
{
	RelPose.Clear();
	for (int i = 0; i < 7; i++)
	{
		RelPose[i] = InputRelPose[i];
	}
}

/*******************�����Ӳ�����**********************/
bool VisionInit::CameraLTest(void)
{
	//LSerialPort.SendData(0, 0, 0, 0, 0, MinPCCameraError);
	for (;;)
	{
		try{
			CloseFramegrabber(AcqHandleL);
			OpenFramegrabber("DirectShow", 1, 1, 0, 0, 0, 0, "default", 8, "rgb", -1, "false",
				"[1] mjpg (1920x1080)", "[0] HD USB Camera", 0, -1, &AcqHandleL);
			SetFramegrabberParam(AcqHandleL, "gamma", 72);
			SetFramegrabberParam(AcqHandleL, "exposure", -7);
			cout << "CameraL Normal..." << endl;
			break;
		}
		catch (...){
			cout << "CameraL Error..." << endl;
			continue;
		}
		break;
	}
	return 1;
}

/*******************�����Ӳ�����**********************/
bool VisionInit::CameraRTest(void)
{
	//LSerialPort.SendData(0, 0, 0, 0, 0, MinPCCameraError);
	for (;;)
	{
		try{
			CloseFramegrabber(AcqHandleR);
			OpenFramegrabber("DirectShow", 1, 1, 0, 0, 0, 0, "default", 8, "rgb", -1, "false",
				"[1] mjpg (1920x1080)", "[1] HD USB Camera", 0, -1, &AcqHandleR);
			SetFramegrabberParam(AcqHandleR, "gamma", 72);
			SetFramegrabberParam(AcqHandleR, "exposure", -7);
			cout << "CameraR Normal..." << endl;
			break;
		}
		catch (...){
			cout << "CameraR Error..." << endl;
			continue;
		}
		break;
	}
	return 1;
}



/*****************�������ģʽ�������******************/
void VisionInit::SetCamShoot(void)
{
	SetFramegrabberParam(AcqHandleL, "brightness", -64);
	SetFramegrabberParam(AcqHandleL, "contrast", 54);
	SetFramegrabberParam(AcqHandleL, "hue", 0);
	SetFramegrabberParam(AcqHandleL, "saturation", 128);
	SetFramegrabberParam(AcqHandleL, "sharpness", 0);
	SetFramegrabberParam(AcqHandleL, "gamma", 72);
	SetFramegrabberParam(AcqHandleL, "video_gain", 0);
	SetFramegrabberParam(AcqHandleL, "exposure", -7);

	SetFramegrabberParam(AcqHandleR, "brightness", -64);
	SetFramegrabberParam(AcqHandleR, "contrast", 54);
	SetFramegrabberParam(AcqHandleR, "hue", 0);
	SetFramegrabberParam(AcqHandleR, "saturation", 128);
	SetFramegrabberParam(AcqHandleR, "sharpness", 0);
	SetFramegrabberParam(AcqHandleR, "gamma", 72);
	SetFramegrabberParam(AcqHandleR, "video_gain", 0);
	SetFramegrabberParam(AcqHandleR, "exposure", -7);
}


/*****************���ģʽ�������������*****************/
void VisionInit::SetCamMark(void)
{
	SetFramegrabberParam(AcqHandleL, "exposure", -7);
	SetFramegrabberParam(AcqHandleL, "contrast", 64);
	SetFramegrabberParam(AcqHandleL, "hue", 0);
	SetFramegrabberParam(AcqHandleL, "saturation", 60);
	SetFramegrabberParam(AcqHandleL, "sharpness", 2);
	SetFramegrabberParam(AcqHandleL, "gamma", 72);
	SetFramegrabberParam(AcqHandleL, "brightness", -20);
	SetFramegrabberParam(AcqHandleL, "video_gain", 0);
	SetFramegrabberParam(AcqHandleL, "frame_rate", 30.0);

	SetFramegrabberParam(AcqHandleR, "exposure", -7);
	SetFramegrabberParam(AcqHandleR, "contrast", 64);
	SetFramegrabberParam(AcqHandleR, "hue", 0);
	SetFramegrabberParam(AcqHandleR, "saturation", 60);
	SetFramegrabberParam(AcqHandleR, "sharpness", 2);
	SetFramegrabberParam(AcqHandleR, "gamma", 72);
	SetFramegrabberParam(AcqHandleR, "brightness", -20);
	SetFramegrabberParam(AcqHandleR, "video_gain", 0);
	SetFramegrabberParam(AcqHandleR, "frame_rate", 30.0);
}
