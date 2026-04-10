#include "ppcimgprocess.h"
#include "appconfig.h"
#include "MyApplication.h"
#include "appconfig.h"

#include <QDebug>

void PPCImgProcess::ClearDisplayBuff(void)
{
    MyApplication *pApp=(MyApplication *)qApp;
}

int PPCImgProcess::FindImgHead(QString FileName,int width,int height,Hlong headWinId,IMGPara &headPara)
{
    MyApplication *pApp=(MyApplication *)qApp;
    NodeSetting *pNodeData=&pApp->pNodeData->setting;

    int row1,row2;

    HObject ho_Regions,ho_SelectedRegions,ho_RegionClosing,ho_FillRegions,ho_ConnectedRegions;
    HObject ho_RegionMax;
    HObject ho_imageScale;

    HTuple hv_Width, hv_Height;
    HTuple  hv_AbsoluteHisto, hv_RelativeHisto, hv_MinThresh,hv_MaxThresh;
    HTuple  hv_find_threshold,hv_min_value,hv_sizes,hv_i;
    HTuple  hv_Number,hv_Area,hv_Row,hv_Column;

    HTuple hv_area_min,hv_area_max,hv_rate_min,hv_rate_max;
    HTuple hv_Indices,hv_Length;
    HTuple hv_Exception,hv_ErrorCode;
    HTuple hv_Row1, hv_Column1, hv_Row2, hv_Column2;

    HTuple hv_find_i;

    hv_area_min = 20000.0;
    hv_area_max = 1.0e+9;
    hv_rate_min = 0.8;
    hv_rate_max = 2.0;

    try
    {
        ReadImage(&ho_imageScale, FileName.toStdString().c_str());
        GetImageSize(ho_imageScale, &hv_Width, &hv_Height);

        GrayHisto(ho_imageScale, ho_imageScale, &hv_AbsoluteHisto, &hv_RelativeHisto);
        HistoToThresh(hv_AbsoluteHisto, 5, &hv_MinThresh, &hv_MaxThresh);

        //寻找阈值
        TupleLength(hv_MinThresh, &hv_sizes);
        hv_find_threshold = 100;
        if (0 != (int(hv_sizes>2)))
        {
            hv_find_i = (hv_sizes/2).TupleInt();
            if (0 != (int((hv_sizes%2)==1)))
            {
                hv_find_threshold = HTuple(hv_MinThresh[hv_find_i]);
            }
            else
            {
                hv_find_threshold = (HTuple(hv_MinThresh[hv_find_i-1])+HTuple(hv_MinThresh[hv_find_i]))*0.5;
            }
        }
        else if (0 != (int(hv_sizes==2)))
        {
            hv_find_threshold = (HTuple(hv_MinThresh[0])+HTuple(hv_MinThresh[1]))*0.5;
        }

        //限幅
        if (0 != (int(hv_find_threshold<50)))
        {
            hv_find_threshold = 50;
        }
        else if (0 != (int(hv_find_threshold>150)))
        {
            hv_find_threshold = 150;
        }

        if (0 != (int(hv_find_threshold>20)))
        {
            GetImageSize(ho_imageScale, &hv_Width, &hv_Height);
            Emphasize(ho_imageScale, &ho_imageScale, hv_Width, hv_Height,1);

            Threshold(ho_imageScale, &ho_Regions, hv_find_threshold,255);

            ClosingCircle(ho_Regions, &ho_RegionClosing, 15);
            FillUp(ho_RegionClosing, &ho_FillRegions);

            Connection(ho_FillRegions, &ho_ConnectedRegions);
            SelectShape(ho_ConnectedRegions, &ho_SelectedRegions, (HTuple("area").Append("rectangularity")),
                "and", hv_area_min.TupleConcat(hv_rate_min), hv_area_max.TupleConcat(hv_rate_max));

            CountObj(ho_SelectedRegions, &hv_Number);
            if (0 != (int(hv_Number>0)))
            {
                //寻找面积最大的区域
                AreaCenter(ho_SelectedRegions, &hv_Area, &hv_Row, &hv_Column);
                TupleSortIndex(hv_Area, &hv_Indices);
                TupleLength(hv_Area, &hv_Length);
                SelectObj(ho_SelectedRegions, &ho_RegionMax, HTuple(hv_Indices[hv_Length-1])+1);

                //区域坐标
                SmallestRectangle1(ho_RegionMax, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);
                row1=hv_Row1[0];
                row2=hv_Row2[0];

                //处理头部图像
                processHeadImage(ho_imageScale,ho_RegionMax,width,height,row1,row2,headWinId,headPara);

                //找到钢板返回true
                return 1;
            }
        }
    }
    catch (HException &HDevExpDefaultException)
    {
        HDevExpDefaultException.ToHTuple(&hv_Exception);
        hv_ErrorCode = ((const HTuple&)hv_Exception)[0];

        QLOG_ERROR() << QString(u8"FindImgHead 处理图像发生错误,错误代码=%2").arg(hv_ErrorCode[0].I());

        return -1;
    }

    return 0;
}

int PPCImgProcess::FindImgTail(QString FileName,int width,int height,Hlong tailWinId,IMGPara &tailPara)
{
    MyApplication *pApp=(MyApplication *)qApp;
    NodeSetting *pNodeData=&pApp->pNodeData->setting;

    int row1,row2;

    HObject ho_Regions,ho_SelectedRegions,ho_RegionClosing,ho_FillRegions,ho_ConnectedRegions;
    HObject ho_RegionMax;
    HObject ho_imageScale;

    HTuple hv_Width, hv_Height;
    HTuple  hv_AbsoluteHisto, hv_RelativeHisto, hv_MinThresh,hv_MaxThresh;
    HTuple  hv_find_threshold,hv_min_value,hv_sizes,hv_i;
    HTuple  hv_Number,hv_Area,hv_Row,hv_Column;

    HTuple hv_area_min,hv_area_max,hv_rate_min,hv_rate_max;
    HTuple hv_Indices,hv_Length;
    HTuple hv_Exception,hv_ErrorCode;
    HTuple hv_Row1, hv_Column1, hv_Row2, hv_Column2;

    HTuple hv_find_i;

    hv_area_min = 20000.0;
    hv_area_max = 1.0e+9;
    hv_rate_min = 0.8;
    hv_rate_max = 2.0;

    try
    {
        ReadImage(&ho_imageScale, FileName.toStdString().c_str());
        GetImageSize(ho_imageScale, &hv_Width, &hv_Height);

        GrayHisto(ho_imageScale, ho_imageScale, &hv_AbsoluteHisto, &hv_RelativeHisto);
        HistoToThresh(hv_AbsoluteHisto, 5, &hv_MinThresh, &hv_MaxThresh);

        //寻找阈值
        TupleLength(hv_MinThresh, &hv_sizes);
        hv_find_threshold = 100;
        if (0 != (int(hv_sizes>2)))
        {
            hv_find_i = (hv_sizes/2).TupleInt();
            if (0 != (int((hv_sizes%2)==1)))
            {
                hv_find_threshold = HTuple(hv_MinThresh[hv_find_i]);
            }
            else
            {
                hv_find_threshold = (HTuple(hv_MinThresh[hv_find_i-1])+HTuple(hv_MinThresh[hv_find_i]))*0.5;
            }
        }
        else if (0 != (int(hv_sizes==2)))
        {
            hv_find_threshold = (HTuple(hv_MinThresh[0])+HTuple(hv_MinThresh[1]))*0.5;
        }

        //限幅
        if (0 != (int(hv_find_threshold<50)))
        {
            hv_find_threshold = 50;
        }
        else if (0 != (int(hv_find_threshold>150)))
        {
            hv_find_threshold = 150;
        }

        if (0 != (int(hv_find_threshold>20)))
        {
            GetImageSize(ho_imageScale, &hv_Width, &hv_Height);
            Emphasize(ho_imageScale, &ho_imageScale, hv_Width, hv_Height,1);

            Threshold(ho_imageScale, &ho_Regions, hv_find_threshold,255);

            ClosingCircle(ho_Regions, &ho_RegionClosing, 15);
            FillUp(ho_RegionClosing, &ho_FillRegions);

            Connection(ho_FillRegions, &ho_ConnectedRegions);
            SelectShape(ho_ConnectedRegions, &ho_SelectedRegions, (HTuple("area").Append("rectangularity")),
                "and", hv_area_min.TupleConcat(hv_rate_min), hv_area_max.TupleConcat(hv_rate_max));

            CountObj(ho_SelectedRegions, &hv_Number);
            if (0 != (int(hv_Number>0)))
            {
                //寻找面积最大的区域
                AreaCenter(ho_SelectedRegions, &hv_Area, &hv_Row, &hv_Column);
                TupleSortIndex(hv_Area, &hv_Indices);
                TupleLength(hv_Area, &hv_Length);
                SelectObj(ho_SelectedRegions, &ho_RegionMax, HTuple(hv_Indices[hv_Length-1])+1);

                //区域坐标
                SmallestRectangle1(ho_RegionMax, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);
                row1=hv_Row1[0];
                row2=hv_Row2[0];

                //处理尾部图像
                processTailImage(ho_imageScale,ho_RegionMax,width,height,row1,row2,tailWinId,tailPara);

                //找到钢板返回true
                return 1;
            }
        }
    }
    catch (HException &HDevExpDefaultException)
    {
        HDevExpDefaultException.ToHTuple(&hv_Exception);
        hv_ErrorCode = ((const HTuple&)hv_Exception)[0];

        QLOG_ERROR() << QString(u8"FindImgTail 处理图像发生错误,错误代码=%2").arg(hv_ErrorCode[0].I());

        return -1;
    }

    return 0;
}
void PPCImgProcess::processHeadImage(HObject &ho_image,HObject &ho_region_head,int width,int height,int row1,int row2,Hlong headWinId,IMGPara &headPara)
{
    MyApplication *pApp=(MyApplication *)qApp;
    NodeData *pNodeData=pApp->pNodeData;

    HObject ho_Contours,ho_SmoothedContours,ho_CroppedContours,ho_Region1;
    HObject ho_rect1,ho_rect2,ho_Region,ho_leftRegion,ho_rightRegion;

    HTuple hv_Width,hv_Height;
    HTuple hv_Row,hv_Col,hv_rowMin,hv_rowMax;
    HTuple hv_num,hv_wid_pre,hv_i,hv_j,hv_Indices;
    HTuple hv_colMin,hv_colMax,hv_mylen,hv_wid_curr,hv_colMax_ok,hv_colMin_ok;
    HTuple hv_div_x,hv_div_y,hv_cut_line,hv_div_num,hv_delta;
    HTuple hv_find_row;

    HTuple hv_width_act,hv_Area,hv_regionX,hv_regionY,hv_height_avg,hv_col_mid;
    HTuple hv_lr_ratio,hv_leftArea,hv_rightArea,hv_leftRegionX,hv_leftRegionY,hv_rightRegionX,hv_rightRegionY;

    GetImageSize(ho_image, &hv_Width, &hv_Height);

    //轮廓处理
    GenContourRegionXld(ho_region_head, &ho_Contours, "center");
    SmoothContoursXld(ho_Contours, &ho_SmoothedContours, pNodeData->setting.smooth_contours_set);

    //为获得整数坐标重新转换一下
    GenRegionContourXld(ho_SmoothedContours, &ho_Region1, "filled");
    GenContourRegionXld(ho_Region1, &ho_Contours, "center");

    GetContourXld(ho_Contours, &hv_Row, &hv_Col);
    TupleMin(hv_Row, &hv_rowMin);

    //寻找位置
    hv_num = 0;
    hv_find_row = 0;
    hv_wid_pre = 0;
    {
        HTuple end_val65 = hv_Height;
        HTuple step_val65 = 1;
        for (hv_i=hv_rowMin+30; hv_i.Continue(end_val65, step_val65); hv_i += step_val65)
        {
          TupleFind(hv_Row, hv_i, &hv_Indices);
          hv_colMin = HTuple(hv_Col[HTuple(hv_Indices[0])]);
          hv_colMax = HTuple(hv_Col[HTuple(hv_Indices[0])]);
          TupleLength(hv_Indices, &hv_mylen);
          {
              HTuple end_val70 = hv_mylen-1;
              HTuple step_val70 = 1;
              for (hv_j=1; hv_j.Continue(end_val70, step_val70); hv_j += step_val70)
              {
                if (0 != (int(hv_colMin>HTuple(hv_Col[HTuple(hv_Indices[hv_j])]))))
                {
                  hv_colMin = HTuple(hv_Col[HTuple(hv_Indices[hv_j])]);
                }
                if (0 != (int(hv_colMax<HTuple(hv_Col[HTuple(hv_Indices[hv_j])]))))
                {
                  hv_colMax = HTuple(hv_Col[HTuple(hv_Indices[hv_j])]);
                }
              }
          }

          hv_wid_curr = hv_colMax-hv_colMin;
          if (0 != (int(hv_wid_curr<=hv_wid_pre)))
          {
            hv_num += 1;
          }
          else
          {
            hv_num = 0;
          }

          if (0 != (int(hv_num>=10)))
          {
            hv_find_row = hv_i;
            hv_colMax_ok = hv_colMax;
            hv_colMin_ok = hv_colMin;
            break;
          }

          hv_wid_pre = hv_wid_curr;
        }
    }

    //截取头部轮廓
    CropContoursXld(ho_Contours, &ho_CroppedContours, 0, 0, hv_find_row, hv_Width,
        "true");

    //重新提取截取轮廓的行列坐标
    GetContourXld(ho_CroppedContours, &hv_Row, &hv_Col);

    //分割
    hv_div_x = HTuple();
    hv_div_y = HTuple();
    hv_cut_line = hv_find_row;
    hv_div_num = 100;
    hv_delta = (hv_colMax_ok-hv_colMin_ok)/hv_div_num;

    hv_div_x[0] = hv_colMin_ok;
    hv_div_y[0] = hv_cut_line;
    {
        HTuple end_val111 = hv_div_num-1;
        HTuple step_val111 = 1;
        for (hv_i=1; hv_i.Continue(end_val111, step_val111); hv_i += step_val111)
        {
          hv_div_x[hv_i] = (hv_colMin_ok+(hv_i*hv_delta)).TupleRound();
          TupleFind(hv_Col, HTuple(hv_div_x[hv_i]), &hv_Indices);
          TupleLength(hv_Indices, &hv_mylen);
          hv_rowMin = HTuple(hv_Row[HTuple(hv_Indices[0])]);
          hv_rowMax = HTuple(hv_Row[HTuple(hv_Indices[0])]);
          {
              HTuple end_val117 = hv_mylen-1;
              HTuple step_val117 = 1;
              for (hv_j=1; hv_j.Continue(end_val117, step_val117); hv_j += step_val117)
              {
                if (0 != (int(hv_rowMin>HTuple(hv_Row[HTuple(hv_Indices[hv_j])]))))
                {
                  hv_rowMin = HTuple(hv_Row[HTuple(hv_Indices[hv_j])]);
                }
                if (0 != (int(hv_rowMax<HTuple(hv_Row[HTuple(hv_Indices[hv_j])]))))
                {
                  hv_rowMax = HTuple(hv_Row[HTuple(hv_Indices[hv_j])]);
                }
              }
          }
          hv_div_y[hv_i] = hv_rowMin;
        }
    }
    hv_div_x[hv_div_num] = hv_colMax_ok;
    hv_div_y[hv_div_num] = hv_cut_line;

    //总面积与平均高度
    hv_width_act = hv_colMax_ok-hv_colMin_ok;
    GenRegionContourXld(ho_CroppedContours, &ho_Region, "filled");
    AreaCenter(ho_Region, &hv_Area, &hv_regionX, &hv_regionY);
    hv_height_avg = hv_Area/hv_width_act;

    hv_col_mid = hv_colMin_ok+((hv_colMax_ok-hv_colMin_ok)*0.5);
    //左面积
    GenRectangle1(&ho_rect1, 0, hv_col_mid, hv_Height, hv_Width);
    Difference(ho_Region, ho_rect1, &ho_leftRegion);
    AreaCenter(ho_leftRegion, &hv_leftArea, &hv_leftRegionX, &hv_leftRegionY);

    //右面积
    GenRectangle1(&ho_rect2, 0, 0, hv_Height, hv_col_mid);
    Difference(ho_Region, ho_rect2, &ho_rightRegion);
    AreaCenter(ho_rightRegion, &hv_rightArea, &hv_rightRegionX, &hv_rightRegionY);

    hv_lr_ratio = (hv_leftArea*1.0)/hv_rightArea;

    //头部数据保存
    float each_pixel_equal_mm=AppConfig::each_pixel_equal_mm;
    headPara.img_area=hv_Area[0].D();
    headPara.img_height=hv_height_avg[0].D();
    headPara.height=headPara.img_height*each_pixel_equal_mm;
    headPara.lr_ratio=hv_lr_ratio[0].D();

    float left_max=0,right_max=0;
    QVector<float> ppc_curve_x;
    QVector<float> ppc_curve_y;

    ppc_curve_x.clear();
    ppc_curve_y.clear();
    QString strCoordX="";
    QString strCoordY="";
    for(int i=0;i<101;i++)
    {
        ppc_curve_x<<hv_div_x[i]-hv_div_x[0];
        ppc_curve_y<<hv_cut_line[0]-hv_div_y[i];

        if(i==100)
        {
            strCoordX+=QString("%1").arg(ppc_curve_x[i]);
            strCoordY+=QString("%1").arg(ppc_curve_y[i]);
        }
        else
        {
            strCoordX+=QString("%1,").arg(ppc_curve_x[i]);
            strCoordY+=QString("%1,").arg(ppc_curve_y[i]);
        }


        if(i<50)
        {
            if(hv_cut_line[0]-hv_div_y[i]>left_max)
                left_max=hv_cut_line[0]-hv_div_y[i];
        }
        if(i>50)
        {
            if(hv_cut_line[0]-hv_div_y[i]>right_max)
                right_max=hv_cut_line[0]-hv_div_y[i];
        }
    }

    headPara.coordX=strCoordX;
    headPara.coordY=strCoordY;
    qDebug()<<strCoordX;
    qDebug()<<strCoordY;

    //左右取指定位置高度
    headPara.left_height=left_max*each_pixel_equal_mm;
    headPara.right_height=right_max*each_pixel_equal_mm;
    headPara.l_r_pos_ratio=left_max*1.0/right_max;


    //打开窗口
    OpenWindow(0, 0, width, height, headWinId, "visible", "", &winHandle_pro);
    HDevWindowStack::Push(winHandle_pro);

    HDevWindowStack::SetActive(winHandle_pro);
    SetWindowAttr("background_color","black");

    SetPart(winHandle_pro,0, 0, hv_Height, hv_Width);
    DispObj(ho_image, winHandle_pro);

    SetColor(winHandle_pro,"yellow");
    DispObj(ho_CroppedContours, winHandle_pro);

    SetColor(winHandle_pro,"red");
    for (hv_i=1; hv_i<hv_div_num-1; hv_i +=2)
        DispLine(winHandle_pro, hv_cut_line, HTuple(hv_div_x[hv_i]), HTuple(hv_div_y[hv_i]),HTuple(hv_div_x[hv_i]));
}

void PPCImgProcess::processTailImage(HObject &ho_image,HObject &ho_region_tail,int width,int height,int row1,int row2,Hlong tailWinId,IMGPara &tailPara)
{
    MyApplication *pApp=(MyApplication *)qApp;
    NodeData *pNodeData=pApp->pNodeData;

    HObject ho_Contours,ho_SmoothedContours,ho_CroppedContours,ho_Region1;
    HObject ho_rect1,ho_rect2,ho_Region,ho_leftRegion,ho_rightRegion;

    HTuple hv_Width,hv_Height;
    HTuple hv_Row_int,hv_Col_int,hv_Row,hv_Col,hv_rowMin,hv_rowMax;
    HTuple hv_num,hv_wid_pre,hv_i,hv_j,hv_Indices;
    HTuple hv_colMin,hv_colMax,hv_mylen,hv_wid_curr,hv_colMax_ok,hv_colMin_ok;
    HTuple hv_div_x,hv_div_y,hv_cut_line,hv_div_num,hv_delta;
    HTuple hv_find_row;

    HTuple hv_width_act,hv_Area,hv_regionX,hv_regionY,hv_height_avg,hv_col_mid;
    HTuple hv_lr_ratio,hv_leftArea,hv_rightArea,hv_leftRegionX,hv_leftRegionY,hv_rightRegionX,hv_rightRegionY;

    GetImageSize(ho_image, &hv_Width, &hv_Height);

    //轮廓处理
    GenContourRegionXld(ho_region_tail, &ho_Contours, "center");
    SmoothContoursXld(ho_Contours, &ho_SmoothedContours, pNodeData->setting.smooth_contours_set);

    //为获得整数坐标重新转换一下
    GenRegionContourXld(ho_SmoothedContours, &ho_Region1, "filled");
    GenContourRegionXld(ho_Region1, &ho_Contours, "center");

    GetContourXld(ho_Contours, &hv_Row, &hv_Col);
    TupleMax(hv_Row, &hv_rowMax);

    //寻找位置
    hv_num = 0;
    hv_find_row = 0;
    hv_wid_pre = 0;
    for (hv_i=hv_rowMax-30; hv_i>=0; hv_i+=-1)
    {
      TupleFind(hv_Row, hv_i, &hv_Indices);
      hv_colMin = HTuple(hv_Col[HTuple(hv_Indices[0])]);
      hv_colMax = HTuple(hv_Col[HTuple(hv_Indices[0])]);
      TupleLength(hv_Indices, &hv_mylen);
      {
          HTuple end_val69 = hv_mylen-1;
          HTuple step_val69 = 1;
          for (hv_j=1; hv_j.Continue(end_val69, step_val69); hv_j += step_val69)
          {
            if (0 != (int(hv_colMin>HTuple(hv_Col[HTuple(hv_Indices[hv_j])]))))
            {
              hv_colMin = HTuple(hv_Col[HTuple(hv_Indices[hv_j])]);
            }
            if (0 != (int(hv_colMax<HTuple(hv_Col[HTuple(hv_Indices[hv_j])]))))
            {
              hv_colMax = HTuple(hv_Col[HTuple(hv_Indices[hv_j])]);
            }
          }
      }

      hv_wid_curr = hv_colMax-hv_colMin;
      if (0 != (int(hv_wid_curr<=hv_wid_pre)))
      {
        hv_num += 1;
      }
      else
      {
        hv_num = 0;
      }

      if (0 != (int(hv_num>=10)))
      {
        hv_find_row = hv_i;
        hv_colMax_ok = hv_colMax;
        hv_colMin_ok = hv_colMin;
        break;
      }

      hv_wid_pre = hv_wid_curr;
    }

    //截取头部轮廓
    CropContoursXld(ho_Contours, &ho_CroppedContours, hv_find_row, 0, hv_Height,
        hv_Width, "true");

    //重新提取截取轮廓的行列坐标
    GetContourXld(ho_CroppedContours, &hv_Row, &hv_Col);

    //分割
    hv_div_x = HTuple();
    hv_div_y = HTuple();
    hv_cut_line = hv_find_row;
    hv_div_num = 100;
    hv_delta = (hv_colMax_ok-hv_colMin_ok)/hv_div_num;

    hv_div_x[0] = hv_colMin_ok;
    hv_div_y[0] = hv_cut_line;
    {
        HTuple end_val110 = hv_div_num-1;
        HTuple step_val110 = 1;
        for (hv_i=1; hv_i.Continue(end_val110, step_val110); hv_i += step_val110)
        {
          hv_div_x[hv_i] = (hv_colMin_ok+(hv_i*hv_delta)).TupleRound();
          TupleFind(hv_Col, HTuple(hv_div_x[hv_i]), &hv_Indices);
          TupleLength(hv_Indices, &hv_mylen);
          hv_rowMin = HTuple(hv_Row[HTuple(hv_Indices[0])]);
          hv_rowMax = HTuple(hv_Row[HTuple(hv_Indices[0])]);
          {
          HTuple end_val116 = hv_mylen-1;
          HTuple step_val116 = 1;
          for (hv_j=1; hv_j.Continue(end_val116, step_val116); hv_j += step_val116)
          {
            if (0 != (int(hv_rowMin>HTuple(hv_Row[HTuple(hv_Indices[hv_j])]))))
            {
              hv_rowMin = HTuple(hv_Row[HTuple(hv_Indices[hv_j])]);
            }
            if (0 != (int(hv_rowMax<HTuple(hv_Row[HTuple(hv_Indices[hv_j])]))))
            {
              hv_rowMax = HTuple(hv_Row[HTuple(hv_Indices[hv_j])]);
            }
          }
          }
          hv_div_y[hv_i] = hv_rowMax;
        }
    }
    hv_div_x[hv_div_num] = hv_colMax_ok;
    hv_div_y[hv_div_num] = hv_cut_line;

    //总面积与平均高度
    hv_width_act = hv_colMax_ok-hv_colMin_ok;
    GenRegionContourXld(ho_CroppedContours, &ho_Region, "filled");
    AreaCenter(ho_Region, &hv_Area, &hv_regionX, &hv_regionY);
    hv_height_avg = hv_Area/hv_width_act;

    hv_col_mid = hv_colMin_ok+((hv_colMax_ok-hv_colMin_ok)*0.5);
    //左面积
    GenRectangle1(&ho_rect1, 0, hv_col_mid, hv_Height, hv_Width);
    Difference(ho_Region, ho_rect1, &ho_leftRegion);
    AreaCenter(ho_leftRegion, &hv_leftArea, &hv_leftRegionX, &hv_leftRegionY);

    //右面积
    GenRectangle1(&ho_rect2, 0, 0, hv_Height, hv_col_mid);
    Difference(ho_Region, ho_rect2, &ho_rightRegion);
    AreaCenter(ho_rightRegion, &hv_rightArea, &hv_rightRegionX, &hv_rightRegionY);

    hv_lr_ratio = (hv_leftArea*1.0)/hv_rightArea;

    //尾部数据保存
    float each_pixel_equal_mm=AppConfig::each_pixel_equal_mm;
    tailPara.img_area=hv_Area[0].D();
    tailPara.img_height=hv_height_avg[0].D();
    tailPara.height=tailPara.img_height*each_pixel_equal_mm;
    tailPara.lr_ratio=hv_lr_ratio[0].D();

    float left_max=0,right_max=0;
    QVector<float> ppc_curve_x;
    QVector<float> ppc_curve_y;

    ppc_curve_x.clear();
    ppc_curve_y.clear();
    QString strCoordX="";
    QString strCoordY="";
    for(int i=0;i<101;i++)
    {
        ppc_curve_x<<hv_div_x[i]-hv_div_x[0];
        ppc_curve_y<<hv_div_y[i]-hv_cut_line[0];

        if(i==100)
        {
            strCoordX+=QString("%1").arg(ppc_curve_x[i]);
            strCoordY+=QString("%1").arg(ppc_curve_y[i]);
        }
        else
        {
            strCoordX+=QString("%1,").arg(ppc_curve_x[i]);
            strCoordY+=QString("%1,").arg(ppc_curve_y[i]);
        }

        if(i<50)
        {
            if(hv_div_y[i]-hv_cut_line[0]>left_max)
                left_max=hv_div_y[i]-hv_cut_line[0];
        }
        if(i>50)
        {
            if(hv_div_y[i]-hv_cut_line[0]>right_max)
                right_max=hv_div_y[i]-hv_cut_line[0];
        }
    }

    tailPara.coordX=strCoordX;
    tailPara.coordY=strCoordY;
    qDebug()<<strCoordX;
    qDebug()<<strCoordY;

    tailPara.left_height=left_max*each_pixel_equal_mm;
    tailPara.right_height=right_max*each_pixel_equal_mm;
    tailPara.l_r_pos_ratio=left_max*1.0/right_max;

    OpenWindow(0, 0, width, height, tailWinId, "visible", "", &pApp->winHandle_tail_search);
    HDevWindowStack::Push(pApp->winHandle_tail_search);

    HDevWindowStack::SetActive(pApp->winHandle_tail_search);
    SetWindowAttr("background_color","black");

    SetPart(pApp->winHandle_tail_search,0, 0, hv_Height, hv_Width);
    DispObj(ho_image, pApp->winHandle_tail_search);

    SetColor(pApp->winHandle_tail_search,"yellow");
    DispObj(ho_CroppedContours, pApp->winHandle_tail_search);

    SetColor(pApp->winHandle_tail_search,"red");
    for (hv_i=1; hv_i<hv_div_num-1; hv_i +=2)
        DispLine(pApp->winHandle_tail_search, hv_cut_line, HTuple(hv_div_x[hv_i]), HTuple(hv_div_y[hv_i]),HTuple(hv_div_x[hv_i]));

}

