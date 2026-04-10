//
// Created by wcy on 25-8-1.
//

#include <OCR/HalconOCRInfer.h>

// Chapter: OCR / Deep OCR
// Short Description: Get the detection word boxes contained in the specified ResultKey.
void get_deep_ocr_detection_word_boxes (HTuple hv_DeepOcrResult, HTuple hv_ResultKey,
    HTuple *hv_WordBoxRow, HTuple *hv_WordBoxCol, HTuple *hv_WordBoxPhi, HTuple *hv_WordBoxLength1,
    HTuple *hv_WordBoxLength2)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_SupportedKey, hv_WordBoxDict, hv_SameSize;

  //This procedure retrieves the rectangle2 parameters of the detection word boxes
  //contained in the specified result dictionary key.
  //
  hv_SupportedKey = HTuple(HTuple(int(hv_ResultKey==HTuple("words"))).TupleOr(int(hv_ResultKey==HTuple("word_boxes_on_image")))).TupleOr(int(hv_ResultKey==HTuple("word_boxes_on_score_maps")));
  if (0 != (hv_SupportedKey.TupleNot()))
  {
    throw HException("Unsupported dictionary key.");
  }
  //
  //Get detection rectangle2 parameters.
  GetDictTuple(hv_DeepOcrResult, hv_ResultKey, &hv_WordBoxDict);
  GetDictTuple(hv_WordBoxDict, "row", &(*hv_WordBoxRow));
  GetDictTuple(hv_WordBoxDict, "col", &(*hv_WordBoxCol));
  GetDictTuple(hv_WordBoxDict, "phi", &(*hv_WordBoxPhi));
  GetDictTuple(hv_WordBoxDict, "length1", &(*hv_WordBoxLength1));
  GetDictTuple(hv_WordBoxDict, "length2", &(*hv_WordBoxLength2));
  hv_SameSize = HTuple(HTuple(HTuple(int(((*hv_WordBoxRow).TupleLength())==((*hv_WordBoxCol).TupleLength()))).TupleAnd(int(((*hv_WordBoxRow).TupleLength())==((*hv_WordBoxPhi).TupleLength())))).TupleAnd(int(((*hv_WordBoxRow).TupleLength())==((*hv_WordBoxLength1).TupleLength())))).TupleAnd(int(((*hv_WordBoxRow).TupleLength())==((*hv_WordBoxLength2).TupleLength())));
  if (0 != (hv_SameSize.TupleNot()))
  {
    throw HException("Rectangle2 parameters do not have the same size.");
  }
  //
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Parse generic visualization parameters.
void parse_generic_visualization_parameters (HTuple hv_GenParamName, HTuple hv_GenParamValue,
    HTuple *hv_BoxColor, HTuple *hv_LineWidth, HTuple *hv_FontSize, HTuple *hv_ShowWords,
    HTuple *hv_ShowOrientation)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_ShowScoreMaps, hv_ParamIdx, hv_AllowedGenParams;
  HTuple  hv_CaseIdx, hv_BoolShowWords, hv_BoolShowArrow;

  //Set default values.
  (*hv_BoxColor) = "green";
  (*hv_LineWidth) = 3;
  (*hv_FontSize) = 12;
  hv_ShowScoreMaps = 1;
  (*hv_ShowWords) = 1;
  (*hv_ShowOrientation) = 1;

  //Parse the generic parameters.
  {
  HTuple end_val9 = (hv_GenParamName.TupleLength())-1;
  HTuple step_val9 = 1;
  for (hv_ParamIdx=0; hv_ParamIdx.Continue(end_val9, step_val9); hv_ParamIdx += step_val9)
  {
    hv_AllowedGenParams.Clear();
    hv_AllowedGenParams[0] = "box_color";
    hv_AllowedGenParams[1] = "line_width";
    hv_AllowedGenParams[2] = "font_size";
    hv_AllowedGenParams[3] = "show_words";
    hv_AllowedGenParams[4] = "show_orientation";
    TupleFind(hv_AllowedGenParams, HTuple(hv_GenParamName[hv_ParamIdx]), &hv_CaseIdx);
    switch (hv_CaseIdx.I())
    {
    case 0:
      //Get color.
      (*hv_BoxColor) = HTuple(hv_GenParamValue[hv_ParamIdx]);
      break;
    case 1:
      //Get line width.
      (*hv_LineWidth) = HTuple(hv_GenParamValue[hv_ParamIdx]);
      break;
    case 2:
      //Get font size.
      (*hv_FontSize) = HTuple(hv_GenParamValue[hv_ParamIdx]);
      break;
    case 3:
      //Check whether words shall be displayed.
      hv_BoolShowWords = HTuple(hv_GenParamValue[hv_ParamIdx]);
      if (0 != (int(hv_BoolShowWords==HTuple("true"))))
      {
        (*hv_ShowWords) = 1;
      }
      else
      {
        (*hv_ShowWords) = 0;
      }
      break;
    case 4:
      //Check whether arrow of the word box should be displayed.
      hv_BoolShowArrow = HTuple(hv_GenParamValue[hv_ParamIdx]);
      if (0 != (int(hv_BoolShowArrow==HTuple("true"))))
      {
        (*hv_ShowOrientation) = 1;
      }
      else
      {
        (*hv_ShowOrientation) = 0;
      }
      break;
    case -1:
      //General parameter not valid.
      throw HException(("The general parameter \""+HTuple(hv_GenParamName[hv_ParamIdx]))+"\" is not valid.");
    }
  }
  }
  return;
}


// Short Description: Retrieve a deep learning device to work with.
void get_inference_dl_device (HTuple hv_UseFastAI2Devices, HTuple *hv_DLDevice)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_GenParamName, hv_GenParamValue, hv_AIIdx;
  HTuple  hv_DLDevices, hv_P, hv_DLDeviceHandles, hv_DLDeviceAI;
  HTuple  hv_DLDeviceType;

  //This procedure retrieves an available deep learning device that can
  //be used for inference by apply_deep_ocr. It tries to choose the faster
  //device type following this order: tensorrt, gpu, openvino and cpu.

  //Generic parameters for inference devices sorted by speed.
  hv_GenParamName.Clear();
  hv_GenParamName[0] = "ai_accelerator_interface";
  hv_GenParamName[1] = "runtime";
  hv_GenParamName[2] = "ai_accelerator_interface";
  hv_GenParamName[3] = "runtime";
  hv_GenParamValue.Clear();
  hv_GenParamValue[0] = "tensorrt";
  hv_GenParamValue[1] = "gpu";
  hv_GenParamValue[2] = "openvino";
  hv_GenParamValue[3] = "cpu";
  if (0 != (int(hv_UseFastAI2Devices==HTuple("false"))))
  {
    hv_AIIdx = hv_GenParamName.TupleFind("ai_accelerator_interface");
    hv_GenParamName = hv_GenParamName.TupleRemove(hv_AIIdx);
    hv_GenParamValue = hv_GenParamValue.TupleRemove(hv_AIIdx);
  }

  //Get the deep learning inference device.
  hv_DLDevices = HTuple();
  {
  HTuple end_val15 = (hv_GenParamName.TupleLength())-1;
  HTuple step_val15 = 1;
  for (hv_P=0; hv_P.Continue(end_val15, step_val15); hv_P += step_val15)
  {
    QueryAvailableDlDevices(HTuple(hv_GenParamName[hv_P]), HTuple(hv_GenParamValue[hv_P]),
        &hv_DLDeviceHandles);
    hv_DLDevices = hv_DLDevices.TupleConcat(hv_DLDeviceHandles);
    if (0 != (int(hv_DLDevices!=HTuple())))
    {
      break;
    }
  }
  }
  if (0 != (int(hv_DLDevices==HTuple())))
  {
    throw HException("No supported deep learning device found");
  }
  (*hv_DLDevice) = ((const HTuple&)hv_DLDevices)[0];

  //In case of CPU the number of threads impacts the example duration.
  GetDlDeviceParam((*hv_DLDevice), "ai_accelerator_interface", &hv_DLDeviceAI);
  GetDlDeviceParam((*hv_DLDevice), "type", &hv_DLDeviceType);
  if (0 != (HTuple(int(hv_DLDeviceAI==HTuple("none"))).TupleAnd(int(hv_DLDeviceType==HTuple("cpu")))))
  {
    SetSystem("thread_num", 4);
  }

  return;
}


