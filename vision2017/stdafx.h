#pragma once

#include <stdio.h>
#include<list>
#include<vector>
#include<iostream>
#include<fstream>
#include<math.h>
#include<windows.h>
#include<HalconCpp.h>
#include<HDevThread.h>
using namespace std;
using namespace HalconCpp;


typedef enum
{
	SHOOTENEMY,
	BIGSYMBOL,
}WorkMode;

/** ��������� */
#define   CAMERAL  {0.00397942,27123.3,7.94434e+008,8.65221e+013,-0.455712,0.100732,3.00449e-006,3e-006,963.334,547.427,1920,1080}
/** ��������� */
#define   CAMERAR  {0.00396105,27376.5,9.15663e+008,7.0383e+013,0.0669379,0.435635,3.00291e-006,3e-006,1018.68,510.166,1920,1080}
/** ���λ�ù�ϵ */
#define   REALPOSE  {0.370458, 0.0142961, -0.0490998, 359.745, 0.559289, 0.0698773, 0}
//0.371675

/** ��������������� */
#define  CameraLRowCenter   547.427
/** ��������������� */
#define  CameraLColumnCenter   963.334

/** ��������������� */
#define  CameraRRowCenter   510.166
/** ��������������� */
#define  CameraRColumnCenter   1018.68

/** ��������� */
#define   CAMERALFOCUS   3.97942
/** ��������� */
#define   CAMERARFOCUS   3.96105

/*����Ԫ��С*/
#define PIXEL_SIZE 0.003

/* �����ư峤�� */
#define Hei 0.06
#define Wet 0.13

/* Ӣ�۵ư峤�� */
#define HeiS 0.06
#define WetS 0.225


/** ��תȨֵ */
#define RotateWeight 0.3

/** �ٶ�Ȩֵ */
#define SpeedWeight 0.3

/** ����Ȩֵ */
#define DistanceWeight 0.4

/* �������Ѷ� */
#define TrackLever 6

/** �������Ѷ� */
#define ShootingLever  4.8

/** ת�����Ŀ���ѶȲ�ֵ */
#define ChangeLever 0.7

/* �������� */
#define  REDORBLUE  1

/** ���ƥ������ֵ */
#define   MAXMATCHPP   0.35

