#ifndef PPCIMGPROCESS_H
#define PPCIMGPROCESS_H
#include "MyApplication.h"

//Halcon
#ifndef __APPLE__
#  include "HalconCpp.h"
#  include "HDevThread.h"
#else
#  ifndef HC_LARGE_IMAGES
#    include <HALCONCpp/HalconCpp.h>
#    include <HALCONCpp/HDevThread.h>
#    include <HALCON/HpThread.h>
#  else
#    include <HALCONCppxl/HalconCpp.h>
#    include <HALCONCppxl/HDevThread.h>
#    include <HALCONxl/HpThread.h>
#  endif
#  include <stdio.h>
#  include <CoreFoundation/CFRunLoop.h>
#endif

using namespace HalconCpp;

class PPCImgProcess
{
public:
    static void ClearDisplayBuff(void);
    static int FindImgHead(QString FileName,int width,int height,Hlong headWinId,IMGPara &headPara);
    static int FindImgTail(QString FileName,int width,int height,Hlong tailWinId,IMGPara &headPara);
    static void processHeadImage(HObject &ho_image,HObject &ho_region_head,int width,int height,int row1,int row2,Hlong headWinId,IMGPara &tailPara);
    static void processTailImage(HObject &ho_image,HObject &ho_region_head,int width,int height,int row1,int row2,Hlong tailWinId,IMGPara &tailPara);
};


#endif // PPCIMGPROCESS_H
