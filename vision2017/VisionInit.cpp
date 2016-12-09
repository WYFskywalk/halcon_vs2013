#include "VisionInit.h"
#include "SerialPort.h"



/* 串口对象 */
extern CSerialPort LSerialPort;


VisionInit::VisionInit()
{}

VisionInit::~VisionInit()
{}

/* 相机初始化函数 */
void VisionInit::Init(void)
{
	OpenCameraAll();//打开相机
	CreatWindow();//创建窗口
	CameraParamInit();//相机参数初始化
}


/******************打开双目相机***********************/
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

			SetFramegrabberParam(AcqHandleL, "exposure", -7);//曝光度
			SetFramegrabberParam(AcqHandleL, "contrast", 64);//对比度
			SetFramegrabberParam(AcqHandleL, "hue", 0);//色调
			SetFramegrabberParam(AcqHandleL, "saturation", 60);//饱和度
			SetFramegrabberParam(AcqHandleL, "sharpness", 2);//锐利度
			SetFramegrabberParam(AcqHandleL, "gamma", 72);//伽马度
			SetFramegrabberParam(AcqHandleL, "brightness", -64);//亮度
			SetFramegrabberParam(AcqHandleL, "video_gain", 0);//视频增益
			SetFramegrabberParam(AcqHandleL, "frame_rate", 30.0);//帧率

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
/*****************初始化相机参数**********************/
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

/*****************创建图像窗口************************/
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

/*******************读取左相机参数*******************/
void VisionInit::ReadCamParamL(double* InputCamParamL)
{
	CamParamL.Clear();
	for (int i = 0; i < 12; i++)
	{
		CamParamL[i] = InputCamParamL[i];
	}
}

/*******************读取右相机参数********************/
void VisionInit::ReadCamParamR(double* InputCamParamR)
{
	CamParamR.Clear();
	for (int i = 0; i < 12; i++)
	{
		CamParamR[i] = InputCamParamR[i];
	}
}

/******************读取相机相对位置关系****************/
void VisionInit::ReadRealPose(double* InputRelPose)
{
	RelPose.Clear();
	for (int i = 0; i < 7; i++)
	{
		RelPose[i] = InputRelPose[i];
	}
}

/*******************左相机硬件检测**********************/
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

/*******************右相机硬件检测**********************/
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



/*****************设置射击模式相机参数******************/
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


/*****************大符模式下相机参数设置*****************/
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
