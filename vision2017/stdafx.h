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

/** 左相机参数 */
#define   CAMERAL  {0.00397942,27123.3,7.94434e+008,8.65221e+013,-0.455712,0.100732,3.00449e-006,3e-006,963.334,547.427,1920,1080}
/** 右相机参数 */
#define   CAMERAR  {0.00396105,27376.5,9.15663e+008,7.0383e+013,0.0669379,0.435635,3.00291e-006,3e-006,1018.68,510.166,1920,1080}
/** 相机位置关系 */
#define   REALPOSE  {0.370458, 0.0142961, -0.0490998, 359.745, 0.559289, 0.0698773, 0}
//0.371675

/** 左相机光轴行坐标 */
#define  CameraLRowCenter   547.427
/** 左相机光轴列坐标 */
#define  CameraLColumnCenter   963.334

/** 右相机光轴行坐标 */
#define  CameraRRowCenter   510.166
/** 右相机光轴列坐标 */
#define  CameraRColumnCenter   1018.68

/** 左相机焦距 */
#define   CAMERALFOCUS   3.97942
/** 右相机焦距 */
#define   CAMERARFOCUS   3.96105

/*像素元大小*/
#define PIXEL_SIZE 0.003

/* 步兵灯板长宽 */
#define Hei 0.06
#define Wet 0.13

/* 英雄灯板长宽 */
#define HeiS 0.06
#define WetS 0.225


/** 旋转权值 */
#define RotateWeight 0.3

/** 速度权值 */
#define SpeedWeight 0.3

/** 距离权值 */
#define DistanceWeight 0.4

/* 最大跟踪难度 */
#define TrackLever 6

/** 最大射击难度 */
#define ShootingLever  4.8

/** 转换射击目标难度差值 */
#define ChangeLever 0.7

/* 红蓝开关 */
#define  REDORBLUE  1

/** 最大匹配距离差值 */
#define   MAXMATCHPP   0.35

