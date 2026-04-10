#include <Det/HalconDetInfer.h>

// Procedures
// External procedures 
// Chapter: Image / Channel
void add_colormap_to_image (HObject ho_GrayValueImage, HObject ho_Image, HObject *ho_ColoredImage, 
    HTuple hv_HeatmapColorScheme)
{

  // Local iconic variables
  HObject  ho_RGBValueImage, ho_Channels, ho_ChannelsScaled;
  HObject  ho_Channel, ho_ChannelScaled, ho_ChannelScaledByte;
  HObject  ho_ImageByte, ho_ImageByteR, ho_ImageByteG, ho_ImageByteB;

  // Local control variables
  HTuple  hv_Type, hv_NumChannels, hv_ChannelIndex;
  HTuple  hv_ChannelMin, hv_ChannelMax, hv__;

  //
  //This procedure adds a gray-value image to a RGB image with a chosen color map.
  //
  GetImageType(ho_GrayValueImage, &hv_Type);
  //The image LUT needs a byte image. Rescale real images.
  if (0 != (int(hv_Type==HTuple("real"))))
  {
    scale_image_range(ho_GrayValueImage, &ho_GrayValueImage, 0, 1);
    ConvertImageType(ho_GrayValueImage, &ho_GrayValueImage, "byte");
  }
  else if (0 != (int(hv_Type!=HTuple("byte"))))
  {
    throw HException(HTuple("For this transformation, a byte or real image is needed!"));
  }
  //
  //Apply the chosen color scheme on the gray value.
  apply_colorscheme_on_gray_value_image(ho_GrayValueImage, &ho_RGBValueImage, hv_HeatmapColorScheme);
  //
  //Convert input image to byte image for visualization.
  ImageToChannels(ho_Image, &ho_Channels);
  CountChannels(ho_Image, &hv_NumChannels);
  GenEmptyObj(&ho_ChannelsScaled);
  {
  HTuple end_val19 = hv_NumChannels;
  HTuple step_val19 = 1;
  for (hv_ChannelIndex=1; hv_ChannelIndex.Continue(end_val19, step_val19); hv_ChannelIndex += step_val19)
  {
    SelectObj(ho_Channels, &ho_Channel, hv_ChannelIndex);
    MinMaxGray(ho_Channel, ho_Channel, 0, &hv_ChannelMin, &hv_ChannelMax, &hv__);
    scale_image_range(ho_Channel, &ho_ChannelScaled, hv_ChannelMin, hv_ChannelMax);
    ConvertImageType(ho_ChannelScaled, &ho_ChannelScaledByte, "byte");
    ConcatObj(ho_ChannelsScaled, ho_ChannelScaledByte, &ho_ChannelsScaled);
  }
  }
  ChannelsToImage(ho_ChannelsScaled, &ho_ImageByte);
  //
  //Note that ImageByte needs to have the same number of channels as
  //RGBValueImage to display color map image correctly.
  CountChannels(ho_ImageByte, &hv_NumChannels);
  if (0 != (int(hv_NumChannels!=3)))
  {
    //Just take the first channel and use this to generate
    //an image with 3 channels for visualization.
    AccessChannel(ho_ImageByte, &ho_ImageByteR, 1);
    CopyImage(ho_ImageByteR, &ho_ImageByteG);
    CopyImage(ho_ImageByteR, &ho_ImageByteB);
    Compose3(ho_ImageByteR, ho_ImageByteG, ho_ImageByteB, &ho_ImageByte);
  }
  //
  AddImage(ho_ImageByte, ho_RGBValueImage, &ho_RGBValueImage, 0.5, 0);
  (*ho_ColoredImage) = ho_RGBValueImage;
  //
  return;
}

// Chapter: Image / Channel
// Short Description: Create a lookup table and convert a gray scale image. 
void apply_colorscheme_on_gray_value_image (HObject ho_InputImage, HObject *ho_ResultImage, 
    HTuple hv_Schema)
{

  // Local iconic variables
  HObject  ho_ImageR, ho_ImageG, ho_ImageB;

  // Local control variables
  HTuple  hv_X, hv_Low, hv_High, hv_OffR, hv_OffG;
  HTuple  hv_OffB, hv_A1, hv_A0, hv_R, hv_G, hv_B, hv_A0R;
  HTuple  hv_A0G, hv_A0B;

  //
  //This procedure generates an RGB ResultImage for a gray-value InputImage.
  //In order to do so, create a color distribution as look up table
  //according to the Schema.
  //
  hv_X = HTuple::TupleGenSequence(0,255,1);
  TupleGenConst(256, 0, &hv_Low);
  TupleGenConst(256, 255, &hv_High);
  //
  if (0 != (int(hv_Schema==HTuple("jet"))))
  {
    //Scheme Jet: from blue to red
    hv_OffR = 3.0*64.0;
    hv_OffG = 2.0*64.0;
    hv_OffB = 64.0;
    hv_A1 = -4.0;
    hv_A0 = 255.0+128.0;
    hv_R = (((((hv_X-hv_OffR).TupleAbs())*hv_A1)+hv_A0).TupleMax2(hv_Low)).TupleMin2(hv_High);
    hv_G = (((((hv_X-hv_OffG).TupleAbs())*hv_A1)+hv_A0).TupleMax2(hv_Low)).TupleMin2(hv_High);
    hv_B = (((((hv_X-hv_OffB).TupleAbs())*hv_A1)+hv_A0).TupleMax2(hv_Low)).TupleMin2(hv_High);
    //
  }
  else if (0 != (int(hv_Schema==HTuple("inverse_jet"))))
  {
    //Scheme InvJet: from red to blue.
    hv_OffR = 64;
    hv_OffG = 2*64;
    hv_OffB = 3*64;
    hv_A1 = -4.0;
    hv_A0 = 255.0+128.0;
    hv_R = (((((hv_X-hv_OffR).TupleAbs())*hv_A1)+hv_A0).TupleMax2(hv_Low)).TupleMin2(hv_High);
    hv_G = (((((hv_X-hv_OffG).TupleAbs())*hv_A1)+hv_A0).TupleMax2(hv_Low)).TupleMin2(hv_High);
    hv_B = (((((hv_X-hv_OffB).TupleAbs())*hv_A1)+hv_A0).TupleMax2(hv_Low)).TupleMin2(hv_High);
    //
  }
  else if (0 != (int(hv_Schema==HTuple("hot"))))
  {
    //Scheme Hot.
    hv_A1 = 3.0;
    hv_A0R = 0.0;
    hv_A0G = ((1.0/3.0)*hv_A1)*255.0;
    hv_A0B = ((2.0/3.0)*hv_A1)*255.0;
    hv_R = (((hv_X*hv_A1)-hv_A0R).TupleMax2(hv_Low)).TupleMin2(hv_High);
    hv_G = (((hv_X*hv_A1)-hv_A0G).TupleMax2(hv_Low)).TupleMin2(hv_High);
    hv_B = (((hv_X*hv_A1)-hv_A0B).TupleMax2(hv_Low)).TupleMin2(hv_High);
    //
  }
  else if (0 != (int(hv_Schema==HTuple("inverse_hot"))))
  {
    //Scheme Inverse Hot.
    hv_A1 = -3.0;
    hv_A0R = hv_A1*255.0;
    hv_A0G = ((2.0/3.0)*hv_A1)*255.0;
    hv_A0B = ((1.0/3.0)*hv_A1)*255.0;
    hv_R = (((hv_X*hv_A1)-hv_A0R).TupleMax2(hv_Low)).TupleMin2(hv_High);
    hv_G = (((hv_X*hv_A1)-hv_A0G).TupleMax2(hv_Low)).TupleMin2(hv_High);
    hv_B = (((hv_X*hv_A1)-hv_A0B).TupleMax2(hv_Low)).TupleMin2(hv_High);
    //
  }
  else
  {
    //
    throw HException(("Unknown color schema: "+hv_Schema)+".");
    //
  }
  //
  LutTrans(ho_InputImage, &ho_ImageR, hv_R);
  LutTrans(ho_InputImage, &ho_ImageG, hv_G);
  LutTrans(ho_InputImage, &ho_ImageB, hv_B);
  Compose3(ho_ImageR, ho_ImageG, ho_ImageB, &(*ho_ResultImage));
  //
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Compute zoom factors to fit an image to a target size. 
void calculate_dl_image_zoom_factors (HTuple hv_ImageWidth, HTuple hv_ImageHeight, 
    HTuple hv_TargetWidth, HTuple hv_TargetHeight, HTuple hv_DLPreprocessParam, HTuple *hv_ZoomFactorWidth, 
    HTuple *hv_ZoomFactorHeight)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_ScaleWidthUnit, hv_ScaleHeightUnit;
  HTuple  hv_PreserveAspectRatio, hv_Scale, hv___Tmp_Ctrl_Dict_Init_0;

  //Calculate the unit zoom factors, which zoom the input image to 1px.
  hv_ScaleWidthUnit = 1.0/(hv_ImageWidth.TupleReal());
  hv_ScaleHeightUnit = 1.0/(hv_ImageHeight.TupleReal());
  //
  //Calculate the required zoom factors for the available target size.
  (*hv_ZoomFactorWidth) = hv_TargetWidth*hv_ScaleWidthUnit;
  (*hv_ZoomFactorHeight) = hv_TargetHeight*hv_ScaleHeightUnit;
  //
  //Aspect-ratio preserving zoom is supported for model type 'ocr_detection' only.
  CreateDict(&hv___Tmp_Ctrl_Dict_Init_0);
  SetDictTuple(hv___Tmp_Ctrl_Dict_Init_0, "comp", "ocr_detection");
  hv_PreserveAspectRatio = (hv_DLPreprocessParam.TupleConcat(hv___Tmp_Ctrl_Dict_Init_0)).TupleTestEqualDictItem("model_type","comp");
  hv___Tmp_Ctrl_Dict_Init_0 = HTuple::TupleConstant("HNULL");
  //
  if (0 != hv_PreserveAspectRatio)
  {
    //
    //Use smaller scaling factor, which results in unfilled domain
    //on the respective other axis.
    hv_Scale = (*hv_ZoomFactorWidth).TupleMin2((*hv_ZoomFactorHeight));
    //Ensure that the zoom factors result in lengths of at least 1px.
    (*hv_ZoomFactorWidth) = hv_Scale.TupleMax2(hv_ScaleWidthUnit);
    (*hv_ZoomFactorHeight) = hv_Scale.TupleMax2(hv_ScaleHeightUnit);
  }
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Check the content of the parameter dictionary DLPreprocessParam. 
void check_dl_preprocess_param (HTuple hv_DLPreprocessParam)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_CheckParams, hv_KeyExists, hv_DLModelType;
  HTuple  hv_Exception, hv_SupportedModelTypes, hv_Index;
  HTuple  hv_ParamNamesGeneral, hv_ParamNamesSegmentation;
  HTuple  hv_ParamNamesDetectionOptional, hv_ParamNamesPreprocessingOptional;
  HTuple  hv_ParamNames3DGrippingPointsOptional, hv_ParamNamesAll;
  HTuple  hv_ParamNames, hv_KeysExists, hv_I, hv_Exists, hv_InputKeys;
  HTuple  hv_Key, hv_Value, hv_Indices, hv_ValidValues, hv_ValidTypes;
  HTuple  hv_V, hv_T, hv_IsInt, hv_ValidTypesListing, hv_ValidValueListing;
  HTuple  hv_EmptyStrings, hv_ImageRangeMinExists, hv_ImageRangeMaxExists;
  HTuple  hv_ImageRangeMin, hv_ImageRangeMax, hv_IndexParam;
  HTuple  hv_SetBackgroundID, hv_ClassIDsBackground, hv_Intersection;
  HTuple  hv_IgnoreClassIDs, hv_KnownClasses, hv_IgnoreClassID;
  HTuple  hv_OptionalKeysExist, hv_InstanceType, hv_IsInstanceSegmentation;
  HTuple  hv_IgnoreDirection, hv_ClassIDsNoOrientation, hv_SemTypes;

  //
  //This procedure checks a dictionary with parameters for DL preprocessing.
  //
  hv_CheckParams = 1;
  //If check_params is set to false, do not check anything.
  GetDictParam(hv_DLPreprocessParam, "key_exists", "check_params", &hv_KeyExists);
  if (0 != hv_KeyExists)
  {
    GetDictTuple(hv_DLPreprocessParam, "check_params", &hv_CheckParams);
    if (0 != (hv_CheckParams.TupleNot()))
    {
      return;
    }
  }
  //
  try
  {
    GetDictTuple(hv_DLPreprocessParam, "model_type", &hv_DLModelType);
  }
  // catch (Exception) 
  catch (HException &HDevExpDefaultException)
  {
    HDevExpDefaultException.ToHTuple(&hv_Exception);
    throw HException(HTuple(HTuple("DLPreprocessParam needs the parameter: '")+"model_type")+"'");
  }
  //
  //Check for correct model type.
  hv_SupportedModelTypes.Clear();
  hv_SupportedModelTypes[0] = "counting";
  hv_SupportedModelTypes[1] = "3d_gripping_point_detection";
  hv_SupportedModelTypes[2] = "anomaly_detection";
  hv_SupportedModelTypes[3] = "classification";
  hv_SupportedModelTypes[4] = "detection";
  hv_SupportedModelTypes[5] = "gc_anomaly_detection";
  hv_SupportedModelTypes[6] = "multi_label_classification";
  hv_SupportedModelTypes[7] = "ocr_recognition";
  hv_SupportedModelTypes[8] = "ocr_detection";
  hv_SupportedModelTypes[9] = "segmentation";
  TupleFind(hv_SupportedModelTypes, hv_DLModelType, &hv_Index);
  if (0 != (HTuple(int(hv_Index==-1)).TupleOr(int(hv_Index==HTuple()))))
  {
    throw HException(HTuple("Only models of type '3d_gripping_point_detection', 'anomaly_detection', 'classification', 'detection', 'gc_anomaly_detection', 'multi_label_classification', 'ocr_recognition', 'ocr_detection' or 'segmentation' are supported"));
    return;
  }
  //
  //Parameter names that are required.
  //General parameters.
  hv_ParamNamesGeneral.Clear();
  hv_ParamNamesGeneral[0] = "model_type";
  hv_ParamNamesGeneral[1] = "image_width";
  hv_ParamNamesGeneral[2] = "image_height";
  hv_ParamNamesGeneral[3] = "image_num_channels";
  hv_ParamNamesGeneral[4] = "image_range_min";
  hv_ParamNamesGeneral[5] = "image_range_max";
  hv_ParamNamesGeneral[6] = "normalization_type";
  hv_ParamNamesGeneral[7] = "domain_handling";
  //Segmentation specific parameters.
  hv_ParamNamesSegmentation.Clear();
  hv_ParamNamesSegmentation[0] = "ignore_class_ids";
  hv_ParamNamesSegmentation[1] = "set_background_id";
  hv_ParamNamesSegmentation[2] = "class_ids_background";
  //Detection specific parameters.
  hv_ParamNamesDetectionOptional.Clear();
  hv_ParamNamesDetectionOptional[0] = "instance_type";
  hv_ParamNamesDetectionOptional[1] = "ignore_direction";
  hv_ParamNamesDetectionOptional[2] = "class_ids_no_orientation";
  hv_ParamNamesDetectionOptional[3] = "instance_segmentation";
  //Optional preprocessing parameters.
  hv_ParamNamesPreprocessingOptional.Clear();
  hv_ParamNamesPreprocessingOptional[0] = "mean_values_normalization";
  hv_ParamNamesPreprocessingOptional[1] = "deviation_values_normalization";
  hv_ParamNamesPreprocessingOptional[2] = "check_params";
  hv_ParamNamesPreprocessingOptional[3] = "augmentation";
  //3D Gripping Point Detection specific parameters.
  hv_ParamNames3DGrippingPointsOptional.Clear();
  hv_ParamNames3DGrippingPointsOptional[0] = "min_z";
  hv_ParamNames3DGrippingPointsOptional[1] = "max_z";
  hv_ParamNames3DGrippingPointsOptional[2] = "normal_image_width";
  hv_ParamNames3DGrippingPointsOptional[3] = "normal_image_height";
  //All parameters
  hv_ParamNamesAll.Clear();
  hv_ParamNamesAll.Append(hv_ParamNamesGeneral);
  hv_ParamNamesAll.Append(hv_ParamNamesSegmentation);
  hv_ParamNamesAll.Append(hv_ParamNamesDetectionOptional);
  hv_ParamNamesAll.Append(hv_ParamNames3DGrippingPointsOptional);
  hv_ParamNamesAll.Append(hv_ParamNamesPreprocessingOptional);
  hv_ParamNames = hv_ParamNamesGeneral;
  if (0 != (HTuple(int(hv_DLModelType==HTuple("segmentation"))).TupleOr(int(hv_DLModelType==HTuple("3d_gripping_point_detection")))))
  {
    //Extend ParamNames for models of type segmentation.
    hv_ParamNames = hv_ParamNames.TupleConcat(hv_ParamNamesSegmentation);
  }
  //
  //Check if legacy parameter exist.
  //Otherwise map it to the legal parameter.
  replace_legacy_preprocessing_parameters(hv_DLPreprocessParam);
  //
  //Check that all necessary parameters are included.
  //
  GetDictParam(hv_DLPreprocessParam, "key_exists", hv_ParamNames, &hv_KeysExists);
  if (0 != (int(((hv_KeysExists.TupleEqualElem(0)).TupleSum())>0)))
  {
    {
    HTuple end_val54 = hv_KeysExists.TupleLength();
    HTuple step_val54 = 1;
    for (hv_I=0; hv_I.Continue(end_val54, step_val54); hv_I += step_val54)
    {
      hv_Exists = HTuple(hv_KeysExists[hv_I]);
      if (0 != (hv_Exists.TupleNot()))
      {
        throw HException(("DLPreprocessParam needs the parameter: '"+HTuple(hv_ParamNames[hv_I]))+"'");
      }
    }
    }
  }
  //
  //Check the keys provided.
  GetDictParam(hv_DLPreprocessParam, "keys", HTuple(), &hv_InputKeys);
  {
  HTuple end_val64 = (hv_InputKeys.TupleLength())-1;
  HTuple step_val64 = 1;
  for (hv_I=0; hv_I.Continue(end_val64, step_val64); hv_I += step_val64)
  {
    hv_Key = HTuple(hv_InputKeys[hv_I]);
    GetDictTuple(hv_DLPreprocessParam, hv_Key, &hv_Value);
    //Check that the key is known.
    TupleFind(hv_ParamNamesAll, hv_Key, &hv_Indices);
    if (0 != (int(hv_Indices==-1)))
    {
      throw HException(("Unknown key for DLPreprocessParam: '"+HTuple(hv_InputKeys[hv_I]))+"'");
      return;
    }
    //Set expected values and types.
    hv_ValidValues = HTuple();
    hv_ValidTypes = HTuple();
    if (0 != (int(hv_Key==HTuple("normalization_type"))))
    {
      hv_ValidValues.Clear();
      hv_ValidValues[0] = "all_channels";
      hv_ValidValues[1] = "first_channel";
      hv_ValidValues[2] = "constant_values";
      hv_ValidValues[3] = "none";
    }
    else if (0 != (int(hv_Key==HTuple("domain_handling"))))
    {
      if (0 != (int(hv_DLModelType==HTuple("anomaly_detection"))))
      {
        hv_ValidValues.Clear();
        hv_ValidValues[0] = "full_domain";
        hv_ValidValues[1] = "crop_domain";
        hv_ValidValues[2] = "keep_domain";
      }
      else if (0 != (int(hv_DLModelType==HTuple("3d_gripping_point_detection"))))
      {
        hv_ValidValues.Clear();
        hv_ValidValues[0] = "full_domain";
        hv_ValidValues[1] = "crop_domain";
        hv_ValidValues[2] = "keep_domain";
      }
      else
      {
        hv_ValidValues.Clear();
        hv_ValidValues[0] = "full_domain";
        hv_ValidValues[1] = "crop_domain";
      }
    }
    else if (0 != (int(hv_Key==HTuple("model_type"))))
    {
      hv_ValidValues.Clear();
      hv_ValidValues[0] = "counting";
      hv_ValidValues[1] = "3d_gripping_point_detection";
      hv_ValidValues[2] = "anomaly_detection";
      hv_ValidValues[3] = "classification";
      hv_ValidValues[4] = "detection";
      hv_ValidValues[5] = "gc_anomaly_detection";
      hv_ValidValues[6] = "multi_label_classification";
      hv_ValidValues[7] = "ocr_recognition";
      hv_ValidValues[8] = "ocr_detection";
      hv_ValidValues[9] = "segmentation";
    }
    else if (0 != (int(hv_Key==HTuple("augmentation"))))
    {
      hv_ValidValues.Clear();
      hv_ValidValues[0] = "true";
      hv_ValidValues[1] = "false";
    }
    else if (0 != (int(hv_Key==HTuple("set_background_id"))))
    {
      hv_ValidTypes = "int";
    }
    else if (0 != (int(hv_Key==HTuple("class_ids_background"))))
    {
      hv_ValidTypes = "int";
    }
    //Check that type is valid.
    if (0 != (int((hv_ValidTypes.TupleLength())>0)))
    {
      {
      HTuple end_val97 = (hv_ValidTypes.TupleLength())-1;
      HTuple step_val97 = 1;
      for (hv_V=0; hv_V.Continue(end_val97, step_val97); hv_V += step_val97)
      {
        hv_T = HTuple(hv_ValidTypes[hv_V]);
        if (0 != (int(hv_T==HTuple("int"))))
        {
          TupleIsInt(hv_Value, &hv_IsInt);
          if (0 != (hv_IsInt.TupleNot()))
          {
            hv_ValidTypes = ("'"+hv_ValidTypes)+"'";
            if (0 != (int((hv_ValidTypes.TupleLength())<2)))
            {
              hv_ValidTypesListing = hv_ValidTypes;
            }
            else
            {
              hv_ValidTypesListing = (((hv_ValidTypes.TupleSelectRange(0,HTuple(0).TupleMax2((hv_ValidTypes.TupleLength())-2)))+HTuple(", "))+HTuple(hv_ValidTypes[(hv_ValidTypes.TupleLength())-1])).TupleSum();
            }
            throw HException(((((("The value given in the key '"+hv_Key)+"' of DLPreprocessParam is invalid. Valid types are: ")+hv_ValidTypesListing)+". The given value was '")+hv_Value)+"'.");
            return;
          }
        }
        else
        {
          throw HException("Internal error. Unknown valid type.");
        }
      }
      }
    }
    //Check that value is valid.
    if (0 != (int((hv_ValidValues.TupleLength())>0)))
    {
      TupleFindFirst(hv_ValidValues, hv_Value, &hv_Index);
      if (0 != (int(hv_Index==-1)))
      {
        hv_ValidValues = ("'"+hv_ValidValues)+"'";
        if (0 != (int((hv_ValidValues.TupleLength())<2)))
        {
          hv_ValidValueListing = hv_ValidValues;
        }
        else
        {
          hv_EmptyStrings = HTuple((hv_ValidValues.TupleLength())-2,"");
          hv_ValidValueListing = (((hv_ValidValues.TupleSelectRange(0,HTuple(0).TupleMax2((hv_ValidValues.TupleLength())-2)))+HTuple(", "))+(hv_EmptyStrings.TupleConcat(HTuple(hv_ValidValues[(hv_ValidValues.TupleLength())-1])))).TupleSum();
        }
        throw HException(((((("The value given in the key '"+hv_Key)+"' of DLPreprocessParam is invalid. Valid values are: ")+hv_ValidValueListing)+". The given value was '")+hv_Value)+"'.");
      }
    }
  }
  }
  //
  //Check the correct setting of ImageRangeMin and ImageRangeMax.
  if (0 != (HTuple(HTuple(int(hv_DLModelType==HTuple("classification"))).TupleOr(int(hv_DLModelType==HTuple("multi_label_classification")))).TupleOr(int(hv_DLModelType==HTuple("detection")))))
  {
    //Check ImageRangeMin and ImageRangeMax.
    GetDictParam(hv_DLPreprocessParam, "key_exists", "image_range_min", &hv_ImageRangeMinExists);
    GetDictParam(hv_DLPreprocessParam, "key_exists", "image_range_max", &hv_ImageRangeMaxExists);
    //If they are present, check that they are set correctly.
    if (0 != hv_ImageRangeMinExists)
    {
      GetDictTuple(hv_DLPreprocessParam, "image_range_min", &hv_ImageRangeMin);
      if (0 != (int(hv_ImageRangeMin!=-127)))
      {
        throw HException(("For model type "+hv_DLModelType)+" ImageRangeMin has to be -127.");
      }
    }
    if (0 != hv_ImageRangeMaxExists)
    {
      GetDictTuple(hv_DLPreprocessParam, "image_range_max", &hv_ImageRangeMax);
      if (0 != (int(hv_ImageRangeMax!=128)))
      {
        throw HException(("For model type "+hv_DLModelType)+" ImageRangeMax has to be 128.");
      }
    }
  }
  //
  //Check segmentation specific parameters.
  if (0 != (HTuple(int(hv_DLModelType==HTuple("segmentation"))).TupleOr(int(hv_DLModelType==HTuple("3d_gripping_point_detection")))))
  {
    //Check if detection specific parameters are set.
    GetDictParam(hv_DLPreprocessParam, "key_exists", hv_ParamNamesDetectionOptional, 
        &hv_KeysExists);
    //If they are present, check that they are [].
    {
    HTuple end_val157 = (hv_ParamNamesDetectionOptional.TupleLength())-1;
    HTuple step_val157 = 1;
    for (hv_IndexParam=0; hv_IndexParam.Continue(end_val157, step_val157); hv_IndexParam += step_val157)
    {
      if (0 != (HTuple(hv_KeysExists[hv_IndexParam])))
      {
        GetDictTuple(hv_DLPreprocessParam, HTuple(hv_ParamNamesDetectionOptional[hv_IndexParam]), 
            &hv_Value);
        if (0 != (int(hv_Value!=HTuple())))
        {
          throw HException(((("The preprocessing parameter '"+HTuple(hv_ParamNamesDetectionOptional[hv_IndexParam]))+"' was set to ")+hv_Value)+HTuple(" but for segmentation it should be set to [], as it is not used for this method."));
        }
      }
    }
    }
    //Check 'set_background_id'.
    GetDictTuple(hv_DLPreprocessParam, "set_background_id", &hv_SetBackgroundID);
    if (0 != (HTuple(int(hv_SetBackgroundID!=HTuple())).TupleAnd(int(hv_DLModelType==HTuple("3d_gripping_point_detection")))))
    {
      throw HException(HTuple(HTuple("The preprocessing parameter '")+"set_background_id")+HTuple("' should be set to [] for 3d_gripping_point_detection, as it is not used for this method."));
    }
    if (0 != (int((hv_SetBackgroundID.TupleLength())>1)))
    {
      throw HException("Only one class_id as 'set_background_id' allowed.");
    }
    //Check 'class_ids_background'.
    GetDictTuple(hv_DLPreprocessParam, "class_ids_background", &hv_ClassIDsBackground);
    if (0 != (HTuple(int(hv_ClassIDsBackground!=HTuple())).TupleAnd(int(hv_DLModelType==HTuple("3d_gripping_point_detection")))))
    {
      throw HException(HTuple(HTuple("The preprocessing parameter '")+"class_ids_background")+HTuple("' should be set to [] for 3d_gripping_point_detection, as it is not used for this method."));
    }
    if (0 != (HTuple(HTuple(int((hv_SetBackgroundID.TupleLength())>0)).TupleAnd(HTuple(int((hv_ClassIDsBackground.TupleLength())>0)).TupleNot())).TupleOr(HTuple(int((hv_ClassIDsBackground.TupleLength())>0)).TupleAnd(HTuple(int((hv_SetBackgroundID.TupleLength())>0)).TupleNot()))))
    {
      throw HException("Both keys 'set_background_id' and 'class_ids_background' are required.");
    }
    //Check that 'class_ids_background' and 'set_background_id' are disjoint.
    if (0 != (int((hv_SetBackgroundID.TupleLength())>0)))
    {
      TupleIntersection(hv_SetBackgroundID, hv_ClassIDsBackground, &hv_Intersection);
      if (0 != (hv_Intersection.TupleLength()))
      {
        throw HException("Class IDs in 'set_background_id' and 'class_ids_background' need to be disjoint.");
      }
    }
    //Check 'ignore_class_ids'.
    GetDictTuple(hv_DLPreprocessParam, "ignore_class_ids", &hv_IgnoreClassIDs);
    if (0 != (HTuple(int(hv_IgnoreClassIDs!=HTuple())).TupleAnd(int(hv_DLModelType==HTuple("3d_gripping_point_detection")))))
    {
      throw HException(HTuple(HTuple("The preprocessing parameter '")+"ignore_class_ids")+HTuple("' should be set to [] for 3d_gripping_point_detection, as it is not used for this method."));
    }
    hv_KnownClasses.Clear();
    hv_KnownClasses.Append(hv_SetBackgroundID);
    hv_KnownClasses.Append(hv_ClassIDsBackground);
    {
    HTuple end_val194 = (hv_IgnoreClassIDs.TupleLength())-1;
    HTuple step_val194 = 1;
    for (hv_I=0; hv_I.Continue(end_val194, step_val194); hv_I += step_val194)
    {
      hv_IgnoreClassID = HTuple(hv_IgnoreClassIDs[hv_I]);
      TupleFindFirst(hv_KnownClasses, hv_IgnoreClassID, &hv_Index);
      if (0 != (HTuple(int((hv_Index.TupleLength())>0)).TupleAnd(int(hv_Index!=-1))))
      {
        throw HException("The given 'ignore_class_ids' must not be included in the 'class_ids_background' or 'set_background_id'.");
      }
    }
    }
  }
  else if (0 != (int(hv_DLModelType==HTuple("detection"))))
  {
    //Check if segmentation specific parameters are set.
    GetDictParam(hv_DLPreprocessParam, "key_exists", hv_ParamNamesSegmentation, &hv_KeysExists);
    //If they are present, check that they are [].
    {
    HTuple end_val205 = (hv_ParamNamesSegmentation.TupleLength())-1;
    HTuple step_val205 = 1;
    for (hv_IndexParam=0; hv_IndexParam.Continue(end_val205, step_val205); hv_IndexParam += step_val205)
    {
      if (0 != (HTuple(hv_KeysExists[hv_IndexParam])))
      {
        GetDictTuple(hv_DLPreprocessParam, HTuple(hv_ParamNamesSegmentation[hv_IndexParam]), 
            &hv_Value);
        if (0 != (int(hv_Value!=HTuple())))
        {
          throw HException(((("The preprocessing parameter '"+HTuple(hv_ParamNamesSegmentation[hv_IndexParam]))+"' was set to ")+hv_Value)+HTuple(" but for detection it should be set to [], as it is not used for this method."));
        }
      }
    }
    }
    //Check optional parameters.
    GetDictParam(hv_DLPreprocessParam, "key_exists", hv_ParamNamesDetectionOptional, 
        &hv_OptionalKeysExist);
    if (0 != (HTuple(hv_OptionalKeysExist[0])))
    {
      //Check 'instance_type'.
      GetDictTuple(hv_DLPreprocessParam, HTuple(hv_ParamNamesDetectionOptional[0]), 
          &hv_InstanceType);
      if (0 != (int((((HTuple("rectangle1").Append("rectangle2")).Append("mask")).TupleFind(hv_InstanceType))==-1)))
      {
        throw HException(("Invalid generic parameter for 'instance_type': "+hv_InstanceType)+HTuple(", only 'rectangle1' and 'rectangle2' are allowed"));
      }
    }
    //If instance_segmentation is set we might overwrite the instance_type for the preprocessing.
    if (0 != (HTuple(hv_OptionalKeysExist[3])))
    {
      GetDictTuple(hv_DLPreprocessParam, HTuple(hv_ParamNamesDetectionOptional[3]), 
          &hv_IsInstanceSegmentation);
      if (0 != (int(((((HTuple(1).Append(0)).Append("true")).Append("false")).TupleFind(hv_IsInstanceSegmentation))==-1)))
      {
        throw HException(("Invalid generic parameter for 'instance_segmentation': "+hv_IsInstanceSegmentation)+HTuple(", only true, false, 'true' and 'false' are allowed"));
      }
    }
    if (0 != (HTuple(hv_OptionalKeysExist[1])))
    {
      //Check 'ignore_direction'.
      GetDictTuple(hv_DLPreprocessParam, HTuple(hv_ParamNamesDetectionOptional[1]), 
          &hv_IgnoreDirection);
      if (0 != (int(((HTuple(1).Append(0)).TupleFind(hv_IgnoreDirection))==-1)))
      {
        throw HException(("Invalid generic parameter for 'ignore_direction': "+hv_IgnoreDirection)+HTuple(", only true and false are allowed"));
      }
    }
    if (0 != (HTuple(hv_OptionalKeysExist[2])))
    {
      //Check 'class_ids_no_orientation'.
      GetDictTuple(hv_DLPreprocessParam, HTuple(hv_ParamNamesDetectionOptional[2]), 
          &hv_ClassIDsNoOrientation);
      TupleSemTypeElem(hv_ClassIDsNoOrientation, &hv_SemTypes);
      if (0 != (HTuple(int(hv_ClassIDsNoOrientation!=HTuple())).TupleAnd(int(((hv_SemTypes.TupleEqualElem("integer")).TupleSum())!=(hv_ClassIDsNoOrientation.TupleLength())))))
      {
        throw HException(("Invalid generic parameter for 'class_ids_no_orientation': "+hv_ClassIDsNoOrientation)+HTuple(", only integers are allowed"));
      }
      else
      {
        if (0 != (HTuple(int(hv_ClassIDsNoOrientation!=HTuple())).TupleAnd(int(((hv_ClassIDsNoOrientation.TupleGreaterEqualElem(0)).TupleSum())!=(hv_ClassIDsNoOrientation.TupleLength())))))
        {
          throw HException(("Invalid generic parameter for 'class_ids_no_orientation': "+hv_ClassIDsNoOrientation)+HTuple(", only non-negative integers are allowed"));
        }
      }
    }
  }
  //
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Compute 3D normals. 
void compute_normals_xyz (HObject ho_x, HObject ho_y, HObject ho_z, HObject *ho_NXImage, 
    HObject *ho_NYImage, HObject *ho_NZImage, HTuple hv_Smoothing)
{

  // Local iconic variables
  HObject  ho_xScaled, ho_yScaled, ho_zScaled, ho_xDiffRow;
  HObject  ho_xDiffCol, ho_yDiffRow, ho_yDiffCol, ho_zDiffRow;
  HObject  ho_zDiffCol, ho_ImageResult, ho_ImageResult2, ho_NXRaw;
  HObject  ho_NYRaw, ho_NZRaw, ho_NXSquare, ho_NYSquare, ho_NZSquare;
  HObject  ho_ImageResult1, ho_SqrtImage;

  // Local control variables
  HTuple  hv_Factor, hv_MaskRow, hv_MaskCol;

  //For numerical reasons we scale the input data
  hv_Factor = 1e6;
  ScaleImage(ho_x, &ho_xScaled, hv_Factor, 0);
  ScaleImage(ho_y, &ho_yScaled, hv_Factor, 0);
  ScaleImage(ho_z, &ho_zScaled, hv_Factor, 0);

  //Filter for diffs in row/col direction
  hv_MaskRow.Clear();
  hv_MaskRow[0] = 2;
  hv_MaskRow[1] = 1;
  hv_MaskRow[2] = 1.0;
  hv_MaskRow[3] = 1;
  hv_MaskRow[4] = -1;
  hv_MaskCol.Clear();
  hv_MaskCol[0] = 1;
  hv_MaskCol[1] = 2;
  hv_MaskCol[2] = 1.0;
  hv_MaskCol[3] = -1;
  hv_MaskCol[4] = 1;
  ConvolImage(ho_xScaled, &ho_xDiffRow, hv_MaskRow, "continued");
  ConvolImage(ho_xScaled, &ho_xDiffCol, hv_MaskCol, "continued");
  ConvolImage(ho_yScaled, &ho_yDiffRow, hv_MaskRow, "continued");
  ConvolImage(ho_yScaled, &ho_yDiffCol, hv_MaskCol, "continued");
  ConvolImage(ho_zScaled, &ho_zDiffRow, hv_MaskRow, "continued");
  ConvolImage(ho_zScaled, &ho_zDiffCol, hv_MaskCol, "continued");
  //
  //Calculate normal as cross product
  MultImage(ho_yDiffRow, ho_zDiffCol, &ho_ImageResult, 1.0, 0);
  MultImage(ho_zDiffRow, ho_yDiffCol, &ho_ImageResult2, -1.0, 0);
  AddImage(ho_ImageResult, ho_ImageResult2, &ho_NXRaw, 1.0, 0);
  //
  MultImage(ho_xDiffRow, ho_zDiffCol, &ho_ImageResult, -1.0, 0);
  MultImage(ho_zDiffRow, ho_xDiffCol, &ho_ImageResult2, 1.0, 0);
  AddImage(ho_ImageResult, ho_ImageResult2, &ho_NYRaw, 1.0, 0);
  //
  MultImage(ho_xDiffRow, ho_yDiffCol, &ho_ImageResult, 1.0, 0);
  MultImage(ho_yDiffRow, ho_xDiffCol, &ho_ImageResult2, -1.0, 0);
  AddImage(ho_ImageResult, ho_ImageResult2, &ho_NZRaw, 1.0, 0);

  //Smooth
  //-> 5 is used as it is used in surface_normals_object_model_3d - 'xyz_mapping'
  if (0 != hv_Smoothing)
  {
    MeanImage(ho_NXRaw, &ho_NXRaw, 5, 5);
    MeanImage(ho_NYRaw, &ho_NYRaw, 5, 5);
    MeanImage(ho_NZRaw, &ho_NZRaw, 5, 5);
  }

  //Normalize
  MultImage(ho_NXRaw, ho_NXRaw, &ho_NXSquare, 1.0, 0);
  MultImage(ho_NYRaw, ho_NYRaw, &ho_NYSquare, 1.0, 0);
  MultImage(ho_NZRaw, ho_NZRaw, &ho_NZSquare, 1.0, 0);
  AddImage(ho_NXSquare, ho_NYSquare, &ho_ImageResult1, 1.0, 0);
  AddImage(ho_ImageResult1, ho_NZSquare, &ho_ImageResult2, 1.0, 0);
  SqrtImage(ho_ImageResult2, &ho_SqrtImage);
  //
  DivImage(ho_NXRaw, ho_SqrtImage, &(*ho_NXImage), 1.0, 0);
  DivImage(ho_NYRaw, ho_SqrtImage, &(*ho_NYImage), 1.0, 0);
  DivImage(ho_NZRaw, ho_SqrtImage, &(*ho_NZImage), 1.0, 0);
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: This procedure converts Deep OCR Detection results to an Object Detection results. 
void convert_ocr_detection_result_to_object_detection (HTuple hv_OcrResults, HTuple *hv_DetectionResults)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_Index, hv_OcrResult, hv_RequiredKeysExist;
  HTuple  hv_DetectionResult, hv___Tmp_Ctrl_Type;

  //
  //Convert Deep OCR Detection results
  //to Object Detection results.
  //
  //Create DetectionResults Dict
  TupleGenConst(hv_OcrResults.TupleLength(), HTuple::TupleConstant("HNULL"), &(*hv_DetectionResults));
  {
  HTuple end_val6 = (hv_OcrResults.TupleLength())-1;
  HTuple step_val6 = 1;
  for (hv_Index=0; hv_Index.Continue(end_val6, step_val6); hv_Index += step_val6)
  {
    hv_OcrResult = HTuple(hv_OcrResults[hv_Index]);
    //Check if input is valid
    GetDictParam(hv_OcrResult, "key_exists", "words", &hv_RequiredKeysExist);
    if (0 != (int((hv_RequiredKeysExist.TupleSum())!=(hv_RequiredKeysExist.TupleLength()))))
    {
      throw HException(("The item at Index "+hv_Index)+" is not a valid Deep OCR Detection Result");
    }
    GetDictParam(hv_OcrResult.TupleGetDictTuple("words"), "key_exists", ((((HTuple("row").Append("col")).Append("phi")).Append("length1")).Append("length2")), 
        &hv_RequiredKeysExist);
    if (0 != (int((hv_RequiredKeysExist.TupleSum())!=(hv_RequiredKeysExist.TupleLength()))))
    {
      throw HException(("The item at Index "+hv_Index)+" is not a valid Deep OCR Detection Result");
    }
    //Convert ocr detection result to object detection
    CreateDict(&hv_DetectionResult);
    GetDictParam(hv_OcrResult.TupleGetDictTuple("words"), "key_data_type", "row", 
        &hv___Tmp_Ctrl_Type);
    if (0 != (int(hv___Tmp_Ctrl_Type==HTuple("object"))))
    {
      SetDictObject((hv_OcrResult.TupleGetDictTuple("words")).TupleGetDictObject("row"), 
          hv_DetectionResult, "bbox_row");
    }
    else
    {
      SetDictTuple(hv_DetectionResult, "bbox_row", (hv_OcrResult.TupleGetDictTuple("words")).TupleGetDictTuple("row"));
    }
    GetDictParam(hv_OcrResult.TupleGetDictTuple("words"), "key_data_type", "col", 
        &hv___Tmp_Ctrl_Type);
    if (0 != (int(hv___Tmp_Ctrl_Type==HTuple("object"))))
    {
      SetDictObject((hv_OcrResult.TupleGetDictTuple("words")).TupleGetDictObject("col"), 
          hv_DetectionResult, "bbox_col");
    }
    else
    {
      SetDictTuple(hv_DetectionResult, "bbox_col", (hv_OcrResult.TupleGetDictTuple("words")).TupleGetDictTuple("col"));
    }
    GetDictParam(hv_OcrResult.TupleGetDictTuple("words"), "key_data_type", "phi", 
        &hv___Tmp_Ctrl_Type);
    if (0 != (int(hv___Tmp_Ctrl_Type==HTuple("object"))))
    {
      SetDictObject((hv_OcrResult.TupleGetDictTuple("words")).TupleGetDictObject("phi"), 
          hv_DetectionResult, "bbox_phi");
    }
    else
    {
      SetDictTuple(hv_DetectionResult, "bbox_phi", (hv_OcrResult.TupleGetDictTuple("words")).TupleGetDictTuple("phi"));
    }
    GetDictParam(hv_OcrResult.TupleGetDictTuple("words"), "key_data_type", "length1", 
        &hv___Tmp_Ctrl_Type);
    if (0 != (int(hv___Tmp_Ctrl_Type==HTuple("object"))))
    {
      SetDictObject((hv_OcrResult.TupleGetDictTuple("words")).TupleGetDictObject("length1"), 
          hv_DetectionResult, "bbox_length1");
    }
    else
    {
      SetDictTuple(hv_DetectionResult, "bbox_length1", (hv_OcrResult.TupleGetDictTuple("words")).TupleGetDictTuple("length1"));
    }
    GetDictParam(hv_OcrResult.TupleGetDictTuple("words"), "key_data_type", "length2", 
        &hv___Tmp_Ctrl_Type);
    if (0 != (int(hv___Tmp_Ctrl_Type==HTuple("object"))))
    {
      SetDictObject((hv_OcrResult.TupleGetDictTuple("words")).TupleGetDictObject("length2"), 
          hv_DetectionResult, "bbox_length2");
    }
    else
    {
      SetDictTuple(hv_DetectionResult, "bbox_length2", (hv_OcrResult.TupleGetDictTuple("words")).TupleGetDictTuple("length2"));
    }
    SetDictTuple(hv_DetectionResult, "bbox_confidence", HTuple((hv_DetectionResult.TupleGetDictTuple("bbox_row")).TupleLength(),1.0));
    SetDictTuple(hv_DetectionResult, "bbox_class_id", HTuple((hv_DetectionResult.TupleGetDictTuple("bbox_row")).TupleLength(),0));
    (*hv_DetectionResults)[hv_Index] = hv_DetectionResult;
  }
  }


  return;
}

// Chapter: Tools / Geometry
// Short Description: Convert the parameters of rectangles with format rectangle2 to the coordinates of its 4 corner-points. 
void convert_rect2_5to8param (HTuple hv_Row, HTuple hv_Col, HTuple hv_Length1, HTuple hv_Length2, 
    HTuple hv_Phi, HTuple *hv_Row1, HTuple *hv_Col1, HTuple *hv_Row2, HTuple *hv_Col2, 
    HTuple *hv_Row3, HTuple *hv_Col3, HTuple *hv_Row4, HTuple *hv_Col4)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_Co1, hv_Co2, hv_Si1, hv_Si2;

  //This procedure takes the parameters for a rectangle of type 'rectangle2'
  //and returns the coordinates of the four corners.
  //
  hv_Co1 = (hv_Phi.TupleCos())*hv_Length1;
  hv_Co2 = (hv_Phi.TupleCos())*hv_Length2;
  hv_Si1 = (hv_Phi.TupleSin())*hv_Length1;
  hv_Si2 = (hv_Phi.TupleSin())*hv_Length2;

  (*hv_Col1) = (hv_Co1-hv_Si2)+hv_Col;
  (*hv_Row1) = ((-hv_Si1)-hv_Co2)+hv_Row;
  (*hv_Col2) = ((-hv_Co1)-hv_Si2)+hv_Col;
  (*hv_Row2) = (hv_Si1-hv_Co2)+hv_Row;
  (*hv_Col3) = ((-hv_Co1)+hv_Si2)+hv_Col;
  (*hv_Row3) = (hv_Si1+hv_Co2)+hv_Row;
  (*hv_Col4) = (hv_Co1+hv_Si2)+hv_Col;
  (*hv_Row4) = ((-hv_Si1)+hv_Co2)+hv_Row;

  return;
}

// Chapter: Tools / Geometry
// Short Description: Convert for four-sided figures the coordinates of the 4 corner-points to the parameters of format rectangle2. 
void convert_rect2_8to5param (HTuple hv_Row1, HTuple hv_Col1, HTuple hv_Row2, HTuple hv_Col2, 
    HTuple hv_Row3, HTuple hv_Col3, HTuple hv_Row4, HTuple hv_Col4, HTuple hv_ForceL1LargerL2, 
    HTuple *hv_Row, HTuple *hv_Col, HTuple *hv_Length1, HTuple *hv_Length2, HTuple *hv_Phi)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_Hor, hv_Vert, hv_IdxSwap, hv_Tmp;

  //This procedure takes the corners of four-sided figures
  //and returns the parameters of type 'rectangle2'.
  //
  //Calculate center row and column.
  (*hv_Row) = (((hv_Row1+hv_Row2)+hv_Row3)+hv_Row4)/4.0;
  (*hv_Col) = (((hv_Col1+hv_Col2)+hv_Col3)+hv_Col4)/4.0;
  //Length1 and Length2.
  (*hv_Length1) = ((((hv_Row1-hv_Row2)*(hv_Row1-hv_Row2))+((hv_Col1-hv_Col2)*(hv_Col1-hv_Col2))).TupleSqrt())/2.0;
  (*hv_Length2) = ((((hv_Row2-hv_Row3)*(hv_Row2-hv_Row3))+((hv_Col2-hv_Col3)*(hv_Col2-hv_Col3))).TupleSqrt())/2.0;
  //Calculate the angle phi.
  hv_Hor = hv_Col1-hv_Col2;
  hv_Vert = hv_Row2-hv_Row1;
  if (0 != hv_ForceL1LargerL2)
  {
    //Swap length1 and length2 if necessary.
    hv_IdxSwap = (((*hv_Length2)-(*hv_Length1)).TupleGreaterElem(1e-9)).TupleFind(1);
    if (0 != (int(hv_IdxSwap!=-1)))
    {
      hv_Tmp = HTuple((*hv_Length1)[hv_IdxSwap]);
      (*hv_Length1)[hv_IdxSwap] = HTuple((*hv_Length2)[hv_IdxSwap]);
      (*hv_Length2)[hv_IdxSwap] = hv_Tmp;
      hv_Hor[hv_IdxSwap] = HTuple(hv_Col2[hv_IdxSwap])-HTuple(hv_Col3[hv_IdxSwap]);
      hv_Vert[hv_IdxSwap] = HTuple(hv_Row3[hv_IdxSwap])-HTuple(hv_Row2[hv_IdxSwap]);
    }
  }
  (*hv_Phi) = hv_Vert.TupleAtan2(hv_Hor);
  //
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Crops a given image object based on the given domain handling. 
void crop_dl_sample_image (HObject ho_Domain, HTuple hv_DLSample, HTuple hv_Key, 
    HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho___Tmp_Obj_0;

  // Local control variables
  HTuple  hv_KeyExists, hv_Row1, hv_Column1, hv_Row2;
  HTuple  hv_Column2, hv___Tmp_Ctrl_Dict_Init_0;

  GetDictParam(hv_DLSample, "key_exists", hv_Key, &hv_KeyExists);
  if (0 != hv_KeyExists)
  {
    CreateDict(&hv___Tmp_Ctrl_Dict_Init_0);
    SetDictTuple(hv___Tmp_Ctrl_Dict_Init_0, "comp", "crop_domain");
    if (0 != ((hv_DLPreprocessParam.TupleConcat(hv___Tmp_Ctrl_Dict_Init_0)).TupleTestEqualDictItem("domain_handling","comp")))
    {
      SmallestRectangle1(ho_Domain, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);
      CropPart(hv_DLSample.TupleGetDictObject(hv_Key), &ho___Tmp_Obj_0, hv_Row1, 
          hv_Column1, (hv_Column2-hv_Column1)+1, (hv_Row2-hv_Row1)+1);
      SetDictObject(ho___Tmp_Obj_0, hv_DLSample, hv_Key);
    }
    hv___Tmp_Ctrl_Dict_Init_0 = HTuple::TupleConstant("HNULL");
  }
  return;
}

// Chapter: Graphics / Window
// Short Description: Close all window handles contained in a dictionary. 
void dev_close_window_dict (HTuple hv_WindowHandleDict)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_WindowHandleDicts, hv_WindowDictIdx;
  HTuple  hv_WindowHandleKeys, hv_Index, hv_WindowHandles;
  HTuple  hv_Exception, hv_RemovedWindowIndices, hv_WindowHandleIndex;

  //
  //This procedure closes all window handles
  //that are contained in the dictionary WindowHandleDict.
  //
  hv_WindowHandleDicts = hv_WindowHandleDict;
  {
  HTuple end_val5 = (hv_WindowHandleDicts.TupleLength())-1;
  HTuple step_val5 = 1;
  for (hv_WindowDictIdx=0; hv_WindowDictIdx.Continue(end_val5, step_val5); hv_WindowDictIdx += step_val5)
  {
    hv_WindowHandleDict = HTuple(hv_WindowHandleDicts[hv_WindowDictIdx]);
    GetDictParam(hv_WindowHandleDict, "keys", HTuple(), &hv_WindowHandleKeys);
    {
    HTuple end_val8 = (hv_WindowHandleKeys.TupleLength())-1;
    HTuple step_val8 = 1;
    for (hv_Index=0; hv_Index.Continue(end_val8, step_val8); hv_Index += step_val8)
    {
      try
      {
        GetDictTuple(hv_WindowHandleDict, HTuple(hv_WindowHandleKeys[hv_Index]), 
            &hv_WindowHandles);
      }
      // catch (Exception) 
      catch (HException &HDevExpDefaultException)
      {
        HDevExpDefaultException.ToHTuple(&hv_Exception);
        continue;
      }
      hv_RemovedWindowIndices = HTuple();
      {
      HTuple end_val15 = (hv_WindowHandles.TupleLength())-1;
      HTuple step_val15 = 1;
      for (hv_WindowHandleIndex=0; hv_WindowHandleIndex.Continue(end_val15, step_val15); hv_WindowHandleIndex += step_val15)
      {
        //Not every entry has to be a window handle, therefore use try-catch.
        try
        {
          //Call set_window_param to check if the handle is a window handle.
          SetWindowParam(HTuple(hv_WindowHandles[hv_WindowHandleIndex]), "flush", 
              "true");
          HDevWindowStack::SetActive(HTuple(hv_WindowHandles[hv_WindowHandleIndex]));
          if (HDevWindowStack::IsOpen())
            CloseWindow(HDevWindowStack::Pop());
          hv_RemovedWindowIndices = hv_RemovedWindowIndices.TupleConcat(hv_WindowHandleIndex);
        }
        // catch (Exception) 
        catch (HException &HDevExpDefaultException)
        {
          HDevExpDefaultException.ToHTuple(&hv_Exception);
        }
      }
      }
      TupleRemove(hv_WindowHandles, hv_RemovedWindowIndices, &hv_WindowHandles);
      //If some entries remained, set reduced tuple. Otherwise, remove whole key entry.
      if (0 != (int((hv_WindowHandles.TupleLength())>0)))
      {
        SetDictTuple(hv_WindowHandleDict, HTuple(hv_WindowHandleKeys[hv_Index]), 
            hv_WindowHandles);
      }
      else
      {
        RemoveDictKey(hv_WindowHandleDict, HTuple(hv_WindowHandleKeys[hv_Index]));
      }
    }
    }
  }
  }
  //
  return;
}

// Chapter: Graphics / Output
// Short Description: Display a map of the confidences. 
void dev_display_confidence_regions (HObject ho_ImageConfidence, HTuple hv_DrawTransparency, 
    HTuple *hv_Colors)
{

  // Local iconic variables
  HObject  ho_Region;

  // Local control variables
  HTuple  hv_NumColors, hv_WeightsColorsAlpha, hv_ColorIndex;
  HTuple  hv_Threshold, hv_MinGray, hv_MaxGray;

  //
  //This procedure displays a map of the confidences
  //given in ImageConfidence as regions.
  //DrawTransparency determines the alpha value of the colors.
  //The used colors are returned.
  //
  //Define colors.
  hv_NumColors = 20;
  get_distinct_colors(hv_NumColors, 0, 0, 100, &(*hv_Colors));
  hv_WeightsColorsAlpha = (*hv_Colors)+hv_DrawTransparency;
  hv_ColorIndex = 0;
  //
  //Threshold the image according to
  //the number of colors and
  //display resulting regions.
  {
  HTuple end_val15 = hv_NumColors-1;
  HTuple step_val15 = 1;
  for (hv_ColorIndex=0; hv_ColorIndex.Continue(end_val15, step_val15); hv_ColorIndex += step_val15)
  {
    hv_Threshold = hv_ColorIndex*(1.0/hv_NumColors);
    hv_MinGray = hv_Threshold;
    hv_MaxGray = hv_Threshold+(1/hv_NumColors);
    Threshold(ho_ImageConfidence, &ho_Region, hv_Threshold, hv_Threshold+(1.0/hv_NumColors));
    if (HDevWindowStack::IsOpen())
      SetColor(HDevWindowStack::GetActive(),HTuple(hv_WeightsColorsAlpha[hv_ColorIndex]));
    if (HDevWindowStack::IsOpen())
      DispObj(ho_Region, HDevWindowStack::GetActive());
  }
  }
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Visualize different images, annotations and inference results for a sample. 
void dev_display_dl_data (HTuple hv_DLSample, HTuple hv_DLResult, HTuple hv_DLDatasetInfo, 
    HTuple hv_KeysForDisplay, HTuple hv_GenParam, HTuple hv_WindowHandleDict)
{

  // Local iconic variables
  HObject  ho_Image, ho_Domain, ho_GrippingMapImageResult;
  HObject  ho_Cross, ho_GrippingMapGroundTruth, ho_X, ho_Y;
  HObject  ho_Z, ho_AnomalyImage, ho_AnomalyRegion, ho_ConfidenceImage;
  HObject  ho_HeatmapScene, ho_ScoreMaps, ho_CharacterScoreMap;
  HObject  ho_LinkScoreMap, ho_OrientationSinScoreMap, ho_OrientationCosScoreMap;
  HObject  ho_ScoreMap, ho_OrientationScoreMap, ho_PredictionColorFrame;
  HObject  ho_ImageHeatmap, ho_PredictionSymbol, ho_CrossLineH;
  HObject  ho_CrossLineV, ho_ImageConfidence, ho_SegmentationImagGroundTruth;
  HObject  ho_SegmentationImageResult, ho_ImageAbsDiff, ho_DiffRegion;
  HObject  ho_ImageWeight;

  // Local control variables
  HTuple  hv_Params, hv_GenParamNames, hv_ParamIndex;
  HTuple  hv_GenParamName, hv_KeyExists, hv_SampleKeys, hv_ResultKeys;
  HTuple  hv_ImageIDExists, hv_ImageID, hv_ImageIDString;
  HTuple  hv_ImageIDStringBraces, hv_ImageIDStringCapital;
  HTuple  hv_IsOCRDetection, hv_AdditionalGreenClassNames;
  HTuple  hv_KeyIndex, hv_OcrResult, hv_MaxClassIdSample;
  HTuple  hv_EmptySample, hv_MaxClassIdResult, hv_EmptyResult;
  HTuple  hv_MaxClassId, hv_ClassNames, hv_ClassIDs, hv_ClassNameKey;
  HTuple  hv_ClassIdKey, hv_ResultClassNames, hv_ResultClassIds;
  HTuple  hv_SortIndices, hv_UniqueClassIds, hv_UniqueClassNames;
  HTuple  hv_Colors, hv_ClassesLegend, hv_InvalidInput, hv_ClassKeys;
  HTuple  hv_ClassKeysExist, hv_DLDatasetInfoKeys, hv_Index;
  HTuple  hv_ClassIDstoIndex, hv_I, hv_PrevWindowCoordinates;
  HTuple  hv_Keys, hv_Exception, hv_MetaInfoExists, hv_FlushValues;
  HTuple  hv_WindowHandleKeys, hv_WindowHandles, hv_WindowIndex;
  HTuple  hv_FlushValue, hv_WidthImage, hv_HeightImage, hv_CurrentWindowHandle;
  HTuple  hv_WindowImageRatio, hv_ColorsGrippingGroundTruth;
  HTuple  hv_ImageClassIDs, hv_GrippingPointsExists, hv_Rows;
  HTuple  hv_Columns, hv_AnomalyImages, hv_PossibleKeysForDisplay;
  HTuple  hv_AddDisplayKey, hv_AnomalyLabelGroundTruth, hv_AnomalyLabelIDGroundTruth;
  HTuple  hv_AnomalyResultPostfix, hv_AnomalyScore, hv_AnomalyClassID;
  HTuple  hv_AnomalyClassThresholdDisplay, hv_AnomalyRegionThresholdDisplay;
  HTuple  hv_WindowHandleName, hv_AnomalyRegionGroundTruthExists;
  HTuple  hv_Text, hv_PredictionColor, hv_LineColors, hv_ResultColorOffset;
  HTuple  hv_AnomalyRegionExists, hv_AnomalyImageKey, hv_AnomalyScoreKey;
  HTuple  hv_AnomalyResultKey, hv_AnomalyRegionKey, hv_DisplayDirectionTemp;
  HTuple  hv_BboxLabelIndex, hv_BboxConfidences, hv_TextConf;
  HTuple  hv_BboxClassIndex, hv_BboxColorsBoth, hv_BboxClassLabelIndexUniq;
  HTuple  hv_BboxIDs, hv_BboxColors, hv_BboxIDsUniq, hv_BboxColorsResults;
  HTuple  hv_BboxClassIndexUniq, hv_ClassificationLabelIDGroundTruth;
  HTuple  hv_ClassificationLabelIDResult, hv_PredictionText;
  HTuple  hv_BoarderOffset, hv_MetaInfo, hv_WindowImageRatioHeight;
  HTuple  hv_WindowImageRatioWidth, hv_BoarderOffsetRow, hv_BoarderOffsetCol;
  HTuple  hv_MarginBottom, hv_WindowCoordinates, hv_CurrentWindowHeight;
  HTuple  hv__, hv_MaxHeight, hv_PluralPostfix, hv_ClassificationLabelIDGroundTruthList;
  HTuple  hv_C, hv_ClassificationLabelIDResultList, hv_HasConfidences;
  HTuple  hv_TPIndices, hv_RIdx, hv_ResultLine, hv_HasFalseNegatives;
  HTuple  hv_FalseNegativeClassIDs, hv_FNIdx, hv_FNClassIdx;
  HTuple  hv_FNLine, hv_NumFNLines, hv_IsMLC, hv_ResultColors;
  HTuple  hv_SelectedHeatmapMethod, hv_DictHeatmap, hv_MethodName;
  HTuple  hv_HeatmapKeys, hv_HeatmapImageName, hv_TargetClassID;
  HTuple  hv_Confidences, hv_MaxDeviation, hv_ClassificationLabelNameResult;
  HTuple  hv_TargetClassConfidence, hv_ClassificationLabelNamesGroundTruth;
  HTuple  hv_ShowGT, hv_ShowResult, hv_NumLines, hv_Type;
  HTuple  hv_GTWordKeyExists, hv_HeightWindow, hv_HeightMarginBottom;
  HTuple  hv_Size, hv_Length, hv_Row, hv_Column, hv_HomMat2DIdentity;
  HTuple  hv_HomMat2DRotate, hv_HomMat2DCompose, hv_PredictionForegroundColor;
  HTuple  hv_PredictionBackgroundColor, hv_Spaces, hv_ConfidenceColors;
  HTuple  hv_ColorsResults, hv_GroundTruthIDs, hv_ResultIDs;
  HTuple  hv_ImageClassIDsUniq, hv_ImageClassIDsIndices, hv_ImageClassIDsIndex;
  HTuple  hv_StringSegExcludeClassIDs, hv_StringIndex, hv_Min;
  HTuple  hv_Max, hv_Range, hv_ColorsSegmentation, hv_DrawMode;
  HTuple  hv_Width, hv_MinWeight, hv_WeightsColors, hv_Indices;
  HTuple  hv_WindowHandleKeysNew, hv___Tmp_Ctrl_Dict_Init_2;
  HTuple  hv___Tmp_Ctrl_Dict_Init_3, hv___Tmp_Ctrl_Dict_Init_4;
  HTuple  hv___Tmp_Ctrl_Dict_Init_5, hv___Tmp_Ctrl_Dict_Init_6;
  HTuple  hv___Tmp_Ctrl_Dict_Init_7, hv___Tmp_Ctrl_Dict_Init_8;
  HTuple  hv___Tmp_Ctrl_1, hv___Tmp_Ctrl_Type;

  //
  //This procedure displays the content of the provided DLSample and/or DLResult
  //depending on the input string KeysForDisplay.
  //DLDatasetInfo is a dictionary containing the information about the dataset.
  //The visualization can be adapted with GenParam.
  //
  //** Set the default values: ***
  CreateDict(&hv_Params);
  //
  //Define the screen width when a new window row is started.
  SetDictTuple(hv_Params, "threshold_width", 1024);
  //Since potentially a lot of windows are opened,
  //scale the windows consistently.
  SetDictTuple(hv_Params, "scale_windows", 0.8);
  //Set a font and a font size.
  SetDictTuple(hv_Params, "font", "mono");
  SetDictTuple(hv_Params, "font_size", 14);
  //
  SetDictTuple(hv_Params, "line_width", 2);
  SetDictTuple(hv_Params, "map_transparency", "cc");
  SetDictTuple(hv_Params, "map_color_bar_width", 140);
  //
  //Define parameter values specifically for 3d_gripping_point_detection
  SetDictTuple(hv_Params, "gripping_point_color", "#00FF0099");
  SetDictTuple(hv_Params, "gripping_point_size", 6);
  SetDictTuple(hv_Params, "region_color", "#FF000040");
  SetDictTuple(hv_Params, "gripping_point_map_color", "#83000080");
  SetDictTuple(hv_Params, "gripping_point_background_color", "#00007F80");
  //
  //Define parameter values specifically for anomaly detection
  //and Global Context Anomaly Detection.
  SetDictTuple(hv_Params, "anomaly_region_threshold", -1);
  SetDictTuple(hv_Params, "anomaly_classification_threshold", -1);
  SetDictTuple(hv_Params, "anomaly_region_label_color", "#40e0d0");
  SetDictTuple(hv_Params, "anomaly_color_transparency", "40");
  SetDictTuple(hv_Params, "anomaly_region_result_color", "#ff0000c0");
  //
  //Define segmentation-specific parameter values.
  SetDictTuple(hv_Params, "segmentation_max_weight", 0);
  SetDictTuple(hv_Params, "segmentation_draw", "fill");
  SetDictTuple(hv_Params, "segmentation_transparency", "aa");
  SetDictTuple(hv_Params, "segmentation_exclude_class_ids", HTuple());
  //
  //Define bounding box-specific parameter values.
  SetDictTuple(hv_Params, "bbox_label_color", HTuple("#000000")+"99");
  SetDictTuple(hv_Params, "bbox_display_confidence", 1);
  SetDictTuple(hv_Params, "bbox_text_color", "#eeeeee");
  //
  //By default, display a description on the bottom.
  SetDictTuple(hv_Params, "display_bottom_desc", 1);
  //
  //By default, show a legend with class IDs.
  SetDictTuple(hv_Params, "display_legend", 1);
  //
  //By default, show the anomaly ground truth regions.
  SetDictTuple(hv_Params, "display_ground_truth_anomaly_regions", 1);
  //
  //By default, show class IDs and color frames for classification ground truth/results.
  SetDictTuple(hv_Params, "display_classification_ids", 1);
  SetDictTuple(hv_Params, "display_classification_color_frame", 1);
  //
  //By default, show class labels for detection ground truth/results.
  SetDictTuple(hv_Params, "display_labels", 1);
  //
  //By default, show direction of the ground truth/results instances for detection with instance_type 'rectangle2'.
  SetDictTuple(hv_Params, "display_direction", 1);
  //
  //By default, use color scheme 'Jet' for the heatmap display.
  SetDictTuple(hv_Params, "heatmap_color_scheme", "jet");
  //** Set user-defined values: ***
  //
  //Overwrite default values by given generic parameters.
  if (0 != (int(hv_GenParam!=HTuple())))
  {
    GetDictParam(hv_GenParam, "keys", HTuple(), &hv_GenParamNames);
    {
    HTuple end_val74 = (hv_GenParamNames.TupleLength())-1;
    HTuple step_val74 = 1;
    for (hv_ParamIndex=0; hv_ParamIndex.Continue(end_val74, step_val74); hv_ParamIndex += step_val74)
    {
      hv_GenParamName = HTuple(hv_GenParamNames[hv_ParamIndex]);
      GetDictParam(hv_Params, "key_exists", hv_GenParamName, &hv_KeyExists);
      if (0 != (hv_KeyExists.TupleNot()))
      {
        throw HException(("Unknown generic parameter: "+hv_GenParamName)+".");
      }
      GetDictParam(hv_GenParam, "key_data_type", hv_GenParamName, &hv___Tmp_Ctrl_Type);
      if (0 != (int(hv___Tmp_Ctrl_Type==HTuple("object"))))
      {
        SetDictObject(hv_GenParam.TupleGetDictObject(hv_GenParamName), hv_Params, 
            hv_GenParamName);
      }
      else
      {
        SetDictTuple(hv_Params, hv_GenParamName, hv_GenParam.TupleGetDictTuple(hv_GenParamName));
      }
    }
    }
  }
  //
  if (0 != (HTuple(int((hv_DLSample.TupleLength())>1)).TupleOr(int((hv_DLResult.TupleLength())>1))))
  {
    throw HException("Only a single dictionary for DLSample and DLResult is allowed");
  }
  //
  //Get the dictionary keys.
  GetDictParam(hv_DLSample, "keys", HTuple(), &hv_SampleKeys);
  if (0 != (int(hv_DLResult!=HTuple())))
  {
    GetDictParam(hv_DLResult, "keys", HTuple(), &hv_ResultKeys);
  }
  //
  //Get image ID if it is available.
  GetDictParam(hv_DLSample, "key_exists", "image_id", &hv_ImageIDExists);
  if (0 != hv_ImageIDExists)
  {
    GetDictTuple(hv_DLSample, "image_id", &hv_ImageID);
    hv_ImageIDString = "image ID "+hv_ImageID;
    hv_ImageIDStringBraces = ("(image ID "+hv_ImageID)+")";
    hv_ImageIDStringCapital = "Image ID "+hv_ImageID;
  }
  else
  {
    hv_ImageIDString = "";
    hv_ImageIDStringBraces = hv_ImageIDString;
    hv_ImageIDStringCapital = hv_ImageIDString;
  }
  //
  //** Convert a Deep OCR Detection result to an Object Detection result if necessary ***
  //
  hv_IsOCRDetection = 0;
  hv_AdditionalGreenClassNames = HTuple();
  hv_KeyIndex = 0;
  {
  HTuple end_val117 = (hv_KeysForDisplay.TupleLength())-1;
  HTuple step_val117 = 1;
  for (hv_KeyIndex=0; hv_KeyIndex.Continue(end_val117, step_val117); hv_KeyIndex += step_val117)
  {
    //Check if Deep OCR Detection case
    TupleStrstr(HTuple(hv_KeysForDisplay[hv_KeyIndex]), "ocr_detection", &hv_IsOCRDetection);
    hv_IsOCRDetection = int(hv_IsOCRDetection==0);
    if (0 != hv_IsOCRDetection)
    {
      //Turn off labels
      SetDictTuple(hv_Params, "bbox_display_confidence", 0);
      SetDictTuple(hv_Params, "display_labels", 0);
      //Mark the class 'word' as green
      hv_AdditionalGreenClassNames = "word";
      if (0 != (int(hv_DLResult!=HTuple())))
      {
        hv_OcrResult = hv_DLResult;
        convert_ocr_detection_result_to_object_detection(hv_OcrResult, &hv_DLResult);
        GetDictParam(hv_DLResult, "keys", HTuple(), &hv_ResultKeys);
        break;
      }
    }
  }
  }
  //
  //Check if DLDatasetInfo is valid.
  //
  if (0 != (int(hv_DLDatasetInfo==HTuple())))
  {
    dev_display_dl_data_get_max_class_id(hv_DLSample, &hv_MaxClassIdSample, &hv_EmptySample);
    dev_display_dl_data_get_max_class_id(hv_DLResult, &hv_MaxClassIdResult, &hv_EmptyResult);
    hv_MaxClassId = hv_MaxClassIdSample.TupleMax2(hv_MaxClassIdResult);
    if (0 != (hv_EmptySample.TupleAnd(hv_EmptyResult)))
    {
      hv_MaxClassId = 1;
    }
    if (0 != (int(hv_MaxClassId==-1)))
    {
      hv_MaxClassId = 1000;
    }
    if (0 != hv_IsOCRDetection)
    {
      hv_ClassNames.Clear();
      hv_ClassNames[0] = "word";
      hv_ClassNames[1] = "char";
      hv_ClassNames[2] = "ignore";
      TupleGenSequence(0, (hv_ClassNames.TupleLength())-1, 1, &hv_ClassIDs);
    }
    else
    {
      TupleGenSequence(0, hv_MaxClassId, 1, &hv_ClassIDs);
      TupleGenConst(hv_MaxClassId+1, "unknown", &hv_ClassNames);
    }
    //Try to get the class names from the result dictionary.
    //This works only for detection and classification results.
    if (0 != (int(hv_DLResult!=HTuple())))
    {
      TupleRegexpSelect(hv_ResultKeys, ".*class_name.*", &hv_ClassNameKey);
      TupleRegexpSelect(hv_ResultKeys, ".*class_id.*", &hv_ClassIdKey);
      if (0 != (HTuple(int((hv_ClassNameKey.TupleLength())==1)).TupleAnd(int((hv_ClassIdKey.TupleLength())==1))))
      {
        GetDictTuple(hv_DLResult, hv_ClassNameKey, &hv_ResultClassNames);
        GetDictTuple(hv_DLResult, hv_ClassIdKey, &hv_ResultClassIds);
        TupleSortIndex(hv_ResultClassIds, &hv_SortIndices);
        TupleUniq(HTuple(hv_ResultClassIds[hv_SortIndices]), &hv_UniqueClassIds);
        TupleUniq(HTuple(hv_ResultClassNames[hv_SortIndices]), &hv_UniqueClassNames);
        hv_ClassNames[hv_UniqueClassIds] = hv_UniqueClassNames;
      }
    }
    get_dl_class_colors(hv_ClassNames, hv_AdditionalGreenClassNames, &hv_Colors);
    hv_ClassesLegend = (hv_ClassIDs+" : ")+hv_ClassNames;
    hv_InvalidInput = 0;
  }
  else
  {
    //Check if DLDatasetInfo contains necessary keys.
    hv_ClassKeys.Clear();
    hv_ClassKeys[0] = "class_names";
    hv_ClassKeys[1] = "class_ids";
    GetHandleParam(hv_DLDatasetInfo, "key_exists", hv_ClassKeys, &hv_ClassKeysExist);
    if (0 != (int((hv_ClassKeysExist.TupleMin())==0)))
    {
      //In that case we expect that the class names and ids are never used.
    }
    else
    {
      GetHandleParam(hv_DLDatasetInfo, "keys", HTuple(), &hv_DLDatasetInfoKeys);
      {
      HTuple end_val180 = (hv_ClassKeys.TupleLength())-1;
      HTuple step_val180 = 1;
      for (hv_Index=0; hv_Index.Continue(end_val180, step_val180); hv_Index += step_val180)
      {
        if (0 != (int((hv_DLDatasetInfoKeys.TupleFindFirst(HTuple(hv_ClassKeys[hv_Index])))==-1)))
        {
          throw HException(("Key "+HTuple(hv_ClassKeys[hv_Index]))+" is missing in DLDatasetInfo.");
        }
      }
      }
      //
      //Get the general dataset information, if available.
      GetHandleTuple(hv_DLDatasetInfo, "class_names", &hv_ClassNames);
      GetHandleTuple(hv_DLDatasetInfo, "class_ids", &hv_ClassIDs);
      //
      //Define distinct colors for the classes.
      get_dl_class_colors(hv_ClassNames, hv_AdditionalGreenClassNames, &hv_Colors);
      //
      hv_ClassesLegend = (hv_ClassIDs+" : ")+hv_ClassNames;
      //Create a Classes map so that we index the other tuples easily
      CreateDict(&hv_ClassIDstoIndex);
      {
      HTuple end_val196 = (hv_ClassIDs.TupleLength())-1;
      HTuple step_val196 = 1;
      for (hv_I=0; hv_I.Continue(end_val196, step_val196); hv_I += step_val196)
      {
        SetDictTuple(hv_ClassIDstoIndex, HTuple(hv_ClassIDs[hv_I]), hv_I);
      }
      }
    }
  }
  //
  //** Set window parameters: ***
  //
  //Set previous window coordinates.
  hv_PrevWindowCoordinates.Clear();
  hv_PrevWindowCoordinates[0] = 0;
  hv_PrevWindowCoordinates[1] = 0;
  hv_PrevWindowCoordinates[2] = 0;
  hv_PrevWindowCoordinates[3] = 0;
  //
  //Check that the WindowHandleDict is of type dictionary.
  try
  {
    GetDictParam(hv_WindowHandleDict, "keys", HTuple(), &hv_Keys);
  }
  // catch (Exception) 
  catch (HException &HDevExpDefaultException)
  {
    HDevExpDefaultException.ToHTuple(&hv_Exception);
    if (0 != (int(HTuple(hv_Exception[0])==1401)))
    {
      throw HException("WindowHandleDict has to be of type dictionary. Use create_dict to create an empty dictionary.");
    }
    else
    {
      throw HException(hv_Exception);
    }
  }
  //For better usage, add meta information about the window handles in WindowHandleDict.
  GetDictParam(hv_WindowHandleDict, "key_exists", "meta_information", &hv_MetaInfoExists);
  if (0 != (hv_MetaInfoExists.TupleNot()))
  {
    CreateDict(&hv___Tmp_Ctrl_Dict_Init_2);
    SetDictTuple(hv_WindowHandleDict, "meta_information", hv___Tmp_Ctrl_Dict_Init_2);
    hv___Tmp_Ctrl_Dict_Init_2 = HTuple::TupleConstant("HNULL");
  }
  //
  //For each window, set 'flush' to 'false' to avoid flickering.
  hv_FlushValues = HTuple();
  GetDictParam(hv_WindowHandleDict, "keys", HTuple(), &hv_WindowHandleKeys);
  {
  HTuple end_val228 = (hv_WindowHandleKeys.TupleLength())-1;
  HTuple step_val228 = 1;
  for (hv_Index=0; hv_Index.Continue(end_val228, step_val228); hv_Index += step_val228)
  {
    //Only consider the WindowHandleKeys that are needed for the current visualization.
    hv_KeyIndex = hv_KeysForDisplay.TupleFind(HTuple(hv_WindowHandleKeys[hv_Index]));
    if (0 != (HTuple(int(hv_KeyIndex!=-1)).TupleAnd(int(hv_KeyIndex!=HTuple()))))
    {
      GetDictTuple(hv_WindowHandleDict, HTuple(hv_WindowHandleKeys[hv_Index]), &hv_WindowHandles);
      {
      HTuple end_val233 = (hv_WindowHandles.TupleLength())-1;
      HTuple step_val233 = 1;
      for (hv_WindowIndex=0; hv_WindowIndex.Continue(end_val233, step_val233); hv_WindowIndex += step_val233)
      {
        GetWindowParam(HTuple(hv_WindowHandles[hv_WindowIndex]), "flush", &hv_FlushValue);
        hv_FlushValues = hv_FlushValues.TupleConcat(hv_FlushValue);
        SetWindowParam(HTuple(hv_WindowHandles[hv_WindowIndex]), "flush", "false");
      }
      }
    }
  }
  }
  //
  //** Display the data: ***
  //
  //Display data dictionaries.
  hv_KeyIndex = 0;
  while (0 != (int(hv_KeyIndex<(hv_KeysForDisplay.TupleLength()))))
  {
    //
    //Is it an Deep OCR detection case?
    TupleStrstr(HTuple(hv_KeysForDisplay[hv_KeyIndex]), "ocr_detection", &hv_IsOCRDetection);
    hv_IsOCRDetection = int(hv_IsOCRDetection==0);
    //
    if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("image"))))
    {
      //
      //Image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, HTuple(hv_KeysForDisplay[hv_KeyIndex]));
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      FullDomain(ho_Image, &ho_Image);
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("image_with_domain"))))
    {
      //
      //Image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      GetDomain(ho_Image, &ho_Domain);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      if (HDevWindowStack::IsOpen())
        SetColor(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("region_color"));
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Domain, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("normals"))))
    {
      //
      //Normal image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, HTuple(hv_KeysForDisplay[hv_KeyIndex]));
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      FullDomain(ho_Image, &ho_Image);
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("gripping_confidence"))))
    {
      //
      //Confidence image.
      if (0 != (int(hv_DLResult==HTuple())))
      {
        throw HException("DLResult dict is empty.");
      }
      if (0 != (int((hv_ResultKeys.TupleFind(HTuple(hv_KeysForDisplay[hv_KeyIndex])))!=-1)))
      {
        GetDictObject(&ho_Image, hv_DLResult, HTuple(hv_KeysForDisplay[hv_KeyIndex]));
      }
      else
      {
        throw HException(("Image with key '"+HTuple(hv_KeysForDisplay[hv_KeyIndex]))+"' could not be found in DLResult.");
      }
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      FullDomain(ho_Image, &ho_Image);
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
      if (HDevWindowStack::IsOpen())
        DispText(HDevWindowStack::GetActive(),"Gripping confidence", "window", "top", 
            "left", "black", "box", "true");
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("gripping_map"))))
    {
      //
      //Image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Gripping map image result.
      if (0 != (int(hv_DLResult==HTuple())))
      {
        throw HException("DLResult dict is empty.");
      }
      if (0 != (int((hv_ResultKeys.TupleFind("gripping_map"))!=-1)))
      {
        GetDictObject(&ho_GrippingMapImageResult, hv_DLResult, "gripping_map");
      }
      else
      {
        throw HException(HTuple(HTuple("Image with key '")+"gripping_map")+"' could not be found in DLResult.");
      }
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      SetWindowParam(hv_CurrentWindowHandle, "background_color", "#000000");
      if (HDevWindowStack::IsOpen())
        ClearWindow(HDevWindowStack::GetActive());
      GetDomain(ho_GrippingMapImageResult, &ho_Domain);
      ReduceDomain(ho_Image, ho_Domain, &ho_Image);
      //
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //Display gripping regions.
      hv_ColorsGrippingGroundTruth.Clear();
      hv_ColorsGrippingGroundTruth.Append(hv_Params.TupleGetDictTuple("gripping_point_map_color"));
      hv_ColorsGrippingGroundTruth.Append(hv_Params.TupleGetDictTuple("gripping_point_background_color"));
      dev_display_segmentation_regions(ho_GrippingMapImageResult, (HTuple(1).Append(0)), 
          hv_ColorsGrippingGroundTruth, HTuple(), &hv_ImageClassIDs);
      //Display gripping points.
      GetDictParam(hv_DLResult, "key_exists", "gripping_points", &hv_GrippingPointsExists);
      if (0 != hv_GrippingPointsExists)
      {
        get_gripping_points_from_dict(hv_DLResult, &hv_Rows, &hv_Columns);
        GenCrossContourXld(&ho_Cross, hv_Rows, hv_Columns, hv_Params.TupleGetDictTuple("gripping_point_size"), 
            0.785398);
        if (HDevWindowStack::IsOpen())
          SetLineWidth(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("line_width"));
        if (HDevWindowStack::IsOpen())
          SetColor(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("gripping_point_color"));
        if (HDevWindowStack::IsOpen())
          DispObj(ho_Cross, HDevWindowStack::GetActive());
      }
      //
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
      //
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("gripping_map_ground_truth"))))
    {
      //
      //Image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Gripping map ground truth.
      if (0 != (int((hv_SampleKeys.TupleFind("segmentation_image"))!=-1)))
      {
        GetDictObject(&ho_GrippingMapGroundTruth, hv_DLSample, "segmentation_image");
      }
      else
      {
        throw HException(HTuple(HTuple("Image with key '")+"segmentation_image")+"' could not be found in DLSample.");
      }
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      //Display gripping regions.
      hv_ColorsGrippingGroundTruth.Clear();
      hv_ColorsGrippingGroundTruth.Append(hv_Params.TupleGetDictTuple("gripping_point_map_color"));
      hv_ColorsGrippingGroundTruth.Append(hv_Params.TupleGetDictTuple("gripping_point_background_color"));
      dev_display_segmentation_regions(ho_GrippingMapGroundTruth, hv_ClassIDs, hv_ColorsGrippingGroundTruth, 
          HTuple(), &hv_ImageClassIDs);
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
      if (HDevWindowStack::IsOpen())
        DispText(HDevWindowStack::GetActive(),"Gripping map ground truth", "window", 
            "top", "left", "black", "box", "true");
      //
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("x"))))
    {
      //
      //X.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, HTuple(hv_KeysForDisplay[hv_KeyIndex]));
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      FullDomain(ho_Image, &ho_Image);
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("xyz"))))
    {
      //
      //XYZ.
      get_dl_sample_image(&ho_X, hv_SampleKeys, hv_DLSample, "x");
      get_dl_sample_image(&ho_Y, hv_SampleKeys, hv_DLSample, "y");
      get_dl_sample_image(&ho_Z, hv_SampleKeys, hv_DLSample, "z");
      Compose3(ho_X, ho_Y, ho_Z, &ho_Image);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      FullDomain(ho_Image, &ho_Image);
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("y"))))
    {
      //
      //Y.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, HTuple(hv_KeysForDisplay[hv_KeyIndex]));
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      FullDomain(ho_Image, &ho_Image);
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("z"))))
    {
      //
      //Z.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, HTuple(hv_KeysForDisplay[hv_KeyIndex]));
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      FullDomain(ho_Image, &ho_Image);
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("gripping_point_cloud"))))
    {
      //
      //In case of 'gripping_point_cloud' provide a window that can be used by the procedure
      //dev_display_dl_3d_data. No actual drawing happens  in this procedure to keep it
      //free from operators not belonging either to the Foundation or Deep Learning
      //license modules.
      GetDictParam(hv_DLSample, "keys", HTuple(), &hv_SampleKeys);
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Do nothing here and draw later
      //
    }
    else if (0 != (HTuple(HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_both"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_both_local")))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_both_global")))))
    {
      //
      //In case of 'anomaly_both', also plot 'anomaly_both_local'
      //and 'anomaly_both_global', if available.
      TupleRegexpSelect(hv_ResultKeys, ".*anomaly_image.*", &hv_AnomalyImages);
      if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_both"))))
      {
        TupleRegexpReplace(hv_AnomalyImages, "image", "both", &hv_PossibleKeysForDisplay);
        hv_AddDisplayKey = 0;
        {
        HTuple end_val469 = (hv_PossibleKeysForDisplay.TupleLength())-1;
        HTuple step_val469 = 1;
        for (hv_Index=0; hv_Index.Continue(end_val469, step_val469); hv_Index += step_val469)
        {
          if (0 != (int((hv_KeysForDisplay.TupleFindFirst(HTuple(hv_PossibleKeysForDisplay[hv_Index])))==-1)))
          {
            hv_KeysForDisplay = hv_KeysForDisplay.TupleConcat(HTuple(hv_PossibleKeysForDisplay[hv_Index]));
            hv_AddDisplayKey = 1;
          }
        }
        }
        //
        //Display not possible for 'anomaly_both' if key 'anomaly_image' is missing.
        if (0 != (HTuple(int((hv_PossibleKeysForDisplay.TupleFindFirst(HTuple(hv_KeysForDisplay[hv_KeyIndex])))==-1)).TupleAnd(hv_AddDisplayKey)))
        {
          hv_KeyIndex += 1;
          continue;
        }
      }
      //
      //Get image and ground truth.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      get_anomaly_ground_truth_label(hv_SampleKeys, hv_DLSample, &hv_AnomalyLabelGroundTruth, 
          &hv_AnomalyLabelIDGroundTruth);
      //
      //Get the anomaly results either by applying the specified thresholds or out of DLResult.
      TupleRegexpMatch(HTuple(hv_KeysForDisplay[hv_KeyIndex]), "anomaly_both(.*)", 
          &hv_AnomalyResultPostfix);
      get_anomaly_result(&ho_AnomalyImage, &ho_AnomalyRegion, hv_DLResult, hv_Params.TupleGetDictTuple("anomaly_classification_threshold"), 
          hv_Params.TupleGetDictTuple("anomaly_region_threshold"), hv_AnomalyResultPostfix, 
          &hv_AnomalyScore, &hv_AnomalyClassID, &hv_AnomalyClassThresholdDisplay, 
          &hv_AnomalyRegionThresholdDisplay);
      //
      //Get open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      hv_WindowHandleName = HTuple(hv_KeysForDisplay[hv_KeyIndex]);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, hv_WindowHandleName, &hv_CurrentWindowHandle, 
          &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualize image, ground truth (if available), and result regions.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      hv_AnomalyRegionGroundTruthExists = "false";
      if (0 != (hv_Params.TupleGetDictTuple("display_ground_truth_anomaly_regions")))
      {
        dev_display_ground_truth_anomaly_regions(hv_SampleKeys, hv_DLSample, hv_CurrentWindowHandle, 
            hv_Params.TupleGetDictTuple("line_width"), hv_Params.TupleGetDictTuple("anomaly_region_label_color"), 
            hv_Params.TupleGetDictTuple("anomaly_color_transparency"), &hv_AnomalyRegionGroundTruthExists);
      }
      //
      //Display anomaly regions defined by the specified threshold or out of DLResult.
      CreateDict(&hv___Tmp_Ctrl_Dict_Init_3);
      SetDictTuple(hv___Tmp_Ctrl_Dict_Init_3, "comp", -1);
      if (0 != (HTuple(((hv_Params.TupleConcat(hv___Tmp_Ctrl_Dict_Init_3)).TupleTestEqualDictItem("anomaly_region_threshold","comp")).TupleNot()).TupleOr(int((hv_ResultKeys.TupleFind("anomaly_region"+hv_AnomalyResultPostfix))!=-1))))
      {
        dev_display_result_anomaly_regions(ho_AnomalyRegion, hv_CurrentWindowHandle, 
            hv_Params.TupleGetDictTuple("line_width"), hv_Params.TupleGetDictTuple("anomaly_region_result_color"));
      }
      hv___Tmp_Ctrl_Dict_Init_3 = HTuple::TupleConstant("HNULL");
      //
      hv_Text = "GT and detected anomalies "+hv_ImageIDStringBraces;
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Get prediction color.
      hv_PredictionColor = "white";
      if (0 != (int(hv_AnomalyLabelIDGroundTruth==hv_AnomalyClassID)))
      {
        hv_PredictionColor = "green";
      }
      else if (0 != (int(hv_AnomalyClassID!=-1)))
      {
        hv_PredictionColor = "red";
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = "---------------";
        hv_Text[hv_Text.TupleLength()] = "Ground truth ";
        hv_Text[hv_Text.TupleLength()] = "---------------";
        hv_Text[hv_Text.TupleLength()] = ((hv_AnomalyLabelIDGroundTruth+" : '")+hv_AnomalyLabelGroundTruth)+"'";
        if (0 != (HTuple(HTuple(int(hv_AnomalyRegionGroundTruthExists==HTuple("false"))).TupleAnd(int(hv_AnomalyLabelIDGroundTruth==1))).TupleAnd(hv_Params.TupleGetDictTuple("display_ground_truth_anomaly_regions"))))
        {
          hv_Text[hv_Text.TupleLength()] = "";
          hv_Text[hv_Text.TupleLength()] = "No segmentation ground truth found";
        }
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = "---------------";
        hv_Text[hv_Text.TupleLength()] = "Results ";
        hv_Text[hv_Text.TupleLength()] = "---------------";
        if (0 != (int(hv_AnomalyClassID==1)))
        {
          hv_Text[hv_Text.TupleLength()] = hv_AnomalyClassID+" : 'nok'";
        }
        else if (0 != (int(hv_AnomalyClassID==0)))
        {
          hv_Text[hv_Text.TupleLength()] = hv_AnomalyClassID+" : 'ok'";
        }
        else
        {
          hv_Text[hv_Text.TupleLength()] = "No classification result found";
        }
        CreateDict(&hv___Tmp_Ctrl_Dict_Init_4);
        SetDictTuple(hv___Tmp_Ctrl_Dict_Init_4, "comp", -1);
        if (0 != (((hv_Params.TupleConcat(hv___Tmp_Ctrl_Dict_Init_4)).TupleTestEqualDictItem("anomaly_region_threshold","comp")).TupleAnd(int((hv_ResultKeys.TupleFind("anomaly_region"+hv_AnomalyResultPostfix))==-1))))
        {
          hv_Text[hv_Text.TupleLength()] = "";
          hv_Text[hv_Text.TupleLength()] = "No segmentation result found";
        }
        hv___Tmp_Ctrl_Dict_Init_4 = HTuple::TupleConstant("HNULL");
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = (("anomaly_score"+hv_AnomalyResultPostfix)+": ")+(hv_AnomalyScore.TupleString(".3f"));
        hv_Text[hv_Text.TupleLength()] = "";
        if (0 != (HTuple(int(hv_AnomalyClassThresholdDisplay!=-1)).TupleOr(int(hv_AnomalyRegionThresholdDisplay!=-1))))
        {
          hv_Text[hv_Text.TupleLength()] = "---------------";
          hv_Text[hv_Text.TupleLength()] = "Thresholds ";
          hv_Text[hv_Text.TupleLength()] = "---------------";
        }
        //
        if (0 != (int(hv_AnomalyClassThresholdDisplay!=-1)))
        {
          hv_Text[hv_Text.TupleLength()] = "Classification: "+(hv_AnomalyClassThresholdDisplay.TupleString(".3f"));
          hv_Text[hv_Text.TupleLength()] = "";
        }
        if (0 != (int(hv_AnomalyRegionThresholdDisplay!=-1)))
        {
          hv_Text[hv_Text.TupleLength()] = "Segmentation: "+(hv_AnomalyRegionThresholdDisplay.TupleString(".3f"));
          hv_Text[hv_Text.TupleLength()] = "";
        }
        //Get or open next child window
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, hv_WindowHandleName, 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        TupleGenConst(hv_Text.TupleLength(), "white", &hv_LineColors);
        hv_ResultColorOffset = 10;
        if (0 != (HTuple(HTuple(int(hv_AnomalyRegionGroundTruthExists==HTuple("false"))).TupleAnd(int(hv_AnomalyLabelIDGroundTruth==1))).TupleAnd(hv_Params.TupleGetDictTuple("display_ground_truth_anomaly_regions"))))
        {
          hv_ResultColorOffset += 2;
        }
        hv_LineColors[hv_ResultColorOffset] = hv_PredictionColor;
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              hv_LineColors, "box", "false");
      }
      //
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_ground_truth"))))
    {
      //Image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      get_anomaly_ground_truth_label(hv_SampleKeys, hv_DLSample, &hv_AnomalyLabelGroundTruth, 
          &hv_AnomalyLabelIDGroundTruth);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      hv_AnomalyRegionExists = "false";
      if (0 != (hv_Params.TupleGetDictTuple("display_ground_truth_anomaly_regions")))
      {
        //Show the ground truth region.
        dev_display_ground_truth_anomaly_regions(hv_SampleKeys, hv_DLSample, hv_CurrentWindowHandle, 
            hv_Params.TupleGetDictTuple("line_width"), hv_Params.TupleGetDictTuple("anomaly_region_label_color"), 
            hv_Params.TupleGetDictTuple("anomaly_color_transparency"), &hv_AnomalyRegionExists);
      }
      //
      hv_Text = "Ground truth anomalies "+hv_ImageIDStringBraces;
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = ((hv_AnomalyLabelIDGroundTruth+" : '")+hv_AnomalyLabelGroundTruth)+"'";
        if (0 != (HTuple(HTuple(int(hv_AnomalyRegionExists==HTuple("false"))).TupleAnd(int(hv_AnomalyLabelIDGroundTruth==1))).TupleAnd(hv_Params.TupleGetDictTuple("display_ground_truth_anomaly_regions"))))
        {
          hv_Text[hv_Text.TupleLength()] = "";
          hv_Text[hv_Text.TupleLength()] = "No 'anomaly_ground_truth' exists!";
        }
        //
        //Get or open next child window
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              "white", "box", "false");
      }
    }
    else if (0 != (HTuple(HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_image"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_image_local")))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_image_global")))))
    {
      //
      //Image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Get the anomaly results either by applying the specified thresholds or out of DLResult.
      hv_AnomalyImageKey = HTuple(hv_KeysForDisplay[hv_KeyIndex]);
      TupleRegexpMatch(hv_AnomalyImageKey, "anomaly_image(.*)", &hv_AnomalyResultPostfix);
      get_anomaly_result(&ho_AnomalyImage, &ho_AnomalyRegion, hv_DLResult, hv_Params.TupleGetDictTuple("anomaly_classification_threshold"), 
          hv_Params.TupleGetDictTuple("anomaly_region_threshold"), hv_AnomalyResultPostfix, 
          &hv_AnomalyScore, &hv_AnomalyClassID, &hv_AnomalyClassThresholdDisplay, 
          &hv_AnomalyRegionThresholdDisplay);
      //
      //Read in input image.
      GetDictObject(&ho_Image, hv_DLSample, "image");
      //Add the anomaly image to the input image.
      add_colormap_to_image(ho_AnomalyImage, ho_Image, &ho_AnomalyImage, hv_Params.TupleGetDictTuple("heatmap_color_scheme"));
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      if (HDevWindowStack::IsOpen())
        DispObj(ho_AnomalyImage, HDevWindowStack::GetActive());
      hv_Text = hv_AnomalyImageKey;
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      TupleRegexpReplace(hv_AnomalyImageKey, "image", "score", &hv_AnomalyScoreKey);
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_Text[hv_Text.TupleLength()] = "";
        if (0 != (int(hv_AnomalyClassID==1)))
        {
          hv_Text[hv_Text.TupleLength()] = hv_AnomalyClassID+" : 'nok'";
        }
        else if (0 != (int(hv_AnomalyClassID==0)))
        {
          hv_Text[hv_Text.TupleLength()] = hv_AnomalyClassID+" : 'ok'";
        }
        else
        {
          hv_Text[hv_Text.TupleLength()] = "No classification result found";
        }
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = (hv_AnomalyScoreKey+": ")+(hv_AnomalyScore.TupleString(".3f"));
        //Get or open next child window
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              "white", "box", "false");
      }
    }
    else if (0 != (HTuple(HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_result"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_result_local")))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("anomaly_result_global")))))
    {
      //
      //Get image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Get the anomaly results either by applying the specified thresholds or out of DLResult.
      hv_AnomalyResultKey = HTuple(hv_KeysForDisplay[hv_KeyIndex]);
      TupleRegexpMatch(hv_AnomalyResultKey, "anomaly_result(.*)", &hv_AnomalyResultPostfix);
      get_anomaly_result(&ho_AnomalyImage, &ho_AnomalyRegion, hv_DLResult, hv_Params.TupleGetDictTuple("anomaly_classification_threshold"), 
          hv_Params.TupleGetDictTuple("anomaly_region_threshold"), hv_AnomalyResultPostfix, 
          &hv_AnomalyScore, &hv_AnomalyClassID, &hv_AnomalyClassThresholdDisplay, 
          &hv_AnomalyRegionThresholdDisplay);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      //Display anomaly regions defined by the specified threshold or out of DLResult.
      TupleRegexpReplace(hv_AnomalyResultKey, "result", "region", &hv_AnomalyRegionKey);
      CreateDict(&hv___Tmp_Ctrl_Dict_Init_5);
      SetDictTuple(hv___Tmp_Ctrl_Dict_Init_5, "comp", -1);
      if (0 != (HTuple(((hv_Params.TupleConcat(hv___Tmp_Ctrl_Dict_Init_5)).TupleTestEqualDictItem("anomaly_region_threshold","comp")).TupleNot()).TupleOr(int((hv_ResultKeys.TupleFind(hv_AnomalyRegionKey))!=-1))))
      {
        dev_display_result_anomaly_regions(ho_AnomalyRegion, hv_CurrentWindowHandle, 
            hv_Params.TupleGetDictTuple("line_width"), hv_Params.TupleGetDictTuple("anomaly_region_result_color"));
      }
      hv___Tmp_Ctrl_Dict_Init_5 = HTuple::TupleConstant("HNULL");
      //
      TupleRegexpReplace(hv_AnomalyResultKey, "result", "score", &hv_AnomalyScoreKey);
      //
      hv_Text = "Detected anomalies "+hv_ImageIDStringBraces;
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = "---------------";
        hv_Text[hv_Text.TupleLength()] = "Results ";
        hv_Text[hv_Text.TupleLength()] = "---------------";
        if (0 != (int(hv_AnomalyClassID==1)))
        {
          hv_Text[hv_Text.TupleLength()] = hv_AnomalyClassID+" : 'nok'";
        }
        else if (0 != (int(hv_AnomalyClassID==0)))
        {
          hv_Text[hv_Text.TupleLength()] = hv_AnomalyClassID+" : 'ok'";
        }
        else
        {
          hv_Text[hv_Text.TupleLength()] = "No classification result found";
        }
        CreateDict(&hv___Tmp_Ctrl_Dict_Init_6);
        SetDictTuple(hv___Tmp_Ctrl_Dict_Init_6, "comp", -1);
        if (0 != (((hv_Params.TupleConcat(hv___Tmp_Ctrl_Dict_Init_6)).TupleTestEqualDictItem("anomaly_region_threshold","comp")).TupleAnd(int((hv_ResultKeys.TupleFind(hv_AnomalyRegionKey))==-1))))
        {
          hv_Text[hv_Text.TupleLength()] = "";
          hv_Text[hv_Text.TupleLength()] = "No segmentation result found";
        }
        hv___Tmp_Ctrl_Dict_Init_6 = HTuple::TupleConstant("HNULL");
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = (hv_AnomalyScoreKey+": ")+(hv_AnomalyScore.TupleString(".3f"));
        hv_Text[hv_Text.TupleLength()] = "";
        if (0 != (HTuple(int(hv_AnomalyClassThresholdDisplay!=-1)).TupleOr(int(hv_AnomalyRegionThresholdDisplay!=-1))))
        {
          hv_Text[hv_Text.TupleLength()] = "---------------";
          hv_Text[hv_Text.TupleLength()] = "Thresholds ";
          hv_Text[hv_Text.TupleLength()] = "---------------";
        }
        //
        if (0 != (int(hv_AnomalyClassThresholdDisplay!=-1)))
        {
          hv_Text[hv_Text.TupleLength()] = "Classification: "+(hv_AnomalyClassThresholdDisplay.TupleString(".3f"));
          hv_Text[hv_Text.TupleLength()] = "";
        }
        if (0 != (int(hv_AnomalyRegionThresholdDisplay!=-1)))
        {
          hv_Text[hv_Text.TupleLength()] = "Segmentation: "+(hv_AnomalyRegionThresholdDisplay.TupleString(".3f"));
          hv_Text[hv_Text.TupleLength()] = "";
        }
        //
        //Get or open next child window
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              "white", "box", "false");
      }
      //
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("gripping_heatmap"))))
    {
      //
      //Image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Get gripping points
      get_gripping_points_from_dict(hv_DLResult, &hv_Rows, &hv_Columns);
      //
      //Confidence image.
      if (0 != (int((hv_ResultKeys.TupleFind("gripping_confidence"))!=-1)))
      {
        GetDictObject(&ho_ConfidenceImage, hv_DLResult, "gripping_confidence");
      }
      else
      {
        throw HException("Image with key 'gripping_confidence' could not be found in DLResult.");
      }
      add_colormap_to_image(ho_ConfidenceImage, ho_Image, &ho_HeatmapScene, hv_Params.TupleGetDictTuple("heatmap_color_scheme"));
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      if (HDevWindowStack::IsOpen())
        DispObj(ho_HeatmapScene, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
      if (HDevWindowStack::IsOpen())
        DispText(HDevWindowStack::GetActive(),"Gripping heatmap", "window", "top", 
            "left", "black", "box", "true");
      //
      GenCrossContourXld(&ho_Cross, hv_Rows, hv_Columns, hv_Params.TupleGetDictTuple("gripping_point_size"), 
          0.785398);
      if (HDevWindowStack::IsOpen())
        SetLineWidth(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("line_width"));
      if (HDevWindowStack::IsOpen())
        SetColor(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("gripping_point_color"));
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Cross, HDevWindowStack::GetActive());
      //
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("gripping_points"))))
    {
      //
      //Image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Get gripping points
      get_gripping_points_from_dict(hv_DLResult, &hv_Rows, &hv_Columns);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      FullDomain(ho_Image, &ho_Image);
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_ImageIDStringCapital, "window", 
              "bottom", "left", "white", "box", "false");
      }
      if (HDevWindowStack::IsOpen())
        DispText(HDevWindowStack::GetActive(),"Gripping points", "window", "top", 
            "left", "black", "box", "true");
      //
      GenCrossContourXld(&ho_Cross, hv_Rows, hv_Columns, hv_Params.TupleGetDictTuple("gripping_point_size"), 
          0.785398);
      if (HDevWindowStack::IsOpen())
        SetLineWidth(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("line_width"));
      if (HDevWindowStack::IsOpen())
        SetColor(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("gripping_point_color"));
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Cross, HDevWindowStack::GetActive());
      //
    }
    else if (0 != (HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("bbox_both"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_detection_both")))))
    {
      //
      //Ground truth and result bounding boxes on image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      if (0 != hv_IsOCRDetection)
      {
        FullDomain(ho_Image, &ho_Image);
      }
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      //Visualization.
      //
      hv_DisplayDirectionTemp = hv_Params.TupleGetDictTuple("display_direction");
      if (0 != ((hv_Params.TupleGetDictTuple("display_direction")).TupleAnd(hv_IsOCRDetection)))
      {
        hv_DisplayDirectionTemp = -1;
      }
      dev_display_ground_truth_detection(hv_DLSample, hv_SampleKeys, hv_Params.TupleGetDictTuple("line_width"), 
          hv_ClassIDs, hv_Colors, hv_Params.TupleGetDictTuple("bbox_label_color"), 
          hv_WindowImageRatio, hv_Params.TupleGetDictTuple("bbox_text_color"), hv_Params.TupleGetDictTuple("display_labels"), 
          hv_DisplayDirectionTemp, hv_CurrentWindowHandle, &hv_BboxLabelIndex);
      if (0 != (int((hv_ResultKeys.TupleFind("bbox_confidence"))!=-1)))
      {
        GetDictTuple(hv_DLResult, "bbox_confidence", &hv_BboxConfidences);
      }
      else
      {
        throw HException("Result data could not be found in DLResult.");
      }
      if (0 != (hv_Params.TupleGetDictTuple("bbox_display_confidence")))
      {
        hv_TextConf = (" ("+(hv_BboxConfidences.TupleString(".2f")))+")";
      }
      else
      {
        hv_TextConf = HTuple(hv_BboxConfidences.TupleLength(),"");
      }
      dev_display_result_detection(hv_DLResult, hv_ResultKeys, hv_Params.TupleGetDictTuple("line_width"), 
          hv_ClassIDs, hv_TextConf, hv_Colors, hv_Params.TupleGetDictTuple("bbox_label_color"), 
          hv_WindowImageRatio, "bottom", hv_Params.TupleGetDictTuple("bbox_text_color"), 
          hv_Params.TupleGetDictTuple("display_labels"), hv_DisplayDirectionTemp, 
          hv_CurrentWindowHandle, &hv_BboxClassIndex);
      hv_Text = "Ground truth and results "+hv_ImageIDStringBraces;
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      hv_Text = "Ground truth and";
      hv_Text[hv_Text.TupleLength()] = "results "+hv_ImageIDStringBraces;
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_BboxColorsBoth.Clear();
        hv_BboxColorsBoth[0] = "white";
        hv_BboxColorsBoth[1] = "white";
        if (0 != (int(((hv_BboxClassIndex.TupleLength())+(hv_BboxLabelIndex.TupleLength()))>0)))
        {
          hv_BboxClassLabelIndexUniq = ((hv_BboxClassIndex.TupleConcat(hv_BboxLabelIndex)).TupleSort()).TupleUniq();
          hv_Text = hv_Text.TupleConcat(HTuple(hv_ClassesLegend[hv_BboxClassLabelIndexUniq]));
          hv_BboxColorsBoth = hv_BboxColorsBoth.TupleConcat(HTuple(hv_Colors[hv_BboxClassLabelIndexUniq]));
        }
        else
        {
          hv_Text = hv_Text.TupleConcat("No ground truth nor results present.");
        }
        //
        //Get or open next child window.
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              hv_BboxColorsBoth, "box", "false");
      }
    }
    else if (0 != (HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("bbox_ground_truth"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_detection_ground_truth")))))
    {
      //
      //Sample bounding boxes on image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (0 != hv_IsOCRDetection)
      {
        FullDomain(ho_Image, &ho_Image);
      }
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      //Display smaller arrow for ocr_detection
      hv_DisplayDirectionTemp = hv_Params.TupleGetDictTuple("display_direction");
      if (0 != ((hv_Params.TupleGetDictTuple("display_direction")).TupleAnd(hv_IsOCRDetection)))
      {
        hv_DisplayDirectionTemp = -1;
      }
      //
      dev_display_ground_truth_detection(hv_DLSample, hv_SampleKeys, hv_Params.TupleGetDictTuple("line_width"), 
          hv_ClassIDs, hv_Colors, hv_Params.TupleGetDictTuple("bbox_label_color"), 
          hv_WindowImageRatio, hv_Params.TupleGetDictTuple("bbox_text_color"), hv_Params.TupleGetDictTuple("display_labels"), 
          hv_DisplayDirectionTemp, hv_CurrentWindowHandle, &hv_BboxIDs);
      hv_Text = "Ground truth "+hv_ImageIDStringBraces;
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      //
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_BboxColors = "white";
        if (0 != (hv_BboxIDs.TupleLength()))
        {
          hv_BboxIDsUniq = (hv_BboxIDs.TupleSort()).TupleUniq();
          hv_Text = hv_Text.TupleConcat(HTuple(hv_ClassesLegend[hv_BboxIDsUniq]));
          hv_BboxColors = hv_BboxColors.TupleConcat(HTuple(hv_Colors[hv_BboxIDsUniq]));
        }
        else
        {
          hv_Text = hv_Text.TupleConcat("No ground truth present.");
        }
        //
        //Get or open next child window.
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              hv_BboxColors, "box", "false");
      }
    }
    else if (0 != (HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("bbox_result"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_detection_result")))))
    {
      //
      //Result bounding boxes on image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (0 != hv_IsOCRDetection)
      {
        FullDomain(ho_Image, &ho_Image);
      }
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      //Display smaller arrow for ocr_detection
      hv_DisplayDirectionTemp = hv_Params.TupleGetDictTuple("display_direction");
      if (0 != ((hv_Params.TupleGetDictTuple("display_direction")).TupleAnd(hv_IsOCRDetection)))
      {
        hv_DisplayDirectionTemp = -1;
      }
      //
      if (0 != (int((hv_ResultKeys.TupleFind("bbox_confidence"))!=-1)))
      {
        GetDictTuple(hv_DLResult, "bbox_confidence", &hv_BboxConfidences);
      }
      else
      {
        throw HException("Result data could not be found in DLResult.");
      }
      if (0 != (hv_Params.TupleGetDictTuple("bbox_display_confidence")))
      {
        hv_TextConf = (" ("+(hv_BboxConfidences.TupleString(".2f")))+")";
      }
      else
      {
        hv_TextConf = HTuple(hv_BboxConfidences.TupleLength(),"");
      }
      dev_display_result_detection(hv_DLResult, hv_ResultKeys, hv_Params.TupleGetDictTuple("line_width"), 
          hv_ClassIDs, hv_TextConf, hv_Colors, hv_Params.TupleGetDictTuple("bbox_label_color"), 
          hv_WindowImageRatio, "top", hv_Params.TupleGetDictTuple("bbox_text_color"), 
          hv_Params.TupleGetDictTuple("display_labels"), hv_DisplayDirectionTemp, 
          hv_CurrentWindowHandle, &hv_BboxClassIndex);
      hv_Text = "Result "+hv_ImageIDStringBraces;
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_BboxColorsResults = "white";
        if (0 != (int((hv_BboxClassIndex.TupleLength())>0)))
        {
          hv_BboxClassIndexUniq = (hv_BboxClassIndex.TupleSort()).TupleUniq();
          hv_Text = hv_Text.TupleConcat(HTuple(hv_ClassesLegend[hv_BboxClassIndexUniq]));
          hv_BboxColorsResults = hv_BboxColorsResults.TupleConcat(HTuple(hv_Colors[hv_BboxClassIndexUniq]));
        }
        else
        {
          hv_Text = hv_Text.TupleConcat("No results present.");
        }
        //
        //Get or open next child window.
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              hv_BboxColorsResults, "box", "false");
      }
    }
    else if (0 != (HTuple(HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_detection_score_map_character"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_detection_score_map_link")))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_detection_score_map_orientation")))))
    {
      //
      //Extract score maps
      ho_ScoreMaps = hv_OcrResult.TupleGetDictObject("score_maps");
      Decompose4(ho_ScoreMaps, &ho_CharacterScoreMap, &ho_LinkScoreMap, &ho_OrientationSinScoreMap, 
          &ho_OrientationCosScoreMap);
      //
      //Select score map to display
      if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_detection_score_map_character"))))
      {
        ho_ScoreMap = ho_CharacterScoreMap;
        hv_Text = "Character score "+hv_ImageIDStringBraces;
      }
      else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_detection_score_map_link"))))
      {
        ho_ScoreMap = ho_LinkScoreMap;
        hv_Text = "Link score "+hv_ImageIDStringBraces;
      }
      else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_detection_score_map_orientation"))))
      {
        Compose2(ho_OrientationSinScoreMap, ho_OrientationCosScoreMap, &ho_OrientationScoreMap
            );
        ho_ScoreMap = ho_OrientationScoreMap;
        hv_Text = HTuple("Orientation (sin,cos) ")+hv_ImageIDStringBraces;
      }
      //Get or open next window.
      GetImageSize(ho_ScoreMap, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Display the score maps using the 'jet' color map.
      SetLut(hv_CurrentWindowHandle, ((HTuple("jet").Append(0)).Append(1)));
      if (HDevWindowStack::IsOpen())
        DispObj(ho_ScoreMap, HDevWindowStack::GetActive());
      //
      //Display text
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
    }
    else if (0 != (HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("classification_both"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("multi_label_classification_both")))))
    {
      //
      //Ground truth and result classification image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      get_classification_ground_truth(hv_SampleKeys, hv_DLSample, &hv_ClassificationLabelIDGroundTruth);
      get_classification_result(hv_ResultKeys, hv_DLResult, &hv_ClassificationLabelIDResult);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Get prediction color.
      hv_PredictionColor = "white";
      if (0 != (int((hv_ClassificationLabelIDGroundTruth.TupleSort())==(hv_ClassificationLabelIDResult.TupleSort()))))
      {
        hv_PredictionText = "Correct";
        hv_PredictionColor = "green";
      }
      else
      {
        hv_PredictionText = "Wrong";
        hv_PredictionColor = "red";
      }
      //
      //Generate prediction color frame and show image.
      if (0 != (hv_Params.TupleGetDictTuple("display_classification_color_frame")))
      {
        //Create a frame with line width 7 that is completely displayed in the window.
        hv_BoarderOffset = 7/2.;
        GetDictTuple(hv_WindowHandleDict, "meta_information", &hv_MetaInfo);
        GetDictTuple(hv_MetaInfo, HTuple(hv_KeysForDisplay[hv_KeyIndex])+"_window_image_ratio_height", 
            &hv_WindowImageRatioHeight);
        GetDictTuple(hv_MetaInfo, HTuple(hv_KeysForDisplay[hv_KeyIndex])+"_window_image_ratio_width", 
            &hv_WindowImageRatioWidth);
        hv_BoarderOffsetRow = hv_BoarderOffset/hv_WindowImageRatioHeight;
        hv_BoarderOffsetCol = hv_BoarderOffset/hv_WindowImageRatioWidth;
        GenContourPolygonXld(&ho_PredictionColorFrame, ((((hv_BoarderOffsetRow-0.5).TupleConcat(hv_BoarderOffsetRow-0.5)).TupleConcat((hv_HeightImage+0.5)-hv_BoarderOffsetRow)).TupleConcat((hv_HeightImage+0.5)-hv_BoarderOffsetRow)).TupleConcat(hv_BoarderOffsetRow-0.5), 
            ((((hv_BoarderOffsetCol-0.5).TupleConcat((hv_WidthImage+0.5)-hv_BoarderOffsetCol)).TupleConcat((hv_WidthImage+0.5)-hv_BoarderOffsetCol)).TupleConcat(hv_BoarderOffsetCol-0.5)).TupleConcat(hv_BoarderOffsetCol-0.5));
        if (HDevWindowStack::IsOpen())
          SetLineWidth(HDevWindowStack::GetActive(),7);
        if (HDevWindowStack::IsOpen())
          SetColor(HDevWindowStack::GetActive(),hv_PredictionColor);
        if (HDevWindowStack::IsOpen())
          DispObj(ho_Image, HDevWindowStack::GetActive());
        if (HDevWindowStack::IsOpen())
          DispObj(ho_PredictionColorFrame, HDevWindowStack::GetActive());
      }
      else
      {
        if (HDevWindowStack::IsOpen())
          DispObj(ho_Image, HDevWindowStack::GetActive());
      }
      //
      if (0 != (hv_Params.TupleGetDictTuple("display_classification_ids")))
      {
        GetDictTuple(hv_WindowHandleDict, "meta_information", &hv_MetaInfo);
        GetDictTuple(hv_MetaInfo, HTuple(hv_KeysForDisplay[hv_KeyIndex])+"_margin_bottom", 
            &hv_MarginBottom);
        GetDictTuple(hv_MetaInfo, HTuple(hv_KeysForDisplay[hv_KeyIndex])+"_window_coordinates", 
            &hv_WindowCoordinates);
        hv_CurrentWindowHeight = HTuple(hv_WindowCoordinates[3])-HTuple(hv_WindowCoordinates[0]);
        GetFontExtents(hv_CurrentWindowHandle, &hv__, &hv__, &hv__, &hv_MaxHeight);
        if (0 != (int((hv_ClassificationLabelIDGroundTruth.TupleLength())>1)))
        {
          hv_PluralPostfix = "s";
          hv_ClassificationLabelIDGroundTruth = hv_ClassificationLabelIDGroundTruth.TupleSort();
        }
        else
        {
          hv_PluralPostfix = "";
        }
        hv_ClassificationLabelIDGroundTruthList = "";
        {
        HTuple end_val1023 = (hv_ClassificationLabelIDGroundTruth.TupleLength())-1;
        HTuple step_val1023 = 1;
        for (hv_C=0; hv_C.Continue(end_val1023, step_val1023); hv_C += step_val1023)
        {
          hv_ClassificationLabelIDGroundTruthList += HTuple(hv_ClassificationLabelIDGroundTruth[hv_C]);
          if (0 != (int(hv_C<((hv_ClassificationLabelIDGroundTruth.TupleLength())-1))))
          {
            hv_ClassificationLabelIDGroundTruthList += HTuple(HTuple(", "));
          }
        }
        }

        hv_Text = (("GT label ID"+hv_PluralPostfix)+": ")+hv_ClassificationLabelIDGroundTruthList;
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              "white", "box", "false");

        if (0 != (int((hv_ClassificationLabelIDResult.TupleLength())>1)))
        {
          hv_PluralPostfix = "s";
          hv_ClassificationLabelIDResult = hv_ClassificationLabelIDResult.TupleSort();
        }
        else
        {
          hv_PluralPostfix = "";
        }
        hv_ClassificationLabelIDResultList = "";
        {
        HTuple end_val1040 = (hv_ClassificationLabelIDResult.TupleLength())-1;
        HTuple step_val1040 = 1;
        for (hv_C=0; hv_C.Continue(end_val1040, step_val1040); hv_C += step_val1040)
        {
          hv_ClassificationLabelIDResultList += HTuple(hv_ClassificationLabelIDResult[hv_C]);
          if (0 != (int(hv_C<((hv_ClassificationLabelIDResult.TupleLength())-1))))
          {
            hv_ClassificationLabelIDResultList += HTuple(HTuple(", "));
          }
        }
        }
        hv_Text = (("Result class ID"+hv_PluralPostfix)+": ")+hv_ClassificationLabelIDResultList;
        if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
        {
          if (HDevWindowStack::IsOpen())
            DispText(HDevWindowStack::GetActive(),hv_Text, "window", hv_CurrentWindowHeight-((hv_MarginBottom+hv_MaxHeight)+10), 
                "left", "white", "box", "false");
        }
        else
        {
          if (HDevWindowStack::IsOpen())
            DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
                "white", "box", "false");
        }
      }
      //
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        hv_Text = "Result/GT classification "+hv_ImageIDStringBraces;
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        if (0 != (int((hv_ClassificationLabelIDGroundTruth.TupleLength())>1)))
        {
          hv_PluralPostfix = "s";
        }
        else
        {
          hv_PluralPostfix = "";
        }
        hv_Text = (("Ground truth class ID"+hv_PluralPostfix)+" ")+hv_ImageIDStringBraces;
        hv_Text = hv_Text.TupleConcat(HTuple(hv_ClassesLegend[hv_ClassIDstoIndex.TupleGetDictTuple(hv_ClassificationLabelIDGroundTruth)]));
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = "";
        if (0 != (int((hv_ClassificationLabelIDResult.TupleLength())>1)))
        {
          hv_PluralPostfix = "s";
        }
        else
        {
          hv_PluralPostfix = "";
        }
        hv_Text[hv_Text.TupleLength()] = "Result class ID"+hv_PluralPostfix;
        if (0 != (int(hv_ClassificationLabelIDResult==HTuple())))
        {
          hv_Text[hv_Text.TupleLength()] = "No classification result is given!";
        }
        GetDictParam(hv_DLResult, "key_exists", "selected_confidences", &hv_HasConfidences);
        if (0 != hv_HasConfidences)
        {
          hv_TPIndices = HTuple();
          {
          HTuple end_val1082 = ((hv_DLResult.TupleGetDictTuple("selected_class_ids")).TupleLength())-1;
          HTuple step_val1082 = 1;
          for (hv_RIdx=0; hv_RIdx.Continue(end_val1082, step_val1082); hv_RIdx += step_val1082)
          {
            hv_ResultLine = HTuple((hv_DLResult.TupleGetDictTuple("selected_class_ids"))[hv_RIdx]);
            hv_ResultLine = ((hv_ResultLine+" (")+(HTuple((hv_DLResult.TupleGetDictTuple("selected_confidences"))[hv_RIdx]).TupleString(".2f")))+") : ";
            hv_ResultLine += HTuple((hv_DLResult.TupleGetDictTuple("selected_class_names"))[hv_RIdx]);
            hv_Text = hv_Text.TupleConcat(hv_ResultLine);
            if (0 != (int(-1!=(hv_ClassificationLabelIDGroundTruth.TupleFind(HTuple((hv_DLResult.TupleGetDictTuple("selected_class_ids"))[hv_RIdx]))))))
            {
              hv_TPIndices = hv_TPIndices.TupleConcat(hv_RIdx);
            }
          }
          }
          //Check for false negatives and add those to the legend (only multi-label classification)
          hv_HasFalseNegatives = int(((hv_ClassificationLabelIDGroundTruth.TupleDifference(hv_DLResult.TupleGetDictTuple("selected_class_ids"))).TupleLength())>0);
          if (0 != hv_HasFalseNegatives)
          {
            hv_FalseNegativeClassIDs = hv_ClassificationLabelIDGroundTruth.TupleDifference(hv_DLResult.TupleGetDictTuple("selected_class_ids"));
            if (0 != (int((hv_FalseNegativeClassIDs.TupleLength())>1)))
            {
              hv_PluralPostfix = "s";
            }
            else
            {
              hv_PluralPostfix = "";
            }
            hv_Text = hv_Text.TupleConcat("");
            hv_Text = hv_Text.TupleConcat("False negative class ID"+hv_PluralPostfix);
            {
            HTuple end_val1102 = (hv_FalseNegativeClassIDs.TupleLength())-1;
            HTuple step_val1102 = 1;
            for (hv_FNIdx=0; hv_FNIdx.Continue(end_val1102, step_val1102); hv_FNIdx += step_val1102)
            {
              hv_FNClassIdx = hv_ClassIDstoIndex.TupleGetDictTuple(HTuple(hv_FalseNegativeClassIDs[hv_FNIdx]));
              hv_FNLine = HTuple((hv_DLResult.TupleGetDictTuple("class_ids"))[hv_FNClassIdx]);
              hv_FNLine = ((hv_FNLine+" (")+((HTuple((hv_DLResult.TupleGetDictTuple("confidences"))[hv_FNClassIdx]).TupleString("f")).TupleSubstr(0,3)))+") : ";
              hv_FNLine += HTuple((hv_DLResult.TupleGetDictTuple("class_names"))[hv_FNClassIdx]);
              hv_Text = hv_Text.TupleConcat(hv_FNLine);
            }
            }
            hv_NumFNLines = hv_FNIdx;
          }
        }
        else
        {
          hv_Text = hv_Text.TupleConcat(HTuple(hv_ClassesLegend[hv_ClassIDstoIndex.TupleGetDictTuple(hv_ClassificationLabelIDResult)]));
        }
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = "Prediction ";
        hv_Text[hv_Text.TupleLength()] = hv_PredictionText;
        //
        //Get or open next child window.
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        TupleGenConst(hv_Text.TupleLength(), "white", &hv_LineColors);
        GetDictParam(hv_DLSample, "key_exists", "image_label_ids", &hv_IsMLC);
        if (0 != hv_IsMLC)
        {
          TupleGenConst(hv_Colors.TupleLength(), "light gray", &hv_Colors);
          TupleGenConst((hv_DLResult.TupleGetDictTuple("selected_class_ids")).TupleLength(), 
              "red", &hv_ResultColors);
        }
        hv_LineColors[HTuple::TupleGenSequence(1,hv_ClassificationLabelIDGroundTruth.TupleLength(),1)] = HTuple(hv_Colors[hv_ClassIDstoIndex.TupleGetDictTuple(hv_ClassificationLabelIDGroundTruth)]);
        if (0 != (int(hv_ClassificationLabelIDResult!=HTuple())))
        {
          if (0 != hv_IsMLC)
          {
            hv_ResultColors[hv_TPIndices] = "green";
            hv_LineColors[HTuple::TupleGenSequence((5+(hv_ClassificationLabelIDGroundTruth.TupleLength()))-1,(((5+(hv_ClassificationLabelIDGroundTruth.TupleLength()))-1)+(hv_ClassificationLabelIDResult.TupleLength()))-1,1)] = hv_ResultColors;
          }
          else
          {
            hv_LineColors[HTuple::TupleGenSequence((5+(hv_ClassificationLabelIDGroundTruth.TupleLength()))-1,(((5+(hv_ClassificationLabelIDGroundTruth.TupleLength()))-1)+(hv_ClassificationLabelIDResult.TupleLength()))-1,1)] = HTuple(hv_Colors[hv_ClassIDstoIndex.TupleGetDictTuple(hv_ClassificationLabelIDResult)]);
          }
        }
        else
        {
          hv_LineColors[(5+(hv_ClassificationLabelIDGroundTruth.TupleLength()))-1] = "red";
        }
        hv_LineColors[(hv_LineColors.TupleLength())-1] = hv_PredictionColor;
        GetDictParam(hv_DLResult, "key_exists", "selected_confidences", &hv_HasConfidences);
        if (0 != hv_HasConfidences)
        {
          hv_HasFalseNegatives = int(((hv_ClassificationLabelIDGroundTruth.TupleDifference(hv_DLResult.TupleGetDictTuple("selected_class_ids"))).TupleLength())>0);
          if (0 != hv_HasFalseNegatives)
          {
            hv_LineColors[HTuple::TupleGenSequence(((hv_LineColors.TupleLength())-4)-hv_NumFNLines,(hv_LineColors.TupleLength())-4,1)] = "red";
          }
        }
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              hv_LineColors, "box", "false");
      }
    }
    else if (0 != (HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("classification_ground_truth"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("multi_label_classification_ground_truth")))))
    {
      //
      //Ground truth classification image and class label.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      get_classification_ground_truth(hv_SampleKeys, hv_DLSample, &hv_ClassificationLabelIDGroundTruth);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      if (0 != (int((hv_ClassificationLabelIDGroundTruth.TupleLength())>1)))
      {
        hv_PluralPostfix = "s";
        hv_ClassificationLabelIDGroundTruth = hv_ClassificationLabelIDGroundTruth.TupleSort();
      }
      else
      {
        hv_PluralPostfix = "";
      }
      //
      if (0 != (hv_Params.TupleGetDictTuple("display_classification_ids")))
      {
        hv_ClassificationLabelIDGroundTruthList = "";
        {
        HTuple end_val1171 = (hv_ClassificationLabelIDGroundTruth.TupleLength())-1;
        HTuple step_val1171 = 1;
        for (hv_C=0; hv_C.Continue(end_val1171, step_val1171); hv_C += step_val1171)
        {
          hv_ClassificationLabelIDGroundTruthList += HTuple(hv_ClassificationLabelIDGroundTruth[hv_C]);
          if (0 != (int(hv_C<((hv_ClassificationLabelIDGroundTruth.TupleLength())-1))))
          {
            hv_ClassificationLabelIDGroundTruthList += HTuple(HTuple(", "));
          }
        }
        }
        hv_Text = (("GT label ID"+hv_PluralPostfix)+": ")+hv_ClassificationLabelIDGroundTruthList;
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              "white", "box", "false");
      }
      //
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        hv_Text = "Ground truth classification "+hv_ImageIDStringBraces;
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_Text = (("Ground truth class ID"+hv_PluralPostfix)+" ")+hv_ImageIDStringBraces;
        hv_Text = hv_Text.TupleConcat(HTuple(hv_ClassesLegend[hv_ClassIDstoIndex.TupleGetDictTuple(hv_ClassificationLabelIDGroundTruth)]));
        //
        //Get or open next child window
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        GetDictParam(hv_DLSample, "key_exists", "image_label_ids", &hv_IsMLC);
        if (0 != hv_IsMLC)
        {
          TupleGenConst(hv_Colors.TupleLength(), "light gray", &hv_Colors);
        }
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              HTuple("white").TupleConcat(HTuple(hv_Colors[hv_ClassIDstoIndex.TupleGetDictTuple(hv_ClassificationLabelIDGroundTruth)])), 
              "box", "false");
      }
    }
    else if (0 != (HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("classification_result"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("multi_label_classification_result")))))
    {
      //
      //Ground truth classification image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      get_classification_result(hv_ResultKeys, hv_DLResult, &hv_ClassificationLabelIDResult);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      if (0 != (int((hv_ClassificationLabelIDResult.TupleLength())>1)))
      {
        hv_PluralPostfix = "s";
        hv_ClassificationLabelIDResult = hv_ClassificationLabelIDResult.TupleSort();
      }
      else
      {
        hv_PluralPostfix = "";
      }
      //
      //Display the class IDs.
      if (0 != (hv_Params.TupleGetDictTuple("display_classification_ids")))
      {
        GetDictTuple(hv_WindowHandleDict, "meta_information", &hv_MetaInfo);
        GetDictTuple(hv_MetaInfo, "classification_result_margin_bottom", &hv_MarginBottom);
        GetDictTuple(hv_MetaInfo, "classification_result_window_coordinates", &hv_WindowCoordinates);
        hv_CurrentWindowHeight = HTuple(hv_WindowCoordinates[3])-HTuple(hv_WindowCoordinates[0]);
        GetFontExtents(hv_CurrentWindowHandle, &hv__, &hv__, &hv__, &hv_MaxHeight);
        hv_ClassificationLabelIDResultList = "";
        {
        HTuple end_val1228 = (hv_ClassificationLabelIDResult.TupleLength())-1;
        HTuple step_val1228 = 1;
        for (hv_C=0; hv_C.Continue(end_val1228, step_val1228); hv_C += step_val1228)
        {
          hv_ClassificationLabelIDResultList += HTuple(hv_ClassificationLabelIDResult[hv_C]);
          if (0 != (int(hv_C<((hv_ClassificationLabelIDResult.TupleLength())-1))))
          {
            hv_ClassificationLabelIDResultList += HTuple(HTuple(", "));
          }
        }
        }
        hv_Text = (("Result class ID"+hv_PluralPostfix)+": ")+hv_ClassificationLabelIDResultList;
        if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
        {
          if (HDevWindowStack::IsOpen())
            DispText(HDevWindowStack::GetActive(),hv_Text, "window", hv_CurrentWindowHeight-((hv_MarginBottom+hv_MaxHeight)+10), 
                "left", "white", "box", "false");
        }
        else
        {
          if (HDevWindowStack::IsOpen())
            DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
                "white", "box", "false");
        }
      }
      //
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        hv_Text = "Result classification "+hv_ImageIDStringBraces;
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_Text = (("Result class ID"+hv_PluralPostfix)+" ")+hv_ImageIDStringBraces;
        if (0 != (int(hv_ClassificationLabelIDResult==HTuple())))
        {
          hv_Text[hv_Text.TupleLength()] = "No classification result is given!";
        }
        else
        {
          GetDictParam(hv_DLResult, "key_exists", "selected_class_ids", &hv_IsMLC);
          if (0 != hv_IsMLC)
          {
            TupleGenConst(hv_Colors.TupleLength(), "light gray", &hv_Colors);
          }
          hv_Text = hv_Text.TupleConcat(HTuple(hv_ClassesLegend[hv_ClassIDstoIndex.TupleGetDictTuple(hv_ClassificationLabelIDResult)]));
        }
        //
        //Get or open next child window
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              HTuple("white").TupleConcat(HTuple(hv_Colors[hv_ClassIDstoIndex.TupleGetDictTuple(hv_ClassificationLabelIDResult)])), 
              "box", "false");
      }
    }
    else if (0 != (HTuple(HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("heatmap_grad_cam"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("heatmap_guided_grad_cam")))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("heatmap_confidence_based")))))
    {
      //
      //Display the heatmap image (method 'heatmap_grad_cam', 'heatmap_guided_grad_cam',
      //or 'heatmap_confidence_based') in the selected color scheme.
      //Retrieve heatmap image, inferred image, and inference results.
      hv_SelectedHeatmapMethod = HTuple(hv_KeysForDisplay[hv_KeyIndex]);
      if (0 != (HTuple(HTuple(int((hv_ResultKeys.TupleFind("heatmap_grad_cam"))!=-1)).TupleOr(int((hv_ResultKeys.TupleFind("heatmap_guided_grad_cam"))!=-1))).TupleOr(int((hv_ResultKeys.TupleFind("heatmap_confidence_based"))!=-1))))
      {
        if (0 != (int(hv_SelectedHeatmapMethod==HTuple("heatmap_grad_cam"))))
        {
          GetDictTuple(hv_DLResult, "heatmap_grad_cam", &hv_DictHeatmap);
          hv_MethodName = "Grad-CAM";
        }
        else if (0 != (int(hv_SelectedHeatmapMethod==HTuple("heatmap_guided_grad_cam"))))
        {
          GetDictTuple(hv_DLResult, "heatmap_guided_grad_cam", &hv_DictHeatmap);
          hv_MethodName = "Guided-Grad-CAM";
        }
        else
        {
          GetDictTuple(hv_DLResult, "heatmap_confidence_based", &hv_DictHeatmap);
          hv_MethodName = "Confidence based";
        }
        GetDictParam(hv_DictHeatmap, "keys", HTuple(), &hv_HeatmapKeys);
        //
        if (0 != (int(hv_SelectedHeatmapMethod==HTuple("heatmap_guided_grad_cam"))))
        {
          hv_HeatmapImageName = hv_HeatmapKeys.TupleRegexpSelect("guided_grad_cam_image_class_[0-9]*");
          hv_TargetClassID = hv_HeatmapImageName.TupleRegexpMatch("guided_grad_cam_image_class_([0-9]+)$");
        }
        else
        {
          hv_HeatmapImageName = hv_HeatmapKeys.TupleRegexpSelect("heatmap_image_class_[0-9]*");
          hv_TargetClassID = hv_HeatmapImageName.TupleRegexpMatch("heatmap_image_class_([0-9]+)$");
        }
        GetDictObject(&ho_ImageHeatmap, hv_DictHeatmap, hv_HeatmapImageName);
      }
      else
      {
        throw HException("Heatmap image could not be found in DLResult.");
      }
      //
      //Only for the Grad-Cam heatmap the input image is shown.
      if (0 != (int(hv_SelectedHeatmapMethod==HTuple("heatmap_grad_cam"))))
      {
        //Read in input image.
        GetDictObject(&ho_Image, hv_DLSample, "image");
        //Add the heatmap to the input image.
        add_colormap_to_image(ho_ImageHeatmap, ho_Image, &ho_ImageHeatmap, hv_Params.TupleGetDictTuple("heatmap_color_scheme"));
      }
      //
      //Get or open next window.
      GetImageSize(ho_ImageHeatmap, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      if (HDevWindowStack::IsOpen())
        DispObj(ho_ImageHeatmap, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        hv_Text = "Classification heatmap "+hv_ImageIDStringBraces;
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        GetDictTuple(hv_DLResult, "classification_class_names", &hv_ClassNames);
        GetDictTuple(hv_DLResult, "classification_class_ids", &hv_ClassIDs);
        GetDictTuple(hv_DLResult, "classification_confidences", &hv_Confidences);
        if (0 != (int(hv_SelectedHeatmapMethod==HTuple("heatmap_confidence_based"))))
        {
          GetDictTuple(hv_DictHeatmap, "classification_heatmap_maxdeviation", &hv_MaxDeviation);
        }
        hv_ClassificationLabelNameResult = ((const HTuple&)hv_ClassNames)[0];
        hv_ClassificationLabelIDResult = ((const HTuple&)hv_ClassIDs)[0];
        hv_TargetClassConfidence = HTuple(hv_Confidences[hv_ClassIDs.TupleFind(hv_TargetClassID.TupleNumber())]);
        hv_Text = "--------- ";
        hv_Text[hv_Text.TupleLength()] = "Image ";
        hv_Text[hv_Text.TupleLength()] = "--------- ";
        hv_Text[hv_Text.TupleLength()] = "";
        if (0 != (int((hv_SampleKeys.TupleFind("image_label_id"))!=-1)))
        {
          GetDictTuple(hv_DLSample, "image_label_id", &hv_ClassificationLabelIDGroundTruth);
          GetDictTuple(hv_DLDatasetInfo, "class_names", &hv_ClassificationLabelNamesGroundTruth);
          //Get prediction color.
          if (0 != (int(hv_ClassificationLabelIDGroundTruth==hv_ClassificationLabelIDResult)))
          {
            hv_PredictionColor = "green";
          }
          else
          {
            hv_PredictionColor = "red";
          }
          hv_Text[hv_Text.TupleLength()] = "Ground truth class: ";
          hv_Text[hv_Text.TupleLength()] = HTuple(hv_ClassificationLabelNamesGroundTruth[hv_ClassificationLabelIDGroundTruth]);
          hv_Text[hv_Text.TupleLength()] = "";
        }
        hv_Text[hv_Text.TupleLength()] = "Predicted class: ";
        hv_Text[hv_Text.TupleLength()] = hv_ClassificationLabelNameResult;
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = "Confidence: "+(HTuple(hv_Confidences[0]).TupleString(".2f"));
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = "--------- ";
        hv_Text[hv_Text.TupleLength()] = "Heatmap ";
        hv_Text[hv_Text.TupleLength()] = "--------- ";
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = "Method: "+hv_MethodName;
        hv_Text[hv_Text.TupleLength()] = "Target class: "+hv_TargetClassID;
        hv_Text[hv_Text.TupleLength()] = "";
        hv_Text[hv_Text.TupleLength()] = "Target class confidence: "+(hv_TargetClassConfidence.TupleString(".2f"));
        if (0 != (int(hv_SelectedHeatmapMethod==HTuple("heatmap_confidence_based"))))
        {
          hv_Text[hv_Text.TupleLength()] = "Maximum deviation: "+(hv_MaxDeviation.TupleString(".2f"));
        }
        //
        //Get or open next child window
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        TupleGenConst(hv_Text.TupleLength(), "white", &hv_LineColors);
        if (0 != (int((hv_SampleKeys.TupleFind("image_label_id"))!=-1)))
        {
          hv_LineColors[8] = hv_PredictionColor;
        }
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              hv_LineColors, "box", "false");
      }
    }
    else if (0 != (HTuple(HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_recognition_ground_truth"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_recognition_result")))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_recognition_both")))))
    {
      //
      //OCR Recognition.
      hv_ShowGT = HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_recognition_both"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_recognition_ground_truth")));
      hv_ShowResult = HTuple(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_recognition_both"))).TupleOr(int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("ocr_recognition_result")));
      hv_NumLines = hv_ShowGT+hv_ShowResult;
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_NumLines*(hv_Params.TupleGetDictTuple("display_bottom_desc")), hv_WidthImage, 
          hv_HeightImage, 0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      FullDomain(ho_Image, &ho_Image);
      GetImageType(ho_Image, &hv_Type);
      if (0 != (int(hv_Type==HTuple("real"))))
      {
        //We assume that real images have been preprocessed
        //to the range -1,1 already. Hence, we need to rescale
        //them back to a visible range in byte.
        ScaleImage(ho_Image, &ho_Image, 255.0/2, 127);
        ConvertImageType(ho_Image, &ho_Image, "byte");
      }
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        hv_Text = HTuple();
        //Ground truth.
        if (0 != hv_ShowGT)
        {
          hv_Text[hv_Text.TupleLength()] = (("Ground truth: \""+(hv_DLSample.TupleGetDictTuple("word")))+"\" ")+hv_ImageIDStringBraces;
        }
        //Result.
        if (0 != hv_ShowResult)
        {
          GetDictParam(hv_DLSample, "key_exists", "word", &hv_GTWordKeyExists);
          if (0 != hv_GTWordKeyExists)
          {
            GetDictTuple(hv_WindowHandleDict, "meta_information", &hv_MetaInfo);
            GetDictTuple(hv_MetaInfo, HTuple(hv_KeysForDisplay[hv_KeyIndex])+"_window_coordinates", 
                &hv_WindowCoordinates);
            GetDictTuple(hv_MetaInfo, HTuple(hv_KeysForDisplay[hv_KeyIndex])+"_margin_bottom", 
                &hv_MarginBottom);
            hv_HeightWindow = ((const HTuple&)hv_WindowCoordinates)[3];
            hv_HeightMarginBottom = (hv_HeightImage*hv_MarginBottom)/(hv_HeightWindow-hv_MarginBottom);
            hv_Size = hv_HeightMarginBottom/2;
            hv_Length = hv_Size/2;
            hv_Row = hv_HeightImage+(hv_HeightMarginBottom/2);
            hv_Column = hv_WidthImage-(1.5*hv_Size);
            HomMat2dIdentity(&hv_HomMat2DIdentity);
            HomMat2dRotate(hv_HomMat2DIdentity, HTuple(45).TupleRad(), 0, 0, &hv_HomMat2DRotate);
            HomMat2dTranslate(hv_HomMat2DRotate, hv_Row, hv_Column, &hv_HomMat2DCompose);
            if (0 != ((hv_DLResult.TupleConcat(hv_DLSample)).TupleTestEqualDictItem("word","word")))
            {
              hv_PredictionForegroundColor = "green";
              hv_PredictionBackgroundColor = "#008000";
              GenContourPolygonXld(&ho_PredictionSymbol, (-0.75*hv_Length).TupleConcat((HTuple(0).Append(0))), 
                  ((-hv_Length).TupleConcat(-hv_Length)).TupleConcat(hv_Length));
            }
            else
            {
              hv_PredictionForegroundColor = "red";
              hv_PredictionBackgroundColor = "#800000";
              GenContourPolygonXld(&ho_CrossLineH, (HTuple(0).Append(0)), (-hv_Length).TupleConcat(hv_Length));
              GenContourPolygonXld(&ho_CrossLineV, (-hv_Length).TupleConcat(hv_Length), 
                  (HTuple(0).Append(0)));
              ConcatObj(ho_CrossLineH, ho_CrossLineV, &ho_PredictionSymbol);
            }
            AffineTransContourXld(ho_PredictionSymbol, &ho_PredictionSymbol, hv_HomMat2DCompose);
            SetWindowParam(hv_CurrentWindowHandle, "background_color", hv_PredictionBackgroundColor);
            if (HDevWindowStack::IsOpen())
              ClearWindow(HDevWindowStack::GetActive());
            if (HDevWindowStack::IsOpen())
              DispObj(ho_Image, HDevWindowStack::GetActive());
            if (HDevWindowStack::IsOpen())
              SetLineWidth(HDevWindowStack::GetActive(),5);
            if (HDevWindowStack::IsOpen())
              SetColor(HDevWindowStack::GetActive(),hv_PredictionForegroundColor);
            if (HDevWindowStack::IsOpen())
              DispObj(ho_PredictionSymbol, HDevWindowStack::GetActive());
          }
          else
          {
            hv_PredictionForegroundColor = "white";
          }
          if (0 != hv_ShowGT)
          {
            hv_Spaces = "    ";
          }
          else
          {
            hv_Spaces = "";
          }
          hv_Text[hv_Text.TupleLength()] = ((("Deep OCR:"+hv_Spaces)+" \"")+(hv_DLResult.TupleGetDictTuple("word")))+"\"";
        }
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("segmentation_confidence"))))
    {
      //
      //Segmentation confidences.
      get_confidence_image(&ho_ImageConfidence, hv_ResultKeys, hv_DLResult);
      //
      //Get or open next window.
      GetImageSize(ho_ImageConfidence, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      if (HDevWindowStack::IsOpen())
        DispObj(ho_ImageConfidence, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),"Confidence image "+hv_ImageIDStringBraces, 
              "window", "bottom", "left", "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("segmentation_confidence_map"))))
    {
      //
      //Segmentation confidence map on image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      get_confidence_image(&ho_ImageConfidence, hv_ResultKeys, hv_DLResult);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          hv_Params.TupleGetDictTuple("map_color_bar_width"), hv_Params.TupleGetDictTuple("scale_windows"), 
          hv_Params.TupleGetDictTuple("threshold_width"), hv_PrevWindowCoordinates, 
          hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), &hv_CurrentWindowHandle, 
          &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      dev_display_confidence_regions(ho_ImageConfidence, hv_Params.TupleGetDictTuple("map_transparency"), 
          &hv_ConfidenceColors);
      dev_display_map_color_bar(hv_WidthImage, hv_HeightImage, hv_Params.TupleGetDictTuple("map_color_bar_width"), 
          hv_ConfidenceColors, 1.0, hv_WindowImageRatio, hv_CurrentWindowHandle);
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),"Confidence map "+hv_ImageIDStringBraces, 
              "window", "bottom", "left", "white", "box", "false");
      }
      //
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("segmentation_image_both"))))
    {
      //
      //Ground truth and result segmentation on image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      get_segmentation_image_ground_truth(&ho_SegmentationImagGroundTruth, hv_SampleKeys, 
          hv_DLSample);
      get_segmentation_image_result(&ho_SegmentationImageResult, hv_ResultKeys, hv_DLResult);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      //Display regions.
      hv_ColorsResults = hv_Colors+(hv_Params.TupleGetDictTuple("segmentation_transparency"));
      if (HDevWindowStack::IsOpen())
        SetDraw(HDevWindowStack::GetActive(),"margin");
      if (HDevWindowStack::IsOpen())
        SetLineWidth(HDevWindowStack::GetActive(),2);
      dev_display_segmentation_regions(ho_SegmentationImagGroundTruth, hv_ClassIDs, 
          hv_ColorsResults, hv_Params.TupleGetDictTuple("segmentation_exclude_class_ids"), 
          &hv_GroundTruthIDs);
      if (HDevWindowStack::IsOpen())
        SetLineWidth(HDevWindowStack::GetActive(),6);
      dev_display_segmentation_regions(ho_SegmentationImageResult, hv_ClassIDs, hv_ColorsResults, 
          hv_Params.TupleGetDictTuple("segmentation_exclude_class_ids"), &hv_ResultIDs);
      if (HDevWindowStack::IsOpen())
        SetDraw(HDevWindowStack::GetActive(),"fill");
      hv_Text = "Ground truth and result segmentation "+hv_ImageIDStringBraces;
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_ImageClassIDsUniq = ((hv_GroundTruthIDs.TupleConcat(hv_ResultIDs)).TupleSort()).TupleUniq();
        //Get Indices according to image class IDs.
        TupleGenConst(hv_ImageClassIDsUniq.TupleLength(), 0, &hv_ImageClassIDsIndices);
        {
        HTuple end_val1504 = (hv_ImageClassIDsUniq.TupleLength())-1;
        HTuple step_val1504 = 1;
        for (hv_ImageClassIDsIndex=0; hv_ImageClassIDsIndex.Continue(end_val1504, step_val1504); hv_ImageClassIDsIndex += step_val1504)
        {
          hv_ImageClassIDsIndices[hv_ImageClassIDsIndex] = hv_ClassIDs.TupleFindFirst(HTuple(hv_ImageClassIDsUniq[hv_ImageClassIDsIndex]));
        }
        }
        hv_Text = hv_Text.TupleConcat(HTuple(hv_ClassesLegend[hv_ImageClassIDsIndices]));
        hv_Text[(hv_Text.TupleLength())+1] = HTuple("- thicker line: result, thinner lines: ground truth");
        hv_Text[hv_Text.TupleLength()] = "  (you may have to zoom in for a more detailed view)";
        hv_StringSegExcludeClassIDs = "";
        {
        HTuple end_val1511 = ((hv_Params.TupleGetDictTuple("segmentation_exclude_class_ids")).TupleLength())-1;
        HTuple step_val1511 = 1;
        for (hv_StringIndex=0; hv_StringIndex.Continue(end_val1511, step_val1511); hv_StringIndex += step_val1511)
        {
          if (0 != (int(hv_StringIndex==(((hv_Params.TupleGetDictTuple("segmentation_exclude_class_ids")).TupleLength())-1))))
          {
            hv_StringSegExcludeClassIDs += HTuple((hv_Params.TupleGetDictTuple("segmentation_exclude_class_ids"))[hv_StringIndex]);
          }
          else
          {
            hv_StringSegExcludeClassIDs = (hv_StringSegExcludeClassIDs+HTuple((hv_Params.TupleGetDictTuple("segmentation_exclude_class_ids"))[hv_StringIndex]))+HTuple(", ");
          }
        }
        }
        CreateDict(&hv___Tmp_Ctrl_Dict_Init_7);
        SetDictTuple(hv___Tmp_Ctrl_Dict_Init_7, "comp", HTuple());
        if (0 != (((hv_Params.TupleConcat(hv___Tmp_Ctrl_Dict_Init_7)).TupleTestEqualDictItem("segmentation_exclude_class_ids","comp")).TupleNot()))
        {
          hv_Text[hv_Text.TupleLength()] = ("- (excluded classID(s) "+hv_StringSegExcludeClassIDs)+" from visualization)";
        }
        hv___Tmp_Ctrl_Dict_Init_7 = HTuple::TupleConstant("HNULL");
        //
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              (HTuple("white").TupleConcat(HTuple(hv_Colors[hv_ImageClassIDsIndices]))).TupleConcat(((HTuple("white").Append("white")).Append("white"))), 
              "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("segmentation_image_diff"))))
    {
      //
      //Difference of ground truth and result segmentation on image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      get_segmentation_image_ground_truth(&ho_SegmentationImagGroundTruth, hv_SampleKeys, 
          hv_DLSample);
      get_segmentation_image_result(&ho_SegmentationImageResult, hv_ResultKeys, hv_DLResult);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      AbsDiffImage(ho_SegmentationImagGroundTruth, ho_SegmentationImageResult, &ho_ImageAbsDiff, 
          1);
      MinMaxGray(ho_SegmentationImageResult, ho_ImageAbsDiff, 0, &hv_Min, &hv_Max, 
          &hv_Range);
      if (0 != (int(hv_Min!=hv_Max)))
      {
        Threshold(ho_ImageAbsDiff, &ho_DiffRegion, 0.00001, hv_Max);
        if (HDevWindowStack::IsOpen())
          SetColor(HDevWindowStack::GetActive(),"#ff0000"+(hv_Params.TupleGetDictTuple("segmentation_transparency")));
        if (HDevWindowStack::IsOpen())
          DispObj(ho_DiffRegion, HDevWindowStack::GetActive());
      }
      else
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),"No difference found.", "window", 
              "top", "left", "black", HTuple(), HTuple());
      }
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        hv_Text = "Difference of ground truth and result segmentation "+hv_ImageIDStringBraces;
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("segmentation_image_ground_truth"))))
    {
      //
      //Ground truth segmentation image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      get_segmentation_image_ground_truth(&ho_SegmentationImagGroundTruth, hv_SampleKeys, 
          hv_DLSample);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      //Display segmentation regions.
      hv_ColorsSegmentation = hv_Colors+(hv_Params.TupleGetDictTuple("segmentation_transparency"));
      GetDraw(hv_CurrentWindowHandle, &hv_DrawMode);
      if (HDevWindowStack::IsOpen())
        SetDraw(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("segmentation_draw"));
      GetLineWidth(hv_CurrentWindowHandle, &hv_Width);
      if (HDevWindowStack::IsOpen())
        SetLineWidth(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("line_width"));
      dev_display_segmentation_regions(ho_SegmentationImagGroundTruth, hv_ClassIDs, 
          hv_ColorsSegmentation, hv_Params.TupleGetDictTuple("segmentation_exclude_class_ids"), 
          &hv_ImageClassIDs);
      if (HDevWindowStack::IsOpen())
        SetDraw(HDevWindowStack::GetActive(),hv_DrawMode);
      if (HDevWindowStack::IsOpen())
        SetLineWidth(HDevWindowStack::GetActive(),hv_Width.TupleInt());
      hv_Text = "Ground truth segmentation "+hv_ImageIDStringBraces;
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_ImageClassIDsUniq = (hv_ImageClassIDs.TupleSort()).TupleUniq();
        //Get Indices according to image class IDs.
        TupleGenConst(hv_ImageClassIDsUniq.TupleLength(), 0, &hv_ImageClassIDsIndices);
        {
        HTuple end_val1586 = (hv_ImageClassIDsUniq.TupleLength())-1;
        HTuple step_val1586 = 1;
        for (hv_ImageClassIDsIndex=0; hv_ImageClassIDsIndex.Continue(end_val1586, step_val1586); hv_ImageClassIDsIndex += step_val1586)
        {
          hv_ImageClassIDsIndices[hv_ImageClassIDsIndex] = hv_ClassIDs.TupleFindFirst(HTuple(hv_ImageClassIDsUniq[hv_ImageClassIDsIndex]));
        }
        }
        hv_Text = hv_Text.TupleConcat(HTuple(hv_ClassesLegend[hv_ImageClassIDsIndices]));
        //
        //Get or open next child window
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              HTuple("white").TupleConcat(HTuple(hv_Colors[hv_ImageClassIDsIndices])), 
              "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("segmentation_image_result"))))
    {
      //
      //Result segmentation on image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      get_segmentation_image_result(&ho_SegmentationImageResult, hv_ResultKeys, hv_DLResult);
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      //
      //Display result segmentation regions.
      hv_ColorsResults = hv_Colors+(hv_Params.TupleGetDictTuple("segmentation_transparency"));
      GetDraw(hv_CurrentWindowHandle, &hv_DrawMode);
      if (HDevWindowStack::IsOpen())
        SetDraw(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("segmentation_draw"));
      GetLineWidth(hv_CurrentWindowHandle, &hv_Width);
      if (HDevWindowStack::IsOpen())
        SetLineWidth(HDevWindowStack::GetActive(),hv_Params.TupleGetDictTuple("line_width"));
      dev_display_segmentation_regions(ho_SegmentationImageResult, hv_ClassIDs, hv_ColorsResults, 
          hv_Params.TupleGetDictTuple("segmentation_exclude_class_ids"), &hv_ImageClassIDs);
      if (HDevWindowStack::IsOpen())
        SetDraw(HDevWindowStack::GetActive(),hv_DrawMode);
      if (HDevWindowStack::IsOpen())
        SetLineWidth(HDevWindowStack::GetActive(),hv_Width.TupleInt());
      hv_Text = "Result segmentation "+hv_ImageIDStringBraces;
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "bottom", "left", 
              "white", "box", "false");
      }
      //
      //Display the legend.
      if (0 != (hv_Params.TupleGetDictTuple("display_legend")))
      {
        hv_ImageClassIDsUniq = (hv_ImageClassIDs.TupleSort()).TupleUniq();
        //Get Indices according to image class IDs.
        TupleGenConst(hv_ImageClassIDsUniq.TupleLength(), 0, &hv_ImageClassIDsIndices);
        {
        HTuple end_val1627 = (hv_ImageClassIDsUniq.TupleLength())-1;
        HTuple step_val1627 = 1;
        for (hv_ImageClassIDsIndex=0; hv_ImageClassIDsIndex.Continue(end_val1627, step_val1627); hv_ImageClassIDsIndex += step_val1627)
        {
          hv_ImageClassIDsIndices[hv_ImageClassIDsIndex] = hv_ClassIDs.TupleFindFirst(HTuple(hv_ImageClassIDsUniq[hv_ImageClassIDsIndex]));
        }
        }
        hv_Text = hv_Text.TupleConcat(HTuple(hv_ClassesLegend[hv_ImageClassIDsIndices]));
        //
        //Get or open next child window.
        get_child_window(hv_HeightImage, hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
            hv_Text, hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
            &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),hv_Text, "window", "top", "left", 
              HTuple("white").TupleConcat(HTuple(hv_Colors[hv_ImageClassIDsIndices])), 
              "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("segmentation_weight"))))
    {
      //
      //Weight image.
      get_weight_image(&ho_ImageWeight, hv_SampleKeys, hv_DLSample);
      //
      //Get or open next window.
      GetImageSize(ho_ImageWeight, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          0, hv_Params.TupleGetDictTuple("scale_windows"), hv_Params.TupleGetDictTuple("threshold_width"), 
          hv_PrevWindowCoordinates, hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), 
          &hv_CurrentWindowHandle, &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      if (HDevWindowStack::IsOpen())
        DispObj(ho_ImageWeight, HDevWindowStack::GetActive());
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),"Weight image "+hv_ImageIDStringBraces, 
              "window", "bottom", "left", "white", "box", "false");
      }
    }
    else if (0 != (int(HTuple(hv_KeysForDisplay[hv_KeyIndex])==HTuple("segmentation_weight_map"))))
    {
      //
      //Weight map on image.
      get_dl_sample_image(&ho_Image, hv_SampleKeys, hv_DLSample, "image");
      get_weight_image(&ho_ImageWeight, hv_SampleKeys, hv_DLSample);
      //
      CreateDict(&hv___Tmp_Ctrl_Dict_Init_8);
      SetDictTuple(hv___Tmp_Ctrl_Dict_Init_8, "comp", 0);
      if (0 != ((hv_Params.TupleConcat(hv___Tmp_Ctrl_Dict_Init_8)).TupleTestEqualDictItem("segmentation_max_weight","comp")))
      {
        //Calculate SegMaxWeight if not given in GenParam.
        MinMaxGray(ho_ImageWeight, ho_ImageWeight, 0, &hv_MinWeight, &hv___Tmp_Ctrl_1, 
            &hv_Range);
        SetDictTuple(hv_Params, "segmentation_max_weight", hv___Tmp_Ctrl_1);
      }
      hv___Tmp_Ctrl_Dict_Init_8 = HTuple::TupleConstant("HNULL");
      //
      //Get or open next window.
      GetImageSize(ho_Image, &hv_WidthImage, &hv_HeightImage);
      get_next_window(hv_Params.TupleGetDictTuple("font"), hv_Params.TupleGetDictTuple("font_size"), 
          hv_Params.TupleGetDictTuple("display_bottom_desc"), hv_WidthImage, hv_HeightImage, 
          hv_Params.TupleGetDictTuple("map_color_bar_width"), hv_Params.TupleGetDictTuple("scale_windows"), 
          hv_Params.TupleGetDictTuple("threshold_width"), hv_PrevWindowCoordinates, 
          hv_WindowHandleDict, HTuple(hv_KeysForDisplay[hv_KeyIndex]), &hv_CurrentWindowHandle, 
          &hv_WindowImageRatio, &hv_PrevWindowCoordinates);
      //
      //Visualization.
      if (HDevWindowStack::IsOpen())
        DispObj(ho_Image, HDevWindowStack::GetActive());
      dev_display_weight_regions(ho_ImageWeight, hv_Params.TupleGetDictTuple("map_transparency"), 
          hv_Params.TupleGetDictTuple("segmentation_max_weight"), &hv_WeightsColors);
      dev_display_map_color_bar(hv_WidthImage, hv_HeightImage, hv_Params.TupleGetDictTuple("map_color_bar_width"), 
          hv_WeightsColors, hv_Params.TupleGetDictTuple("segmentation_max_weight"), 
          hv_WindowImageRatio, hv_CurrentWindowHandle);
      if (0 != (hv_Params.TupleGetDictTuple("display_bottom_desc")))
      {
        if (HDevWindowStack::IsOpen())
          DispText(HDevWindowStack::GetActive(),"Weight map "+hv_ImageIDStringBraces, 
              "window", "bottom", "left", "white", "box", "false");
      }
    }
    else
    {
      //Reset flush buffer of existing windows before throwing an exception.
      GetDictParam(hv_WindowHandleDict, "keys", HTuple(), &hv_WindowHandleKeys);
      {
      HTuple end_val1678 = (hv_WindowHandleKeys.TupleLength())-1;
      HTuple step_val1678 = 1;
      for (hv_Index=0; hv_Index.Continue(end_val1678, step_val1678); hv_Index += step_val1678)
      {
        //Only consider the WindowHandleKeys that are needed for the current visualization.
        hv_Indices = hv_KeysForDisplay.TupleFind(HTuple(hv_WindowHandleKeys[hv_Index]));
        if (0 != (HTuple(int(hv_Indices!=-1)).TupleAnd(int(hv_Indices!=HTuple()))))
        {
          GetDictTuple(hv_WindowHandleDict, HTuple(hv_WindowHandleKeys[hv_Index]), 
              &hv_WindowHandles);
          {
          HTuple end_val1683 = (hv_WindowHandles.TupleLength())-1;
          HTuple step_val1683 = 1;
          for (hv_WindowIndex=0; hv_WindowIndex.Continue(end_val1683, step_val1683); hv_WindowIndex += step_val1683)
          {
            //Reset values of windows that have been changed temporarily.
            SetWindowParam(HTuple(hv_WindowHandles[hv_WindowIndex]), "flush", HTuple(hv_FlushValues[hv_Index]));
          }
          }
        }
      }
      }
      throw HException("Key for display unknown: "+HTuple(hv_KeysForDisplay[hv_KeyIndex]));
    }
    //
    hv_KeyIndex += 1;
  }
  //
  //Display results.
  GetDictParam(hv_WindowHandleDict, "keys", HTuple(), &hv_WindowHandleKeysNew);
  {
  HTuple end_val1697 = (hv_WindowHandleKeysNew.TupleLength())-1;
  HTuple step_val1697 = 1;
  for (hv_Index=0; hv_Index.Continue(end_val1697, step_val1697); hv_Index += step_val1697)
  {
    //Only consider the WindowHandleKeys that are needed for the current visualization.
    hv_KeyIndex = hv_KeysForDisplay.TupleFind(HTuple(hv_WindowHandleKeysNew[hv_Index]));
    if (0 != (HTuple(int(hv_KeyIndex!=-1)).TupleAnd(int(hv_KeyIndex!=HTuple()))))
    {
      GetDictTuple(hv_WindowHandleDict, HTuple(hv_WindowHandleKeysNew[hv_Index]), 
          &hv_WindowHandles);
      {
      HTuple end_val1702 = (hv_WindowHandles.TupleLength())-1;
      HTuple step_val1702 = 1;
      for (hv_WindowIndex=0; hv_WindowIndex.Continue(end_val1702, step_val1702); hv_WindowIndex += step_val1702)
      {
        //Display content of window handle.
        if (0 != (int((hv_WindowHandleKeys.TupleLength())==(hv_WindowHandleKeysNew.TupleLength()))))
        {
          //Reset values of windows that have been changed temporarily.
          if (0 != (int(HTuple(hv_FlushValues[hv_WindowIndex])==HTuple("true"))))
          {
            FlushBuffer(HTuple(hv_WindowHandles[hv_WindowIndex]));
          }
          SetWindowParam(HTuple(hv_WindowHandles[hv_WindowIndex]), "flush", HTuple(hv_FlushValues[hv_WindowIndex]));
        }
        else
        {
          //Per default, 'flush' of new windows should be set to 'true'.
          FlushBuffer(HTuple(hv_WindowHandles[hv_WindowIndex]));
          SetWindowParam(HTuple(hv_WindowHandles[hv_WindowIndex]), "flush", "true");
        }
      }
      }
    }
  }
  }
  //
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Try to guess the maximum class id based on the given sample/result. 
void dev_display_dl_data_get_max_class_id (HTuple hv_DLSample, HTuple *hv_MaxClassId, 
    HTuple *hv_Empty)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_Keys, hv_Matches, hv_Length, hv_Greatereq;
  HTuple  hv_Indices, hv_LengthAll, hv_KeyIndex, hv_Key, hv_KeyType;
  HTuple  hv_Tuple, hv_MaxTuple, hv_Exception;

  (*hv_MaxClassId) = -1;
  (*hv_Empty) = 0;
  try
  {
    GetDictParam(hv_DLSample, "keys", HTuple(), &hv_Keys);
    //Find keys that end on '_id'. (They contain ids!)
    TupleRegexpMatch(hv_Keys, "_id", &hv_Matches);
    TupleStrlen(hv_Matches, &hv_Length);
    TupleGreaterEqualElem(hv_Length, 1, &hv_Greatereq);
    TupleFind(hv_Greatereq, 1, &hv_Indices);
    if (0 != (int(hv_Indices>-1)))
    {
      //Find the maximum given class id.
      hv_LengthAll = 0;
      {
      HTuple end_val12 = (hv_Indices.TupleLength())-1;
      HTuple step_val12 = 1;
      for (hv_KeyIndex=0; hv_KeyIndex.Continue(end_val12, step_val12); hv_KeyIndex += step_val12)
      {
        hv_Key = HTuple(hv_Keys[HTuple(hv_Indices[hv_KeyIndex])]);
        //Skip image_id.
        if (0 != (int(hv_Key==HTuple("image_id"))))
        {
          continue;
        }
        GetDictParam(hv_DLSample, "key_data_type", hv_Key, &hv_KeyType);
        if (0 != (int(hv_KeyType!=HTuple("tuple"))))
        {
          continue;
        }
        GetDictTuple(hv_DLSample, hv_Key, &hv_Tuple);
        hv_LengthAll += hv_Tuple.TupleLength();
        if (0 != (int((hv_Tuple.TupleLength())==0)))
        {
          continue;
        }
        TupleMax(hv_Tuple, &hv_MaxTuple);
        if (0 != (int(hv_MaxTuple>(*hv_MaxClassId))))
        {
          (*hv_MaxClassId) = hv_MaxTuple;
        }
      }
      }
      if (0 != (int(hv_LengthAll==0)))
      {
        (*hv_Empty) = 1;
      }
    }
  }
  // catch (Exception) 
  catch (HException &HDevExpDefaultException)
  {
    HDevExpDefaultException.ToHTuple(&hv_Exception);
    //Ignore any exception.
  }
  return;
}

// Chapter: Deep Learning / Anomaly Detection and Global Context Anomaly Detection
// Short Description: Display the ground truth anomaly regions of the given DLSample. 
void dev_display_ground_truth_anomaly_regions (HTuple hv_SampleKeys, HTuple hv_DLSample, 
    HTuple hv_CurrentWindowHandle, HTuple hv_LineWidth, HTuple hv_AnomalyRegionLabelColor, 
    HTuple hv_AnomalyColorTransparency, HTuple *hv_AnomalyRegionExists)
{

  // Local iconic variables
  HObject  ho_AnomalyImage, ho_AnomalyRegion;

  // Local control variables
  HTuple  hv_Red, hv_Green, hv_Blue, hv_Alpha, hv_InitialColor;
  HTuple  hv_IndexColor, hv_Color_RGBA, hv_Area;

  //
  //This procedure visualizes the ground truth anomalies
  //if there is an anomaly_ground_truth in DLSample.
  //
  //Get current set color.
  GetRgba(hv_CurrentWindowHandle, &hv_Red, &hv_Green, &hv_Blue, &hv_Alpha);
  hv_InitialColor = HTuple();
  {
  HTuple end_val7 = (hv_Red.TupleLength())-1;
  HTuple step_val7 = 1;
  for (hv_IndexColor=0; hv_IndexColor.Continue(end_val7, step_val7); hv_IndexColor += step_val7)
  {
    hv_Color_RGBA = ((("#"+(HTuple(hv_Red[hv_IndexColor]).TupleString("2x")))+(HTuple(hv_Green[hv_IndexColor]).TupleString("2x")))+(HTuple(hv_Blue[hv_IndexColor]).TupleString("2x")))+(HTuple(hv_Alpha[hv_IndexColor]).TupleString("2x"));
    TupleRegexpReplace(hv_Color_RGBA, (HTuple(" ").Append("replace_all")), "0", &hv_Color_RGBA);
    hv_InitialColor = hv_InitialColor.TupleConcat(hv_Color_RGBA);
  }
  }
  //
  if (0 != (int((hv_SampleKeys.TupleFind("anomaly_ground_truth"))!=-1)))
  {
    GetDictObject(&ho_AnomalyImage, hv_DLSample, "anomaly_ground_truth");
    Threshold(ho_AnomalyImage, &ho_AnomalyRegion, 1, 255);
    //Get non-empty regions.
    RegionFeatures(ho_AnomalyRegion, "area", &hv_Area);
    if (0 != (int(hv_Area>0)))
    {
      if (HDevWindowStack::IsOpen())
        SetColor(HDevWindowStack::GetActive(),hv_AnomalyRegionLabelColor+hv_AnomalyColorTransparency);
      //Display the anomaly region.
      if (HDevWindowStack::IsOpen())
        SetDraw(HDevWindowStack::GetActive(),"fill");
      if (HDevWindowStack::IsOpen())
        DispObj(ho_AnomalyRegion, HDevWindowStack::GetActive());
    }
    (*hv_AnomalyRegionExists) = "true";
  }
  else
  {
    (*hv_AnomalyRegionExists) = "false";
  }
  //
  //Reset colors.
  if (HDevWindowStack::IsOpen())
    SetColor(HDevWindowStack::GetActive(),hv_InitialColor);
  //
  return;
}

// Chapter: Graphics / Output
// Short Description: Display the ground truth bounding boxes of DLSample. 
void dev_display_ground_truth_detection (HTuple hv_DLSample, HTuple hv_SampleKeys, 
    HTuple hv_LineWidthBbox, HTuple hv_ClassIDs, HTuple hv_BboxColors, HTuple hv_BboxLabelColor, 
    HTuple hv_WindowImageRatio, HTuple hv_TextColor, HTuple hv_ShowLabels, HTuple hv_ShowDirection, 
    HTuple hv_WindowHandle, HTuple *hv_BboxIDs)
{

  // Local iconic variables
  HObject  ho_InstanceMask, ho_BboxRectangle, ho_OrientationArrows;
  HObject  ho_RectangleSelected, ho_MaskSelected, ho_ArrowSelected;

  // Local control variables
  HTuple  hv_InstanceType, hv_MaskExists, hv_BboxRow1;
  HTuple  hv_BboxCol1, hv_BboxRow2, hv_BboxCol2, hv_BboxRow;
  HTuple  hv_BboxCol, hv_BboxLength1, hv_BboxLength2, hv_BboxPhi;
  HTuple  hv_BboxLabels, hv_Text, hv_Ascent, hv_Descent, hv__;
  HTuple  hv_TextOffset, hv_LabelRow, hv_LabelCol, hv_ArrowSizeFactorLength;
  HTuple  hv_ArrowSizeFactorHead, hv_MaxLengthArrow, hv_HalfLengthArrow;
  HTuple  hv_ArrowBaseRow, hv_ArrowBaseCol, hv_ArrowHeadRow;
  HTuple  hv_ArrowHeadCol, hv_ArrowHeadSize, hv_ContourStyle;
  HTuple  hv_Style, hv_IndexBbox, hv_ClassID, hv_TextColorClasses;

  //
  //This procedure displays the ground truth bounding boxes and masks (if present) of a DLSample.
  //
  hv_InstanceType = "";
  hv_MaskExists = 0;
  if (0 != (int((hv_SampleKeys.TupleFind("bbox_row1"))!=-1)))
  {
    GetDictTuple(hv_DLSample, "bbox_row1", &hv_BboxRow1);
    GetDictTuple(hv_DLSample, "bbox_col1", &hv_BboxCol1);
    GetDictTuple(hv_DLSample, "bbox_row2", &hv_BboxRow2);
    GetDictTuple(hv_DLSample, "bbox_col2", &hv_BboxCol2);
    hv_InstanceType = "rectangle1";
  }
  else if (0 != (int((hv_SampleKeys.TupleFind("bbox_phi"))!=-1)))
  {
    GetDictTuple(hv_DLSample, "bbox_row", &hv_BboxRow);
    GetDictTuple(hv_DLSample, "bbox_col", &hv_BboxCol);
    GetDictTuple(hv_DLSample, "bbox_length1", &hv_BboxLength1);
    GetDictTuple(hv_DLSample, "bbox_length2", &hv_BboxLength2);
    GetDictTuple(hv_DLSample, "bbox_phi", &hv_BboxPhi);
    hv_InstanceType = "rectangle2";
  }
  else
  {
    throw HException("Ground truth bounding box data could not be found in DLSample.");
  }
  if (0 != (int((hv_SampleKeys.TupleFind("mask"))!=-1)))
  {
    GetDictObject(&ho_InstanceMask, hv_DLSample, "mask");
    hv_MaskExists = 1;
  }
  if (0 != (HTuple(HTuple(int(hv_InstanceType!=HTuple("rectangle1"))).TupleAnd(int(hv_InstanceType!=HTuple("rectangle2")))).TupleAnd(hv_MaskExists.TupleNot())))
  {
    throw HException("Ground truth bounding box or mask data could not be found in DLSample.");
  }
  GetDictTuple(hv_DLSample, "bbox_label_id", &hv_BboxLabels);
  if (0 != (int((hv_BboxLabels.TupleLength())>0)))
  {
    //
    //Get text and text size for correct positioning of label IDs.
    if (0 != hv_ShowLabels)
    {
      hv_Text = hv_BboxLabels;
      GetStringExtents(hv_WindowHandle, hv_Text, &hv_Ascent, &hv_Descent, &hv__, 
          &hv__);
      hv_TextOffset = (hv_Ascent+hv_Descent)/hv_WindowImageRatio;
    }
    //
    //Generate bounding box XLDs.
    if (0 != (int(hv_InstanceType==HTuple("rectangle1"))))
    {
      TupleGenConst(hv_BboxRow1.TupleLength(), 0.0, &hv_BboxPhi);
      GenRectangle2ContourXld(&ho_BboxRectangle, 0.5*(hv_BboxRow1+hv_BboxRow2), 0.5*(hv_BboxCol1+hv_BboxCol2), 
          hv_BboxPhi, 0.5*(hv_BboxCol2-hv_BboxCol1), 0.5*(hv_BboxRow2-hv_BboxRow1));
      if (0 != hv_ShowLabels)
      {
        hv_LabelRow = hv_BboxRow1;
        hv_LabelCol = hv_BboxCol1;
      }
    }
    else if (0 != (int(hv_InstanceType==HTuple("rectangle2"))))
    {
      GenRectangle2ContourXld(&ho_BboxRectangle, hv_BboxRow, hv_BboxCol, hv_BboxPhi, 
          hv_BboxLength1, hv_BboxLength2);
      if (0 != hv_ShowLabels)
      {
        hv_LabelRow = hv_BboxRow-hv_TextOffset;
        hv_LabelCol = hv_BboxCol;
      }
      if (0 != hv_ShowDirection)
      {
        if (0 != (int(hv_ShowDirection==-1)))
        {
          hv_ArrowSizeFactorLength = 0.4;
          hv_ArrowSizeFactorHead = 0.2;
          hv_MaxLengthArrow = 20;
          hv_HalfLengthArrow = hv_MaxLengthArrow.TupleMin2(hv_BboxLength1*hv_ArrowSizeFactorLength);
          hv_ArrowBaseRow = hv_BboxRow-((hv_BboxLength1-hv_HalfLengthArrow)*(hv_BboxPhi.TupleSin()));
          hv_ArrowBaseCol = hv_BboxCol+((hv_BboxLength1-hv_HalfLengthArrow)*(hv_BboxPhi.TupleCos()));
          hv_ArrowHeadRow = hv_BboxRow-((hv_BboxLength1+hv_HalfLengthArrow)*(hv_BboxPhi.TupleSin()));
          hv_ArrowHeadCol = hv_BboxCol+((hv_BboxLength1+hv_HalfLengthArrow)*(hv_BboxPhi.TupleCos()));
          hv_ArrowHeadSize = (hv_MaxLengthArrow.TupleMin2(hv_BboxLength1.TupleMin2(hv_BboxLength2)))*hv_ArrowSizeFactorHead;
        }
        else
        {
          hv_ArrowHeadSize = 20.0;
          hv_ArrowBaseRow = hv_BboxRow;
          hv_ArrowBaseCol = hv_BboxCol;
          hv_ArrowHeadRow = hv_BboxRow-((hv_BboxLength1+hv_ArrowHeadSize)*(hv_BboxPhi.TupleSin()));
          hv_ArrowHeadCol = hv_BboxCol+((hv_BboxLength1+hv_ArrowHeadSize)*(hv_BboxPhi.TupleCos()));
        }
        gen_arrow_contour_xld(&ho_OrientationArrows, hv_ArrowBaseRow, hv_ArrowBaseCol, 
            hv_ArrowHeadRow, hv_ArrowHeadCol, hv_ArrowHeadSize, hv_ArrowHeadSize);
      }
    }
    else if (0 != hv_MaskExists)
    {
      if (0 != hv_ShowLabels)
      {
        AreaCenter(ho_InstanceMask, &hv__, &hv_LabelRow, &hv_LabelCol);
      }
    }
    else
    {
      throw HException("Unknown instance_type: "+hv_InstanceType);
    }
    //
    //Collect the ClassIDs of the bounding boxes.
    TupleGenConst(hv_BboxLabels.TupleLength(), 0, &(*hv_BboxIDs));
    //
    //Draw the bounding boxes.
    GetContourStyle(hv_WindowHandle, &hv_ContourStyle);
    if (HDevWindowStack::IsOpen())
      SetContourStyle(HDevWindowStack::GetActive(),"stroke_and_fill");
    GetLineStyle(hv_WindowHandle, &hv_Style);
    if (HDevWindowStack::IsOpen())
      SetLineWidth(HDevWindowStack::GetActive(),hv_LineWidthBbox);
    {
    HTuple end_val88 = (hv_BboxLabels.TupleLength())-1;
    HTuple step_val88 = 1;
    for (hv_IndexBbox=0; hv_IndexBbox.Continue(end_val88, step_val88); hv_IndexBbox += step_val88)
    {
      SelectObj(ho_BboxRectangle, &ho_RectangleSelected, hv_IndexBbox+1);
      hv_ClassID = hv_ClassIDs.TupleFind(HTuple(hv_BboxLabels[hv_IndexBbox]));
      (*hv_BboxIDs)[hv_IndexBbox] = hv_ClassID;
      if (HDevWindowStack::IsOpen())
        SetColor(HDevWindowStack::GetActive(),HTuple(hv_BboxColors[hv_ClassID])+"60");
      if (0 != hv_MaskExists)
      {
        if (HDevWindowStack::IsOpen())
          SetDraw(HDevWindowStack::GetActive(),"fill");
        SelectObj(ho_InstanceMask, &ho_MaskSelected, hv_IndexBbox+1);
        if (HDevWindowStack::IsOpen())
          DispObj(ho_MaskSelected, HDevWindowStack::GetActive());
        if (HDevWindowStack::IsOpen())
          SetContourStyle(HDevWindowStack::GetActive(),"stroke");
      }
      if (0 != (int(hv_InstanceType!=HTuple(""))))
      {
        SelectObj(ho_BboxRectangle, &ho_RectangleSelected, hv_IndexBbox+1);
        if (HDevWindowStack::IsOpen())
          DispObj(ho_RectangleSelected, HDevWindowStack::GetActive());
        if (0 != (HTuple(int(hv_InstanceType==HTuple("rectangle2"))).TupleAnd(hv_ShowDirection)))
        {
          SelectObj(ho_OrientationArrows, &ho_ArrowSelected, hv_IndexBbox+1);
          if (HDevWindowStack::IsOpen())
            SetColor(HDevWindowStack::GetActive(),HTuple(hv_BboxColors[hv_ClassID])+"FF");
          if (HDevWindowStack::IsOpen())
            DispObj(ho_ArrowSelected, HDevWindowStack::GetActive());
          if (HDevWindowStack::IsOpen())
            SetColor(HDevWindowStack::GetActive(),HTuple(hv_BboxColors[hv_ClassID])+"60");
        }
      }
    }
    }
    //
    //Write text to the bounding boxes.
    if (0 != hv_ShowLabels)
    {
      //For better visibility the text is displayed after all bounding boxes are drawn.
      //Select text color.
      if (0 != (int(hv_TextColor==HTuple(""))))
      {
        hv_TextColorClasses = HTuple(hv_BboxColors[(*hv_BboxIDs)]);
      }
      else
      {
        TupleGenConst((*hv_BboxIDs).TupleLength(), hv_TextColor, &hv_TextColorClasses);
      }
      //Display text.
      if (HDevWindowStack::IsOpen())
        DispText(HDevWindowStack::GetActive(),hv_BboxLabels, "image", hv_LabelRow, 
            hv_LabelCol, hv_TextColorClasses, ((HTuple("box_color").Append("shadow")).Append("border_radius")), 
            hv_BboxLabelColor.TupleConcat((HTuple("false").Append(0))));
    }
    //
    if (HDevWindowStack::IsOpen())
      SetContourStyle(HDevWindowStack::GetActive(),hv_ContourStyle);
    SetLineStyle(hv_WindowHandle, hv_Style);
  }
  else
  {
    //Do nothing if there are no ground truth bounding boxes.
    (*hv_BboxIDs) = HTuple();
  }
  //
  return;
}

// Chapter: Graphics / Output
// Short Description: Display a color bar next to an image. 
void dev_display_map_color_bar (HTuple hv_ImageWidth, HTuple hv_ImageHeight, HTuple hv_MapColorBarWidth, 
    HTuple hv_Colors, HTuple hv_MaxValue, HTuple hv_WindowImageRatio, HTuple hv_WindowHandle)
{

  // Local iconic variables
  HObject  ho_Rectangle;

  // Local control variables
  HTuple  hv_ClipRegion, hv_ColorIndex, hv_RectHeight;
  HTuple  hv_DrawMode, hv_Row, hv_Row1, hv_Column1, hv_Row2;
  HTuple  hv_Column2, hv__, hv_TextHeight, hv_Index, hv_Text;

  //
  //This procedure displays a color bar next to the image
  //specified with ImageWidth and ImageHeight.
  //
  GetSystem("clip_region", &hv_ClipRegion);
  SetSystem("clip_region", "false");
  //
  //Display the color bar.
  hv_ColorIndex = 0;
  hv_RectHeight = (1.0*hv_ImageHeight)/(hv_Colors.TupleLength());
  //Set draw mode to fill
  GetDraw(hv_WindowHandle, &hv_DrawMode);
  if (HDevWindowStack::IsOpen())
    SetDraw(HDevWindowStack::GetActive(),"fill");
  {
  HTuple end_val13 = 0;
  HTuple step_val13 = -hv_RectHeight;
  for (hv_Row=hv_ImageHeight-1; hv_Row.Continue(end_val13, step_val13); hv_Row += step_val13)
  {
    //The color bar consists of multiple rectangle1.
    hv_Row1 = hv_Row-hv_RectHeight;
    hv_Column1 = hv_ImageWidth+(20/hv_WindowImageRatio);
    hv_Row2 = hv_Row;
    hv_Column2 = (hv_ImageWidth+20)+(hv_MapColorBarWidth/hv_WindowImageRatio);
    GenRectangle1(&ho_Rectangle, hv_Row1, hv_Column1, hv_Row2, hv_Column2);
    if (HDevWindowStack::IsOpen())
      SetColor(HDevWindowStack::GetActive(),HTuple(hv_Colors[hv_ColorIndex]));
    if (HDevWindowStack::IsOpen())
      DispObj(ho_Rectangle, HDevWindowStack::GetActive());
    hv_ColorIndex += 1;
  }
  }
  //
  //Display labels for color bar.
  GetStringExtents(hv_WindowHandle, "0123456789", &hv__, &hv__, &hv__, &hv_TextHeight);
  for (hv_Index=0; hv_Index<=1; hv_Index+=0.2)
  {
    hv_Text = (hv_MaxValue-(hv_Index*hv_MaxValue)).TupleString(".1f");
    if (HDevWindowStack::IsOpen())
      DispText(HDevWindowStack::GetActive(),hv_Text, "image", hv_Index*(hv_ImageHeight-(2*(hv_TextHeight/hv_WindowImageRatio))), 
          hv_ImageWidth+(40/hv_WindowImageRatio), "black", "box", "false");
  }
  //
  SetSystem("clip_region", hv_ClipRegion);
  if (HDevWindowStack::IsOpen())
    SetDraw(HDevWindowStack::GetActive(),hv_DrawMode);
  return;
}

// Chapter: Deep Learning / Anomaly Detection and Global Context Anomaly Detection
// Short Description: Display the detected anomaly regions. 
void dev_display_result_anomaly_regions (HObject ho_AnomalyRegion, HTuple hv_CurrentWindowHandle, 
    HTuple hv_LineWidth, HTuple hv_AnomalyRegionResultColor)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_Red, hv_Green, hv_Blue, hv_Alpha, hv_InitialColor;
  HTuple  hv_IndexColor, hv_Color_RGBA, hv_Area;

  //
  //This procedure displays the result anomaly regions.
  //
  //Get current set color.
  GetRgba(hv_CurrentWindowHandle, &hv_Red, &hv_Green, &hv_Blue, &hv_Alpha);
  hv_InitialColor = HTuple();
  {
  HTuple end_val6 = (hv_Red.TupleLength())-1;
  HTuple step_val6 = 1;
  for (hv_IndexColor=0; hv_IndexColor.Continue(end_val6, step_val6); hv_IndexColor += step_val6)
  {
    hv_Color_RGBA = ((("#"+(HTuple(hv_Red[hv_IndexColor]).TupleString("2x")))+(HTuple(hv_Green[hv_IndexColor]).TupleString("2x")))+(HTuple(hv_Blue[hv_IndexColor]).TupleString("2x")))+(HTuple(hv_Alpha[hv_IndexColor]).TupleString("2x"));
    TupleRegexpReplace(hv_Color_RGBA, (HTuple(" ").Append("replace_all")), "0", &hv_Color_RGBA);
    hv_InitialColor = hv_InitialColor.TupleConcat(hv_Color_RGBA);
  }
  }
  //
  //Display anomaly regions.
  //Get non-empty regions.
  RegionFeatures(ho_AnomalyRegion, "area", &hv_Area);
  //
  //Display all non-empty class regions in distinct colors.
  if (0 != (int(hv_Area>0)))
  {
    if (HDevWindowStack::IsOpen())
      SetColor(HDevWindowStack::GetActive(),hv_AnomalyRegionResultColor);
    if (HDevWindowStack::IsOpen())
      SetLineWidth(HDevWindowStack::GetActive(),hv_LineWidth);
    if (HDevWindowStack::IsOpen())
      SetDraw(HDevWindowStack::GetActive(),"margin");
    if (HDevWindowStack::IsOpen())
      DispObj(ho_AnomalyRegion, HDevWindowStack::GetActive());
  }
  //
  //Reset colors.
  if (HDevWindowStack::IsOpen())
    SetColor(HDevWindowStack::GetActive(),hv_InitialColor);
  //
  return;
}

// Chapter: Graphics / Output
// Short Description: Display result bounding boxes. 
void dev_display_result_detection (HTuple hv_DLResult, HTuple hv_ResultKeys, HTuple hv_LineWidthBbox, 
    HTuple hv_ClassIDs, HTuple hv_TextConf, HTuple hv_Colors, HTuple hv_BoxLabelColor, 
    HTuple hv_WindowImageRatio, HTuple hv_TextPositionRow, HTuple hv_TextColor, HTuple hv_ShowLabels, 
    HTuple hv_ShowDirection, HTuple hv_WindowHandle, HTuple *hv_BboxClassIndices)
{

  // Local iconic variables
  HObject  ho_InstanceMask, ho_BboxRectangle, ho_OrientationArrows;
  HObject  ho_MaskSelected, ho_RectangleSelected, ho_ArrowSelected;

  // Local control variables
  HTuple  hv_InstanceType, hv_MaskExists, hv_BboxRow1;
  HTuple  hv_BboxCol1, hv_BboxRow2, hv_BboxCol2, hv_BboxRow;
  HTuple  hv_BboxCol, hv_BboxLength1, hv_BboxLength2, hv_BboxPhi;
  HTuple  hv_BboxClasses, hv_Text, hv_Ascent, hv_Descent;
  HTuple  hv__, hv_TextOffset, hv_LabelRowTop, hv_LabelRowBottom;
  HTuple  hv_LabelCol, hv_ArrowSizeFactorLength, hv_ArrowSizeFactorHead;
  HTuple  hv_MaxLengthArrow, hv_HalfLengthArrow, hv_ArrowBaseRow;
  HTuple  hv_ArrowBaseCol, hv_ArrowHeadRow, hv_ArrowHeadCol;
  HTuple  hv_ArrowHeadSize, hv_MaskRow, hv_MaskCol, hv_ContourStyle;
  HTuple  hv_Style, hv_LineWidths, hv_IndexBbox, hv_ClassID;
  HTuple  hv_CurrentColors, hv_IndexStyle, hv_TextColorClasses;
  HTuple  hv_LabelRow;

  //
  //This procedure displays the bounding boxes and masks (if present) defined by a DLResult.
  //The ClassIDs are necessary to display bounding boxes from the same class
  //always with the same color.
  //
  hv_InstanceType = "";
  hv_MaskExists = 0;
  if (0 != (int((hv_ResultKeys.TupleFind("bbox_row1"))!=-1)))
  {
    GetDictTuple(hv_DLResult, "bbox_row1", &hv_BboxRow1);
    GetDictTuple(hv_DLResult, "bbox_col1", &hv_BboxCol1);
    GetDictTuple(hv_DLResult, "bbox_row2", &hv_BboxRow2);
    GetDictTuple(hv_DLResult, "bbox_col2", &hv_BboxCol2);
    hv_InstanceType = "rectangle1";
  }
  else if (0 != (int((hv_ResultKeys.TupleFind("bbox_phi"))!=-1)))
  {
    GetDictTuple(hv_DLResult, "bbox_row", &hv_BboxRow);
    GetDictTuple(hv_DLResult, "bbox_col", &hv_BboxCol);
    GetDictTuple(hv_DLResult, "bbox_length1", &hv_BboxLength1);
    GetDictTuple(hv_DLResult, "bbox_length2", &hv_BboxLength2);
    GetDictTuple(hv_DLResult, "bbox_phi", &hv_BboxPhi);
    GetDictTuple(hv_DLResult, "bbox_class_id", &hv_BboxClasses);
    hv_InstanceType = "rectangle2";
  }
  else
  {
    throw HException("Result bounding box data could not be found in DLResult.");
  }
  if (0 != (int((hv_ResultKeys.TupleFind("mask"))!=-1)))
  {
    GetDictObject(&ho_InstanceMask, hv_DLResult, "mask");
    hv_MaskExists = 1;
  }
  if (0 != (HTuple(HTuple(int(hv_InstanceType!=HTuple("rectangle1"))).TupleAnd(int(hv_InstanceType!=HTuple("rectangle2")))).TupleAnd(hv_MaskExists.TupleNot())))
  {
    throw HException("Result bounding box or mask data could not be found in DLSample.");
  }
  GetDictTuple(hv_DLResult, "bbox_class_id", &hv_BboxClasses);
  if (0 != (int((hv_BboxClasses.TupleLength())>0)))
  {
    //
    //Get text and text size for correct positioning of result class IDs.
    if (0 != hv_ShowLabels)
    {
      hv_Text = hv_BboxClasses+hv_TextConf;
      GetStringExtents(hv_WindowHandle, hv_Text, &hv_Ascent, &hv_Descent, &hv__, 
          &hv__);
      hv_TextOffset = (hv_Ascent+hv_Descent)/hv_WindowImageRatio;
    }
    //
    //Generate bounding box XLDs.
    if (0 != (int(hv_InstanceType==HTuple("rectangle1"))))
    {
      TupleGenConst(hv_BboxRow1.TupleLength(), 0.0, &hv_BboxPhi);
      GenRectangle2ContourXld(&ho_BboxRectangle, 0.5*(hv_BboxRow1+hv_BboxRow2), 0.5*(hv_BboxCol1+hv_BboxCol2), 
          hv_BboxPhi, 0.5*(hv_BboxCol2-hv_BboxCol1), 0.5*(hv_BboxRow2-hv_BboxRow1));
      if (0 != hv_ShowLabels)
      {
        hv_LabelRowTop = hv_BboxRow1;
        hv_LabelRowBottom = hv_BboxRow2-hv_TextOffset;
        hv_LabelCol = hv_BboxCol1;
      }
    }
    else if (0 != (int(hv_InstanceType==HTuple("rectangle2"))))
    {
      GenRectangle2ContourXld(&ho_BboxRectangle, hv_BboxRow, hv_BboxCol, hv_BboxPhi, 
          hv_BboxLength1, hv_BboxLength2);
      if (0 != hv_ShowLabels)
      {
        hv_LabelRowTop = hv_BboxRow-hv_TextOffset;
        hv_LabelRowBottom = hv_BboxRow;
        hv_LabelCol = hv_BboxCol;
      }
      if (0 != hv_ShowDirection)
      {
        if (0 != (int(hv_ShowDirection==-1)))
        {
          hv_ArrowSizeFactorLength = 0.4;
          hv_ArrowSizeFactorHead = 0.2;
          hv_MaxLengthArrow = 20;
          hv_HalfLengthArrow = hv_MaxLengthArrow.TupleMin2(hv_BboxLength1*hv_ArrowSizeFactorLength);
          hv_ArrowBaseRow = hv_BboxRow-((hv_BboxLength1-hv_HalfLengthArrow)*(hv_BboxPhi.TupleSin()));
          hv_ArrowBaseCol = hv_BboxCol+((hv_BboxLength1-hv_HalfLengthArrow)*(hv_BboxPhi.TupleCos()));
          hv_ArrowHeadRow = hv_BboxRow-((hv_BboxLength1+hv_HalfLengthArrow)*(hv_BboxPhi.TupleSin()));
          hv_ArrowHeadCol = hv_BboxCol+((hv_BboxLength1+hv_HalfLengthArrow)*(hv_BboxPhi.TupleCos()));
          hv_ArrowHeadSize = (hv_MaxLengthArrow.TupleMin2(hv_BboxLength1.TupleMin2(hv_BboxLength2)))*hv_ArrowSizeFactorHead;
        }
        else
        {
          hv_ArrowHeadSize = 20.0;
          hv_ArrowBaseRow = hv_BboxRow;
          hv_ArrowBaseCol = hv_BboxCol;
          hv_ArrowHeadRow = hv_BboxRow-((hv_BboxLength1+hv_ArrowHeadSize)*(hv_BboxPhi.TupleSin()));
          hv_ArrowHeadCol = hv_BboxCol+((hv_BboxLength1+hv_ArrowHeadSize)*(hv_BboxPhi.TupleCos()));
        }
        gen_arrow_contour_xld(&ho_OrientationArrows, hv_ArrowBaseRow, hv_ArrowBaseCol, 
            hv_ArrowHeadRow, hv_ArrowHeadCol, hv_ArrowHeadSize, hv_ArrowHeadSize);
      }
    }
    else if (0 != hv_MaskExists)
    {
      AreaCenter(ho_InstanceMask, &hv__, &hv_MaskRow, &hv_MaskCol);
      hv_LabelRowTop = hv_MaskRow-hv_TextOffset;
      hv_LabelRowBottom = hv_MaskRow;
      hv_LabelCol = hv_MaskCol;
    }
    else
    {
      throw HException("Unknown instance_type: "+hv_InstanceType);
    }
    //
    GetContourStyle(hv_WindowHandle, &hv_ContourStyle);
    if (HDevWindowStack::IsOpen())
      SetContourStyle(HDevWindowStack::GetActive(),"stroke");
    GetLineStyle(hv_WindowHandle, &hv_Style);
    hv_LineWidths.Clear();
    hv_LineWidths.Append(hv_LineWidthBbox+2);
    hv_LineWidths.Append(hv_LineWidthBbox);
    if (HDevWindowStack::IsOpen())
      SetLineWidth(HDevWindowStack::GetActive(),hv_LineWidthBbox);
    //
    //Collect ClassIDs of the bounding boxes.
    TupleGenConst(hv_BboxClasses.TupleLength(), 0, &(*hv_BboxClassIndices));
    //
    //Draw bounding boxes.
    {
    HTuple end_val96 = (hv_BboxClasses.TupleLength())-1;
    HTuple step_val96 = 1;
    for (hv_IndexBbox=0; hv_IndexBbox.Continue(end_val96, step_val96); hv_IndexBbox += step_val96)
    {
      hv_ClassID = hv_ClassIDs.TupleFind(HTuple(hv_BboxClasses[hv_IndexBbox]));
      (*hv_BboxClassIndices)[hv_IndexBbox] = hv_ClassID;
      //First draw in black to make the class-color visible.
      hv_CurrentColors.Clear();
      hv_CurrentColors[0] = "black";
      hv_CurrentColors.Append(HTuple(hv_Colors[hv_ClassID]));
      if (0 != hv_MaskExists)
      {
        SelectObj(ho_InstanceMask, &ho_MaskSelected, hv_IndexBbox+1);
        if (HDevWindowStack::IsOpen())
          SetDraw(HDevWindowStack::GetActive(),"fill");
        if (HDevWindowStack::IsOpen())
          SetColor(HDevWindowStack::GetActive(),HTuple(hv_Colors[hv_ClassID])+"80");
        if (HDevWindowStack::IsOpen())
          DispObj(ho_MaskSelected, HDevWindowStack::GetActive());
        if (HDevWindowStack::IsOpen())
          SetDraw(HDevWindowStack::GetActive(),"margin");
      }
      {
      HTuple end_val108 = (hv_CurrentColors.TupleLength())-1;
      HTuple step_val108 = 1;
      for (hv_IndexStyle=0; hv_IndexStyle.Continue(end_val108, step_val108); hv_IndexStyle += step_val108)
      {
        if (HDevWindowStack::IsOpen())
          SetColor(HDevWindowStack::GetActive(),HTuple(hv_CurrentColors[hv_IndexStyle]));
        if (HDevWindowStack::IsOpen())
          SetLineWidth(HDevWindowStack::GetActive(),HTuple(hv_LineWidths[hv_IndexStyle]));
        if (0 != (int(hv_InstanceType!=HTuple(""))))
        {
          SelectObj(ho_BboxRectangle, &ho_RectangleSelected, hv_IndexBbox+1);
          if (HDevWindowStack::IsOpen())
            DispObj(ho_RectangleSelected, HDevWindowStack::GetActive());
          if (0 != (HTuple(int(hv_InstanceType==HTuple("rectangle2"))).TupleAnd(hv_ShowDirection)))
          {
            SelectObj(ho_OrientationArrows, &ho_ArrowSelected, hv_IndexBbox+1);
            if (HDevWindowStack::IsOpen())
              DispObj(ho_ArrowSelected, HDevWindowStack::GetActive());
          }
        }
      }
      }
    }
    }
    //
    //Draw text of bounding boxes.
    if (0 != hv_ShowLabels)
    {
      //For better visibility the text is displayed after all bounding boxes are drawn.
      //Get text and text size for correct positioning of result class IDs.
      hv_Text = hv_BboxClasses+hv_TextConf;
      //Select text color.
      if (0 != (int(hv_TextColor==HTuple(""))))
      {
        hv_TextColorClasses = HTuple(hv_Colors[(*hv_BboxClassIndices)]);
      }
      else
      {
        TupleGenConst((*hv_BboxClassIndices).TupleLength(), hv_TextColor, &hv_TextColorClasses);
      }
      //Select correct position of the text.
      hv_LabelRow = hv_LabelRowTop;
      if (0 != (int(hv_TextPositionRow==HTuple("bottom"))))
      {
        hv_LabelRow = hv_LabelRowBottom;
      }
      //Display text.
      if (HDevWindowStack::IsOpen())
        DispText(HDevWindowStack::GetActive(),hv_Text, "image", hv_LabelRow, hv_LabelCol, 
            hv_TextColorClasses, ((HTuple("box_color").Append("shadow")).Append("border_radius")), 
            hv_BoxLabelColor.TupleConcat((HTuple("false").Append(0))));
    }
    //
    if (HDevWindowStack::IsOpen())
      SetContourStyle(HDevWindowStack::GetActive(),hv_ContourStyle);
    SetLineStyle(hv_WindowHandle, hv_Style);
  }
  else
  {
    //Do nothing if no results are present.
    (*hv_BboxClassIndices) = HTuple();
  }
  //
  return;
}

// Chapter: Graphics / Output
// Short Description: Display the ground truth/result segmentation as regions. 
void dev_display_segmentation_regions (HObject ho_SegmentationImage, HTuple hv_ClassIDs, 
    HTuple hv_ColorsSegmentation, HTuple hv_ExcludeClassIDs, HTuple *hv_ImageClassIDs)
{

  // Local iconic variables
  HObject  ho_Regions, ho_SelectedRegion;

  // Local control variables
  HTuple  hv_IncludedClassIDs, hv_Area, hv_Index;
  HTuple  hv_ClassID, hv_IndexColor;

  //
  //This procedure displays the ground truth/result segmentation
  //given in SegmentationImage as regions. The ClassIDs are necessary to
  //display ground truth/result segmentations from the same class
  //always with the same color. It is possible to exclude certain ClassIDs
  //from being displayed. The displayed classes are returned in ImageClassIDs.
  //
  //
  //Remove excluded class IDs from the list.
  hv_IncludedClassIDs = hv_ClassIDs.TupleDifference(hv_ExcludeClassIDs);
  //
  //Get a region for each class ID.
  Threshold(ho_SegmentationImage, &ho_Regions, hv_IncludedClassIDs, hv_IncludedClassIDs);
  //
  //Get classes with non-empty regions.
  RegionFeatures(ho_Regions, "area", &hv_Area);
  if (0 != (int((hv_Area.TupleLength())!=(hv_IncludedClassIDs.TupleLength()))))
  {
    throw HException("No equal number of class IDs and segmentation regions.");
  }
  TupleSelectMask(hv_IncludedClassIDs, hv_Area.TupleGreaterElem(0), &(*hv_ImageClassIDs));
  //
  //Display all non-empty class regions in distinct colors.
  {
  HTuple end_val22 = (hv_IncludedClassIDs.TupleLength())-1;
  HTuple step_val22 = 1;
  for (hv_Index=0; hv_Index.Continue(end_val22, step_val22); hv_Index += step_val22)
  {
    if (0 != (int(HTuple(hv_Area[hv_Index])>0)))
    {
      //Use class ID to determine region color.
      hv_ClassID = HTuple(hv_IncludedClassIDs[hv_Index]);
      hv_IndexColor = hv_ClassIDs.TupleFindFirst(hv_ClassID);
      if (HDevWindowStack::IsOpen())
        SetColor(HDevWindowStack::GetActive(),HTuple(hv_ColorsSegmentation[hv_IndexColor]));
      //Display the segmentation region.
      SelectObj(ho_Regions, &ho_SelectedRegion, hv_Index+1);
      if (HDevWindowStack::IsOpen())
        DispObj(ho_SelectedRegion, HDevWindowStack::GetActive());
    }
  }
  }
  return;
}

// Chapter: Graphics / Output
// Short Description: Display a map of weights. 
void dev_display_weight_regions (HObject ho_ImageWeight, HTuple hv_DrawTransparency, 
    HTuple hv_SegMaxWeight, HTuple *hv_Colors)
{

  // Local iconic variables
  HObject  ho_Domain, ho_WeightsRegion;

  // Local control variables
  HTuple  hv_NumColors, hv_WeightsColorsAlpha, hv_Rows;
  HTuple  hv_Columns, hv_GrayVal, hv_GrayValWeight, hv_ColorIndex;
  HTuple  hv_ClassColor;

  //
  //This procedure displays a map of the weights
  //given in ImageWeight as regions.
  //The transparency can be adjusted.
  //The used colors are returned.
  //
  //Define colors.
  hv_NumColors = 20;
  get_distinct_colors(hv_NumColors, 0, 0, 160, &(*hv_Colors));
  TupleInverse((*hv_Colors), &(*hv_Colors));
  hv_WeightsColorsAlpha = (*hv_Colors)+hv_DrawTransparency;
  //
  //Get gay values of ImageWeight.
  GetDomain(ho_ImageWeight, &ho_Domain);
  GetRegionPoints(ho_Domain, &hv_Rows, &hv_Columns);
  GetGrayval(ho_ImageWeight, hv_Rows, hv_Columns, &hv_GrayVal);
  //
  //Check that the gray values of the image
  //are below the specified maximum.
  if (0 != (int((hv_GrayVal.TupleMax())>hv_SegMaxWeight)))
  {
    throw HException(((("The maximum weight ("+(hv_GrayVal.TupleMax()))+") in the weight image is greater than the given SegMaxWeight (")+hv_SegMaxWeight)+").");
  }
  //
  while (0 != (int(hv_GrayVal!=HTuple())))
  {
    //Go through all gray value 'groups',
    //starting from the maximum.
    hv_GrayValWeight = hv_GrayVal.TupleMax();
    hv_GrayVal = hv_GrayVal.TupleRemove(hv_GrayVal.TupleFind(hv_GrayValWeight));
    Threshold(ho_ImageWeight, &ho_WeightsRegion, hv_GrayValWeight, hv_GrayValWeight);
    //
    //Visualize the respective group.
    hv_ColorIndex = (((hv_GrayValWeight/hv_SegMaxWeight)*(hv_NumColors-1)).TupleCeil()).TupleInt();
    hv_ClassColor = HTuple(hv_WeightsColorsAlpha[hv_ColorIndex]);
    if (HDevWindowStack::IsOpen())
      SetColor(HDevWindowStack::GetActive(),hv_ClassColor);
    if (HDevWindowStack::IsOpen())
      DispObj(ho_WeightsRegion, HDevWindowStack::GetActive());
  }
  return;
}

// Chapter: Develop
// Short Description: Open a new graphics window that preserves the aspect ratio of the given image size. 
void dev_open_window_fit_size (HTuple hv_Row, HTuple hv_Column, HTuple hv_Width, 
    HTuple hv_Height, HTuple hv_WidthLimit, HTuple hv_HeightLimit, HTuple *hv_WindowHandle)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_MinWidth, hv_MaxWidth, hv_MinHeight;
  HTuple  hv_MaxHeight, hv_ResizeFactor, hv_TempWidth, hv_TempHeight;
  HTuple  hv_WindowWidth, hv_WindowHeight;

  //This procedure open a new graphic window
  //such that it fits into the limits specified by WidthLimit
  //and HeightLimit, but also maintains the correct aspect ratio
  //given by Width and Height.
  //
  //If it is impossible to match the minimum and maximum extent requirements
  //at the same time (f.e. if the image is very long but narrow),
  //the maximum value gets a higher priority.
  //
  //Parse input tuple WidthLimit
  if (0 != (HTuple(int((hv_WidthLimit.TupleLength())==0)).TupleOr(int(hv_WidthLimit<0))))
  {
    hv_MinWidth = 500;
    hv_MaxWidth = 800;
  }
  else if (0 != (int((hv_WidthLimit.TupleLength())==1)))
  {
    hv_MinWidth = 0;
    hv_MaxWidth = hv_WidthLimit;
  }
  else
  {
    hv_MinWidth = ((const HTuple&)hv_WidthLimit)[0];
    hv_MaxWidth = ((const HTuple&)hv_WidthLimit)[1];
  }
  //Parse input tuple HeightLimit
  if (0 != (HTuple(int((hv_HeightLimit.TupleLength())==0)).TupleOr(int(hv_HeightLimit<0))))
  {
    hv_MinHeight = 400;
    hv_MaxHeight = 600;
  }
  else if (0 != (int((hv_HeightLimit.TupleLength())==1)))
  {
    hv_MinHeight = 0;
    hv_MaxHeight = hv_HeightLimit;
  }
  else
  {
    hv_MinHeight = ((const HTuple&)hv_HeightLimit)[0];
    hv_MaxHeight = ((const HTuple&)hv_HeightLimit)[1];
  }
  //
  //Test, if window size has to be changed.
  hv_ResizeFactor = 1;
  //First, expand window to the minimum extents (if necessary).
  if (0 != (HTuple(int(hv_MinWidth>hv_Width)).TupleOr(int(hv_MinHeight>hv_Height))))
  {
    hv_ResizeFactor = (((hv_MinWidth.TupleReal())/hv_Width).TupleConcat((hv_MinHeight.TupleReal())/hv_Height)).TupleMax();
  }
  hv_TempWidth = hv_Width*hv_ResizeFactor;
  hv_TempHeight = hv_Height*hv_ResizeFactor;
  //Then, shrink window to maximum extents (if necessary).
  if (0 != (HTuple(int(hv_MaxWidth<hv_TempWidth)).TupleOr(int(hv_MaxHeight<hv_TempHeight))))
  {
    hv_ResizeFactor = hv_ResizeFactor*((((hv_MaxWidth.TupleReal())/hv_TempWidth).TupleConcat((hv_MaxHeight.TupleReal())/hv_TempHeight)).TupleMin());
  }
  hv_WindowWidth = hv_Width*hv_ResizeFactor;
  hv_WindowHeight = hv_Height*hv_ResizeFactor;
  //Resize window
  SetWindowAttr("background_color","black");
  OpenWindow(hv_Row,hv_Column,hv_WindowWidth,hv_WindowHeight,0,"visible","",&(*hv_WindowHandle));
  HDevWindowStack::Push((*hv_WindowHandle));
  if (HDevWindowStack::IsOpen())
    SetPart(HDevWindowStack::GetActive(),0, 0, hv_Height-1, hv_Width-1);
  return;
}

// Chapter: Develop
// Short Description: Switch dev_update_pc, dev_update_var, and dev_update_window to 'off'. 
void dev_update_off ()
{

  //This procedure sets different update settings to 'off'.
  //This is useful to get the best performance and reduce overhead.
  //
  // dev_update_pc(...); only in hdevelop
  // dev_update_var(...); only in hdevelop
  // dev_update_window(...); only in hdevelop
  return;
}

// Chapter: Deep Learning / Object Detection and Instance Segmentation
// Short Description: Filter the instance segmentation masks of a DL sample based on a given selection. 
void filter_dl_sample_instance_segmentation_masks (HTuple hv_DLSample, HTuple hv_BBoxSelectionMask)
{

  // Local iconic variables
  HObject  ho_EmptyMasks, ho_Masks;

  // Local control variables
  HTuple  hv_MaskKeyExists, hv_Indices;

  GetDictParam(hv_DLSample, "key_exists", "mask", &hv_MaskKeyExists);
  if (0 != hv_MaskKeyExists)
  {
    //Only if masks exist (-> instance segmentation).
    TupleFind(hv_BBoxSelectionMask, 1, &hv_Indices);
    if (0 != (int(hv_Indices==-1)))
    {
      //We define here that this case will result in an empty object value
      //for the mask key. Another option would be to remove the
      //key 'mask'. However, this would be an unwanted big change in the dictionary.
      GenEmptyObj(&ho_EmptyMasks);
      SetDictObject(ho_EmptyMasks, hv_DLSample, "mask");
    }
    else
    {
      GetDictObject(&ho_Masks, hv_DLSample, "mask");
      //Remove all unused masks.
      SelectObj(ho_Masks, &ho_Masks, hv_Indices+1);
      SetDictObject(ho_Masks, hv_DLSample, "mask");
    }
  }
  return;
}

// Chapter: XLD / Creation
// Short Description: Create an arrow shaped XLD contour. 
void gen_arrow_contour_xld (HObject *ho_Arrow, HTuple hv_Row1, HTuple hv_Column1, 
    HTuple hv_Row2, HTuple hv_Column2, HTuple hv_HeadLength, HTuple hv_HeadWidth)
{

  // Local iconic variables
  HObject  ho_TempArrow;

  // Local control variables
  HTuple  hv_Length, hv_ZeroLengthIndices, hv_DR;
  HTuple  hv_DC, hv_HalfHeadWidth, hv_RowP1, hv_ColP1, hv_RowP2;
  HTuple  hv_ColP2, hv_Index;

  //This procedure generates arrow shaped XLD contours,
  //pointing from (Row1, Column1) to (Row2, Column2).
  //If starting and end point are identical, a contour consisting
  //of a single point is returned.
  //
  //input parameters:
  //Row1, Column1: Coordinates of the arrows' starting points
  //Row2, Column2: Coordinates of the arrows' end points
  //HeadLength, HeadWidth: Size of the arrow heads in pixels
  //
  //output parameter:
  //Arrow: The resulting XLD contour
  //
  //The input tuples Row1, Column1, Row2, and Column2 have to be of
  //the same length.
  //HeadLength and HeadWidth either have to be of the same length as
  //Row1, Column1, Row2, and Column2 or have to be a single element.
  //If one of the above restrictions is violated, an error will occur.
  //
  //
  //Initialization.
  GenEmptyObj(&(*ho_Arrow));
  //
  //Calculate the arrow length
  DistancePp(hv_Row1, hv_Column1, hv_Row2, hv_Column2, &hv_Length);
  //
  //Mark arrows with identical start and end point
  //(set Length to -1 to avoid division-by-zero exception)
  hv_ZeroLengthIndices = hv_Length.TupleFind(0);
  if (0 != (int(hv_ZeroLengthIndices!=-1)))
  {
    hv_Length[hv_ZeroLengthIndices] = -1;
  }
  //
  //Calculate auxiliary variables.
  hv_DR = (1.0*(hv_Row2-hv_Row1))/hv_Length;
  hv_DC = (1.0*(hv_Column2-hv_Column1))/hv_Length;
  hv_HalfHeadWidth = hv_HeadWidth/2.0;
  //
  //Calculate end points of the arrow head.
  hv_RowP1 = (hv_Row1+((hv_Length-hv_HeadLength)*hv_DR))+(hv_HalfHeadWidth*hv_DC);
  hv_ColP1 = (hv_Column1+((hv_Length-hv_HeadLength)*hv_DC))-(hv_HalfHeadWidth*hv_DR);
  hv_RowP2 = (hv_Row1+((hv_Length-hv_HeadLength)*hv_DR))-(hv_HalfHeadWidth*hv_DC);
  hv_ColP2 = (hv_Column1+((hv_Length-hv_HeadLength)*hv_DC))+(hv_HalfHeadWidth*hv_DR);
  //
  //Finally create output XLD contour for each input point pair
  {
  HTuple end_val45 = (hv_Length.TupleLength())-1;
  HTuple step_val45 = 1;
  for (hv_Index=0; hv_Index.Continue(end_val45, step_val45); hv_Index += step_val45)
  {
    if (0 != (int(HTuple(hv_Length[hv_Index])==-1)))
    {
      //Create_ single points for arrows with identical start and end point
      GenContourPolygonXld(&ho_TempArrow, HTuple(hv_Row1[hv_Index]), HTuple(hv_Column1[hv_Index]));
    }
    else
    {
      //Create arrow contour
      GenContourPolygonXld(&ho_TempArrow, ((((HTuple(hv_Row1[hv_Index]).TupleConcat(HTuple(hv_Row2[hv_Index]))).TupleConcat(HTuple(hv_RowP1[hv_Index]))).TupleConcat(HTuple(hv_Row2[hv_Index]))).TupleConcat(HTuple(hv_RowP2[hv_Index]))).TupleConcat(HTuple(hv_Row2[hv_Index])), 
          ((((HTuple(hv_Column1[hv_Index]).TupleConcat(HTuple(hv_Column2[hv_Index]))).TupleConcat(HTuple(hv_ColP1[hv_Index]))).TupleConcat(HTuple(hv_Column2[hv_Index]))).TupleConcat(HTuple(hv_ColP2[hv_Index]))).TupleConcat(HTuple(hv_Column2[hv_Index])));
    }
    ConcatObj((*ho_Arrow), ho_TempArrow, &(*ho_Arrow));
  }
  }
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Generate ground truth characters if they don't exist and words to characters mapping. 
void gen_dl_ocr_detection_gt_chars (HTuple hv_DLSampleTargets, HTuple hv_DLSample, 
    HTuple hv_ScaleWidth, HTuple hv_ScaleHeight, HTupleVector/*{eTupleVector,Dim=1}*/ *hvec_WordsCharsMapping)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_CharBoxIndex, hv_WordLengths, hv_J;
  HTuple  hv_Start, hv_End, hv_SplitRow, hv_SplitColumn, hv_SplitPhi;
  HTuple  hv_SplitLength1, hv_SplitLength2, hv_CharsIds, hv_EmptyWordStrings;

  (*hvec_WordsCharsMapping)[0] = HTupleVector(HTuple());
  if (0 != (int(((hv_DLSample.TupleGetDictTuple("bbox_label_id")).TupleLength())>0)))
  {
    //Check if chars GT exist otherwise generate them.
    TupleFindFirst(hv_DLSample.TupleGetDictTuple("bbox_label_id"), 1, &hv_CharBoxIndex);
    if (0 != (int(hv_CharBoxIndex==-1)))
    {
      hv_WordLengths = (hv_DLSample.TupleGetDictTuple("word")).TupleStrlen();
      (*hvec_WordsCharsMapping)[((hv_DLSample.TupleGetDictTuple("bbox_label_id")).TupleLength())-1] = HTupleVector(HTuple());
      {
      HTuple end_val7 = ((hv_DLSample.TupleGetDictTuple("bbox_label_id")).TupleLength())-1;
      HTuple step_val7 = 1;
      for (hv_J=0; hv_J.Continue(end_val7, step_val7); hv_J += step_val7)
      {
        //For each word box
        if (0 != (int(HTuple((hv_DLSample.TupleGetDictTuple("bbox_label_id"))[hv_J])==0)))
        {
          if (0 != (int(HTuple(hv_WordLengths[hv_J])!=0)))
          {
            hv_Start = (hv_DLSampleTargets.TupleGetDictTuple("bbox_label_id")).TupleLength();
            hv_End = (((hv_DLSampleTargets.TupleGetDictTuple("bbox_label_id")).TupleLength())-1)+HTuple(hv_WordLengths[hv_J]);
            (*hvec_WordsCharsMapping)[hv_J] = HTupleVector(HTuple::TupleGenSequence(hv_Start,hv_End,1));
            split_rectangle2(HTuple((hv_DLSample.TupleGetDictTuple("bbox_row"))[hv_J]), 
                HTuple((hv_DLSample.TupleGetDictTuple("bbox_col"))[hv_J]), HTuple((hv_DLSample.TupleGetDictTuple("bbox_phi"))[hv_J]), 
                HTuple((hv_DLSample.TupleGetDictTuple("bbox_length1"))[hv_J]), HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_J]), 
                HTuple(hv_WordLengths[hv_J]), &hv_SplitRow, &hv_SplitColumn, &hv_SplitPhi, 
                &hv_SplitLength1, &hv_SplitLength2);
            TupleGenConst(HTuple(hv_WordLengths[hv_J]), 1, &hv_CharsIds);
            TupleGenConst(HTuple(hv_WordLengths[hv_J]), "", &hv_EmptyWordStrings);
            SetDictTuple(hv_DLSampleTargets, "bbox_label_id", (hv_DLSampleTargets.TupleGetDictTuple("bbox_label_id")).TupleConcat(hv_CharsIds));
            SetDictTuple(hv_DLSampleTargets, "bbox_row", (hv_DLSampleTargets.TupleGetDictTuple("bbox_row")).TupleConcat(hv_SplitRow));
            SetDictTuple(hv_DLSampleTargets, "bbox_col", (hv_DLSampleTargets.TupleGetDictTuple("bbox_col")).TupleConcat(hv_SplitColumn));
            SetDictTuple(hv_DLSampleTargets, "bbox_phi", (hv_DLSampleTargets.TupleGetDictTuple("bbox_phi")).TupleConcat(hv_SplitPhi));
            SetDictTuple(hv_DLSampleTargets, "bbox_length1", (hv_DLSampleTargets.TupleGetDictTuple("bbox_length1")).TupleConcat(hv_SplitLength1*hv_ScaleWidth));
            SetDictTuple(hv_DLSampleTargets, "bbox_length2", (hv_DLSampleTargets.TupleGetDictTuple("bbox_length2")).TupleConcat(hv_SplitLength2*hv_ScaleHeight));
            SetDictTuple(hv_DLSampleTargets, "word", (hv_DLSampleTargets.TupleGetDictTuple("word")).TupleConcat(hv_EmptyWordStrings));
          }
          else
          {
            throw HException(((("Sample with image id "+(hv_DLSample.TupleGetDictTuple("image_id")))+" is not valid. The word bounding box at index ")+hv_J)+" has an empty string as the ground truth. This is not allowed. Please assign a word label to every word bounding box.");
          }
        }
      }
      }
    }
    else
    {
      gen_words_chars_mapping(hv_DLSample, &(*hvec_WordsCharsMapping));
    }
  }
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Generate target link score map for ocr detection training. 
void gen_dl_ocr_detection_gt_link_map (HObject *ho_GtLinkMap, HTuple hv_ImageWidth, 
    HTuple hv_ImageHeight, HTuple hv_DLSampleTargets, HTupleVector/*{eTupleVector,Dim=1}*/ hvec_WordToCharVec, 
    HTuple hv_Alpha)
{

  // Local iconic variables
  HObject  ho_Lines, ho_Line, ho_LineDilated;

  // Local control variables
  HTuple  hv_InitImage, hv_CRow, hv_CCol, hv_DiameterC;
  HTuple  hv_IndexW, hv_CharBoxIndices, hv_CharCRows, hv_CharCCols;
  HTuple  hv_CharDistToWordCenter, hv_ExtremeCharIndex, hv_DistToExtreme;
  HTuple  hv_CharIndexSorted, hv_Box1Idx, hv_Box2Idx, hv_Diameter1;
  HTuple  hv_Diameter2, hv_DilationRadius, hv_NumLines, hv_Index;

  GenImageConst(&(*ho_GtLinkMap), "real", hv_ImageWidth, hv_ImageHeight);
  GetSystem("init_new_image", &hv_InitImage);
  if (0 != (int(hv_InitImage==HTuple("false"))))
  {
    OverpaintRegion((*ho_GtLinkMap), (*ho_GtLinkMap), 0.0, "fill");
  }
  //Compute box centers.
  hv_CRow = hv_DLSampleTargets.TupleGetDictTuple("bbox_row");
  hv_CCol = hv_DLSampleTargets.TupleGetDictTuple("bbox_col");
  hv_DiameterC = 2*((hv_DLSampleTargets.TupleGetDictTuple("bbox_length1")).TupleHypot(hv_DLSampleTargets.TupleGetDictTuple("bbox_length2")));
  //Loop over word boxes.
  {
  HTuple end_val10 = ((hv_DLSampleTargets.TupleGetDictTuple("bbox_label_id")).TupleLength())-1;
  HTuple step_val10 = 1;
  for (hv_IndexW=0; hv_IndexW.Continue(end_val10, step_val10); hv_IndexW += step_val10)
  {
    //For each word box
    if (0 != (int(HTuple((hv_DLSampleTargets.TupleGetDictTuple("bbox_label_id"))[hv_IndexW])==0)))
    {
      hv_CharBoxIndices = hvec_WordToCharVec[hv_IndexW].T();
      if (0 != (int((hv_CharBoxIndices.TupleLength())==0)))
      {
        continue;
      }
      else if (0 != (int((hv_CharBoxIndices.TupleLength())==1)))
      {
        //Generate a dot in the char center.
        GenCircle(&ho_Lines, HTuple(hv_CRow[hv_CharBoxIndices]), HTuple(hv_CCol[hv_CharBoxIndices]), 
            (((0.5*hv_Alpha)*HTuple(hv_DiameterC[hv_CharBoxIndices])).TupleRound())+0.5);
      }
      else
      {
        //Generate link lines between chars.
        hv_CharCRows = HTuple(hv_CRow[hv_CharBoxIndices]);
        hv_CharCCols = HTuple(hv_CCol[hv_CharBoxIndices]);
        //Sort the char boxes within the word.
        hv_CharDistToWordCenter = (hv_CharCRows-HTuple(hv_CRow[hv_IndexW])).TupleHypot(hv_CharCCols-HTuple(hv_CCol[hv_IndexW]));
        hv_ExtremeCharIndex = ((const HTuple&)HTuple(hv_CharDistToWordCenter.TupleSortIndex()))[(hv_CharDistToWordCenter.TupleLength())-1];
        hv_DistToExtreme = (hv_CharCRows-HTuple(hv_CharCRows[hv_ExtremeCharIndex])).TupleHypot(hv_CharCCols-HTuple(hv_CharCCols[hv_ExtremeCharIndex]));
        hv_CharIndexSorted = hv_DistToExtreme.TupleSortIndex();
        //Get the indices of adjacent characters.
        hv_Box1Idx = hv_CharIndexSorted.TupleSelectRange(0,(hv_CharIndexSorted.TupleLength())-2);
        hv_Box2Idx = hv_CharIndexSorted.TupleSelectRange(1,(hv_CharIndexSorted.TupleLength())-1);
        //Generate link lines between each pair of adjacent characters.
        GenRegionLine(&ho_Lines, HTuple(hv_CharCRows[hv_Box1Idx]), HTuple(hv_CharCCols[hv_Box1Idx]), 
            HTuple(hv_CharCRows[hv_Box2Idx]), HTuple(hv_CharCCols[hv_Box2Idx]));
        //Dilate the lines by 0.5/1.5/2.5/... pixels, such that the line thickness is approximately Alpha*mean(D1, D2)
        hv_Diameter1 = HTuple(hv_DiameterC[HTuple(hv_CharBoxIndices[hv_Box1Idx])]);
        hv_Diameter2 = HTuple(hv_DiameterC[HTuple(hv_CharBoxIndices[hv_Box2Idx])]);
        hv_DilationRadius = (((0.25*hv_Alpha)*(hv_Diameter1+hv_Diameter2)).TupleRound())+0.5;
        //dilation_circle only accepts a single radius, so we need to loop over the lines.
        CountObj(ho_Lines, &hv_NumLines);
        {
        HTuple end_val39 = hv_NumLines;
        HTuple step_val39 = 1;
        for (hv_Index=1; hv_Index.Continue(end_val39, step_val39); hv_Index += step_val39)
        {
          SelectObj(ho_Lines, &ho_Line, hv_Index);
          DilationCircle(ho_Line, &ho_LineDilated, HTuple(hv_DilationRadius[hv_Index-1]));
          ReplaceObj(ho_Lines, ho_LineDilated, &ho_Lines, hv_Index);
        }
        }
      }
      OverpaintRegion((*ho_GtLinkMap), ho_Lines, 1.0, "fill");
    }
  }
  }
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Generate target orientation score maps for ocr detection training. 
void gen_dl_ocr_detection_gt_orientation_map (HObject *ho_GtOrientationMaps, HTuple hv_ImageWidth, 
    HTuple hv_ImageHeight, HTuple hv_DLSample)
{

  // Local iconic variables
  HObject  ho_GtOrientationSin, ho_GtOrientationCos;
  HObject  ho_Region;

  // Local control variables
  HTuple  hv_InitImage, hv_Indices, hv_Phi;

  GenImageConst(&ho_GtOrientationSin, "real", hv_ImageWidth, hv_ImageHeight);
  GenImageConst(&ho_GtOrientationCos, "real", hv_ImageWidth, hv_ImageHeight);
  GetSystem("init_new_image", &hv_InitImage);
  if (0 != (int(hv_InitImage==HTuple("false"))))
  {
    OverpaintRegion(ho_GtOrientationSin, ho_GtOrientationSin, 0.0, "fill");
    OverpaintRegion(ho_GtOrientationCos, ho_GtOrientationCos, 0.0, "fill");
  }
  if (0 != (int(((hv_DLSample.TupleGetDictTuple("bbox_label_id")).TupleLength())>0)))
  {
    //Process char boxes
    TupleFind(hv_DLSample.TupleGetDictTuple("bbox_label_id"), 1, &hv_Indices);
    if (0 != (int(hv_Indices!=-1)))
    {
      hv_Phi = hv_DLSample.TupleGetDictTuple("bbox_phi");
      GenRectangle2(&ho_Region, HTuple((hv_DLSample.TupleGetDictTuple("bbox_row"))[hv_Indices]), 
          HTuple((hv_DLSample.TupleGetDictTuple("bbox_col"))[hv_Indices]), HTuple((hv_DLSample.TupleGetDictTuple("bbox_phi"))[hv_Indices]), 
          HTuple((hv_DLSample.TupleGetDictTuple("bbox_length1"))[hv_Indices]), HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_Indices]));
      OverpaintRegion(ho_GtOrientationSin, ho_Region, HTuple(hv_Phi[hv_Indices]).TupleSin(), 
          "fill");
      OverpaintRegion(ho_GtOrientationCos, ho_Region, HTuple(hv_Phi[hv_Indices]).TupleCos(), 
          "fill");
    }
  }
  Compose2(ho_GtOrientationSin, ho_GtOrientationCos, &(*ho_GtOrientationMaps));
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Generate target text score map for ocr detection training. 
void gen_dl_ocr_detection_gt_score_map (HObject *ho_TargetText, HTuple hv_DLSample, 
    HTuple hv_BoxCutoff, HTuple hv_RenderCutoff, HTuple hv_ImageWidth, HTuple hv_ImageHeight)
{

  // Local iconic variables
  HObject  ho_ExtendedRectangle;

  // Local control variables
  HTuple  hv_InitImage, hv_Index, hv_Sigma1, hv_Sigma2;
  HTuple  hv_ExtendedLength1, hv_ExtendedLength2, hv_Rows;
  HTuple  hv_Columns, hv_Area, hv_Row, hv_Column, hv_HomMat2D;
  HTuple  hv_DistRow, hv_DistCol, hv_ScaledGaussian, hv_Grayval;

  GenImageConst(&(*ho_TargetText), "real", hv_ImageWidth, hv_ImageHeight);
  GetSystem("init_new_image", &hv_InitImage);
  if (0 != (int(hv_InitImage==HTuple("false"))))
  {
    OverpaintRegion((*ho_TargetText), (*ho_TargetText), 0.0, "fill");
  }
  {
  HTuple end_val5 = ((hv_DLSample.TupleGetDictTuple("bbox_label_id")).TupleLength())-1;
  HTuple step_val5 = 1;
  for (hv_Index=0; hv_Index.Continue(end_val5, step_val5); hv_Index += step_val5)
  {
    //For each char box
    if (0 != (HTuple(int(HTuple((hv_DLSample.TupleGetDictTuple("bbox_label_id"))[hv_Index])==1)).TupleAnd(int(hv_BoxCutoff!=0))))
    {
      //Compute the sigma of an unnormalized normal distribution, such that
      //a certain threshold value is reached at the interval of a certain size.
      hv_Sigma1 = HTuple((hv_DLSample.TupleGetDictTuple("bbox_length1"))[hv_Index])*((-0.5/(hv_BoxCutoff.TupleLog())).TupleSqrt());
      hv_Sigma2 = HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_Index])*((-0.5/(hv_BoxCutoff.TupleLog())).TupleSqrt());
      if (0 != (HTuple(HTuple(int(hv_Sigma1!=0)).TupleAnd(int(hv_Sigma2!=0))).TupleAnd(int(hv_RenderCutoff!=0))))
      {
        //Compute the radius of an unnormalized normal distribution,
        //where a certain threshold value is reached at the end.
        hv_ExtendedLength1 = hv_Sigma1*((-2*(hv_RenderCutoff.TupleLog())).TupleSqrt());
        hv_ExtendedLength2 = hv_Sigma2*((-2*(hv_RenderCutoff.TupleLog())).TupleSqrt());
        GenRectangle2(&ho_ExtendedRectangle, HTuple((hv_DLSample.TupleGetDictTuple("bbox_row"))[hv_Index]), 
            HTuple((hv_DLSample.TupleGetDictTuple("bbox_col"))[hv_Index]), HTuple((hv_DLSample.TupleGetDictTuple("bbox_phi"))[hv_Index]), 
            hv_ExtendedLength1, hv_ExtendedLength2);
        ClipRegion(ho_ExtendedRectangle, &ho_ExtendedRectangle, 0, 0, hv_ImageHeight-1, 
            hv_ImageWidth-1);
        GetRegionPoints(ho_ExtendedRectangle, &hv_Rows, &hv_Columns);
        //Verify that the bounding box has an area to plot a gaussian
        AreaCenter(ho_ExtendedRectangle, &hv_Area, &hv_Row, &hv_Column);
        if (0 != (int(hv_Area>1)))
        {
          HomMat2dIdentity(&hv_HomMat2D);
          HomMat2dTranslate(hv_HomMat2D, -HTuple((hv_DLSample.TupleGetDictTuple("bbox_row"))[hv_Index]), 
              -HTuple((hv_DLSample.TupleGetDictTuple("bbox_col"))[hv_Index]), &hv_HomMat2D);
          HomMat2dRotate(hv_HomMat2D, -HTuple((hv_DLSample.TupleGetDictTuple("bbox_phi"))[hv_Index]), 
              0, 0, &hv_HomMat2D);
          AffineTransPoint2d(hv_HomMat2D, hv_Rows, hv_Columns, &hv_DistRow, &hv_DistCol);
          hv_ScaledGaussian = (-0.5*(((hv_DistCol*hv_DistCol)/(hv_Sigma1*hv_Sigma1))+((hv_DistRow*hv_DistRow)/(hv_Sigma2*hv_Sigma2)))).TupleExp();
          GetGrayval((*ho_TargetText), hv_Rows, hv_Columns, &hv_Grayval);
          SetGrayval((*ho_TargetText), hv_Rows, hv_Columns, hv_ScaledGaussian.TupleMax2(hv_Grayval));
        }
      }
    }
  }
  }
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Preprocess dl samples and generate targets and weights for ocr detection training. 
void gen_dl_ocr_detection_targets (HTuple hv_DLSampleOriginal, HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_TargetText, ho_TargetLink, ho_TargetOrientation;
  HObject  ho_TargetWeightText, ho_TargetWeightLink, ho_WeightedCharScore;
  HObject  ho_TargetWeightOrientation, ho_OriginalDomain, ho_Image;
  HObject  ho_DomainWeight, ho_Domain, ho_TargetOrientationOut;
  HObject  ho_TargetWeightOrientationOut, ho_TargetOrientationChannel;
  HObject  ho_TargetWeightOrientationChannel;

  // Local control variables
  HTuple  hv_ImageWidth, hv_ImageHeight, hv_Stride;
  HTuple  hv_ScaleHeight, hv_ScaleWidth, hv_BoxCutoff, hv_RenderCutoff;
  HTuple  hv_Alpha, hv_WSWeightRenderThreshold, hv_LinkZeroWeightRadius;
  HTuple  hv_Confidence, hv_ScoreMapsWidth, hv_ScoreMapsHeight;
  HTuple  hv_DLSample, hv_HomMat2DIdentity, hv_HomMat2DScale;
  HTuple  hv_DLSampleTargets, hv_OriginalDomainArea, hv__;
  HTuple  hv_OriginalWidth, hv_OriginalHeight, hv_IsOriginalDomainFull;
  HTuple  hv_ChannelIdx, hv___Tmp_Ctrl_0, hv___Tmp_Ctrl_1;
  HTupleVector  hvec_WordsCharsMapping(1);

  check_dl_preprocess_param(hv_DLPreprocessParam);
  GetDictTuple(hv_DLPreprocessParam, "image_width", &hv_ImageWidth);
  GetDictTuple(hv_DLPreprocessParam, "image_height", &hv_ImageHeight);
  hv_Stride = 2;
  //Parameters used in the fallback weak supervision case.
  //They make the the uniformly sized char boxes a bit smaller, as we can expect a spacing between the characters.
  hv_ScaleHeight = 0.9;
  hv_ScaleWidth = 0.8;
  //Parameters relevant to plot the gaussian blobs in the score map.
  hv_BoxCutoff = 0.3;
  hv_RenderCutoff = 0.01;
  //Parameter used to determine the dilation of lines in link map.
  hv_Alpha = 0.1;
  //Parameter used to determine the dilation radius of word boxes in the weight score map.
  hv_WSWeightRenderThreshold = 0.05;
  //Parameter represents the dilation radius of word lines in the weight link map.
  hv_LinkZeroWeightRadius = 2.5;
  //Confidence is here only a place holder for the fallback weak supervision case.
  hv_Confidence = 1.0;
  if (0 != (int(hv_Stride==0)))
  {
    throw HException("Stride must be greater than 0.");
  }
  //Calculate the size of score maps.
  hv_ScoreMapsWidth = hv_ImageWidth/hv_Stride;
  hv_ScoreMapsHeight = hv_ImageHeight/hv_Stride;
  //Copy DLSample to maintain the original bounding boxes dimensions.
  CopyDict(hv_DLSampleOriginal, HTuple(), HTuple(), &hv_DLSample);
  //Preprocess bounding boxes to match targets dimensions.
  HomMat2dIdentity(&hv_HomMat2DIdentity);
  HomMat2dScale(hv_HomMat2DIdentity, 1.0/hv_Stride, 1.0/hv_Stride, 0, 0, &hv_HomMat2DScale);
  AffineTransPoint2d(hv_HomMat2DScale, hv_DLSample.TupleGetDictTuple("bbox_col"), 
      hv_DLSample.TupleGetDictTuple("bbox_row"), &hv___Tmp_Ctrl_0, &hv___Tmp_Ctrl_1);
  SetDictTuple(hv_DLSample, "bbox_row", hv___Tmp_Ctrl_1);
  SetDictTuple(hv_DLSample, "bbox_col", hv___Tmp_Ctrl_0);
  SetDictTuple(hv_DLSample, "bbox_length1", (hv_DLSample.TupleGetDictTuple("bbox_length1"))/hv_Stride);
  SetDictTuple(hv_DLSample, "bbox_length2", (hv_DLSample.TupleGetDictTuple("bbox_length2"))/hv_Stride);
  CopyDict(hv_DLSample, HTuple(), HTuple(), &hv_DLSampleTargets);
  gen_dl_ocr_detection_gt_chars(hv_DLSampleTargets, hv_DLSample, hv_ScaleWidth, hv_ScaleHeight, 
      &hvec_WordsCharsMapping);
  //Generate target maps from WordRegions and CharBoxes.
  gen_dl_ocr_detection_gt_score_map(&ho_TargetText, hv_DLSampleTargets, hv_BoxCutoff, 
      hv_RenderCutoff, hv_ScoreMapsWidth, hv_ScoreMapsHeight);
  gen_dl_ocr_detection_gt_link_map(&ho_TargetLink, hv_ScoreMapsWidth, hv_ScoreMapsHeight, 
      hv_DLSampleTargets, hvec_WordsCharsMapping, hv_Alpha);
  gen_dl_ocr_detection_gt_orientation_map(&ho_TargetOrientation, hv_ScoreMapsWidth, 
      hv_ScoreMapsHeight, hv_DLSampleTargets);
  //Generate weight maps from WordRegions and CharBoxes.
  gen_dl_ocr_detection_weight_score_map(&ho_TargetWeightText, hv_ScoreMapsWidth, 
      hv_ScoreMapsHeight, hv_DLSampleTargets, hv_BoxCutoff, hv_WSWeightRenderThreshold, 
      hv_Confidence);
  gen_dl_ocr_detection_weight_link_map(ho_TargetLink, ho_TargetWeightText, &ho_TargetWeightLink, 
      hv_LinkZeroWeightRadius);
  MultImage(ho_TargetText, ho_TargetWeightText, &ho_WeightedCharScore, 1, 0);
  gen_dl_ocr_detection_weight_orientation_map(ho_WeightedCharScore, &ho_TargetWeightOrientation, 
      hv_DLSampleTargets);
  //Take account of the image domain in DLSampleOriginal.
  GetDomain(hv_DLSampleOriginal.TupleGetDictObject("image"), &ho_OriginalDomain);
  AreaCenter(ho_OriginalDomain, &hv_OriginalDomainArea, &hv__, &hv__);
  GetImageSize(hv_DLSampleOriginal.TupleGetDictObject("image"), &hv_OriginalWidth, 
      &hv_OriginalHeight);
  hv_IsOriginalDomainFull = int(hv_OriginalDomainArea==(hv_OriginalWidth*hv_OriginalHeight));
  if (0 != (hv_IsOriginalDomainFull.TupleNot()))
  {
    //Calculate the domain weight.
    GenImageConst(&ho_Image, "real", hv_OriginalWidth, hv_OriginalHeight);
    ChangeDomain(ho_Image, ho_OriginalDomain, &ho_Image);
    ZoomImageSize(ho_Image, &ho_DomainWeight, hv_ScoreMapsWidth, hv_ScoreMapsHeight, 
        "constant");
    GetDomain(ho_DomainWeight, &ho_Domain);
    FullDomain(ho_DomainWeight, &ho_DomainWeight);
    OverpaintRegion(ho_DomainWeight, ho_DomainWeight, 0.0, "fill");
    OverpaintRegion(ho_DomainWeight, ho_Domain, 1.0, "fill");
    //Apply the domain weight.
    MultImage(ho_DomainWeight, ho_TargetText, &ho_TargetText, 1, 0);
    MultImage(ho_DomainWeight, ho_TargetLink, &ho_TargetLink, 1, 0);
    MultImage(ho_DomainWeight, ho_TargetWeightText, &ho_TargetWeightText, 1, 0);
    MultImage(ho_DomainWeight, ho_TargetWeightLink, &ho_TargetWeightLink, 1, 0);
    GenEmptyObj(&ho_TargetOrientationOut);
    GenEmptyObj(&ho_TargetWeightOrientationOut);
    for (hv_ChannelIdx=1; hv_ChannelIdx<=2; hv_ChannelIdx+=1)
    {
      AccessChannel(ho_TargetOrientation, &ho_TargetOrientationChannel, hv_ChannelIdx);
      AccessChannel(ho_TargetWeightOrientation, &ho_TargetWeightOrientationChannel, 
          hv_ChannelIdx);
      MultImage(ho_DomainWeight, ho_TargetOrientationChannel, &ho_TargetOrientationChannel, 
          1, 0);
      MultImage(ho_DomainWeight, ho_TargetWeightOrientationChannel, &ho_TargetWeightOrientationChannel, 
          1, 0);
      AppendChannel(ho_TargetOrientationOut, ho_TargetOrientationChannel, &ho_TargetOrientationOut
          );
      AppendChannel(ho_TargetWeightOrientationOut, ho_TargetWeightOrientationChannel, 
          &ho_TargetWeightOrientationOut);
    }
    ho_TargetOrientation = ho_TargetOrientationOut;
    ho_TargetWeightOrientation = ho_TargetWeightOrientationOut;
  }
  //Set targets in output sample.
  SetDictObject(ho_TargetText, hv_DLSampleOriginal, "target_text");
  SetDictObject(ho_TargetLink, hv_DLSampleOriginal, "target_link");
  SetDictObject(ho_TargetOrientation, hv_DLSampleOriginal, "target_orientation");
  SetDictObject(ho_TargetWeightText, hv_DLSampleOriginal, "target_weight_text");
  SetDictObject(ho_TargetWeightLink, hv_DLSampleOriginal, "target_weight_link");
  SetDictObject(ho_TargetWeightOrientation, hv_DLSampleOriginal, "target_weight_orientation");
}

// Chapter: OCR / Deep OCR
// Short Description: Generate link score map weight for ocr detection training. 
void gen_dl_ocr_detection_weight_link_map (HObject ho_LinkMap, HObject ho_TargetWeight, 
    HObject *ho_TargetWeightLink, HTuple hv_LinkZeroWeightRadius)
{

  // Local iconic variables
  HObject  ho_LinkRegion, ho_RegionDilation, ho_RegionComplement;
  HObject  ho_RegionUnion, ho_RegionBorder;

  // Local control variables
  HTuple  hv_Width, hv_Height;

  if (0 != (int(hv_LinkZeroWeightRadius>0)))
  {
    //Set zero weight around the link regions.
    Threshold(ho_LinkMap, &ho_LinkRegion, 0.01, "max");
    DilationCircle(ho_LinkRegion, &ho_RegionDilation, hv_LinkZeroWeightRadius);
    Complement(ho_RegionDilation, &ho_RegionComplement);
    GetImageSize(ho_TargetWeight, &hv_Width, &hv_Height);
    ClipRegion(ho_RegionComplement, &ho_RegionComplement, 0, 0, hv_Height-1, hv_Width-1);
    Union2(ho_LinkRegion, ho_RegionComplement, &ho_RegionUnion);
    Complement(ho_RegionUnion, &ho_RegionBorder);
    PaintRegion(ho_RegionBorder, ho_TargetWeight, &(*ho_TargetWeightLink), 0, "fill");
  }
  else
  {
    //Just copy the original weight map.
    CopyObj(ho_TargetWeight, &(*ho_TargetWeightLink), 1, 1);
  }
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Generate orientation score map weight for ocr detection training. 
void gen_dl_ocr_detection_weight_orientation_map (HObject ho_InitialWeight, HObject *ho_OrientationTargetWeight, 
    HTuple hv_DLSample)
{

  // Local iconic variables
  HObject  ho_CharRegions, ho_CharRegion, ho_BackgroundRegion;

  // Local control variables
  HTuple  hv_Indices;

  //Inside the valid regions, the inital weight is set to the initial weight.
  CopyImage(ho_InitialWeight, &(*ho_OrientationTargetWeight));
  FullDomain((*ho_OrientationTargetWeight), &(*ho_OrientationTargetWeight));
  //Set orientation weight to 0 outside the valid regions.
  if (0 != (int(((hv_DLSample.TupleGetDictTuple("bbox_label_id")).TupleLength())>0)))
  {
    //Process char boxes
    TupleFind(hv_DLSample.TupleGetDictTuple("bbox_label_id"), 1, &hv_Indices);
    if (0 != (int(hv_Indices!=-1)))
    {
      GenRectangle2(&ho_CharRegions, HTuple((hv_DLSample.TupleGetDictTuple("bbox_row"))[hv_Indices]), 
          HTuple((hv_DLSample.TupleGetDictTuple("bbox_col"))[hv_Indices]), HTuple((hv_DLSample.TupleGetDictTuple("bbox_phi"))[hv_Indices]), 
          HTuple((hv_DLSample.TupleGetDictTuple("bbox_length1"))[hv_Indices]), HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_Indices]));
      Union1(ho_CharRegions, &ho_CharRegion);
      Complement(ho_CharRegion, &ho_BackgroundRegion);
      OverpaintRegion((*ho_OrientationTargetWeight), ho_BackgroundRegion, 0, "fill");
    }
  }
  //We need two channels: for Sin and Cos
  Compose2((*ho_OrientationTargetWeight), (*ho_OrientationTargetWeight), &(*ho_OrientationTargetWeight)
      );
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Generate text score map weight for ocr detection training. 
void gen_dl_ocr_detection_weight_score_map (HObject *ho_TargetWeightText, HTuple hv_ImageWidth, 
    HTuple hv_ImageHeight, HTuple hv_DLSample, HTuple hv_BoxCutoff, HTuple hv_WSWeightRenderThreshold, 
    HTuple hv_Confidence)
{

  // Local iconic variables
  HObject  ho_IgnoreRegion, ho_WordRegion, ho_WordRegionDilated;

  // Local control variables
  HTuple  hv_Indices, hv_WordIndex, hv_SigmaL2;
  HTuple  hv_WordLength2Ext, hv_DilationRadius;

  GenImageConst(&(*ho_TargetWeightText), "real", hv_ImageWidth, hv_ImageHeight);
  OverpaintRegion((*ho_TargetWeightText), (*ho_TargetWeightText), 1.0, "fill");
  if (0 != (int(((hv_DLSample.TupleGetDictTuple("bbox_label_id")).TupleLength())>0)))
  {
    //Process ignore boxes
    TupleFind(hv_DLSample.TupleGetDictTuple("bbox_label_id"), 2, &hv_Indices);
    if (0 != (int(hv_Indices!=-1)))
    {
      GenRectangle2(&ho_IgnoreRegion, HTuple((hv_DLSample.TupleGetDictTuple("bbox_row"))[hv_Indices]), 
          HTuple((hv_DLSample.TupleGetDictTuple("bbox_col"))[hv_Indices]), HTuple((hv_DLSample.TupleGetDictTuple("bbox_phi"))[hv_Indices]), 
          HTuple((hv_DLSample.TupleGetDictTuple("bbox_length1"))[hv_Indices]), HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_Indices]));
      OverpaintRegion((*ho_TargetWeightText), ho_IgnoreRegion, 0.0, "fill");
    }
    {
    HTuple end_val9 = ((hv_DLSample.TupleGetDictTuple("bbox_label_id")).TupleLength())-1;
    HTuple step_val9 = 1;
    for (hv_WordIndex=0; hv_WordIndex.Continue(end_val9, step_val9); hv_WordIndex += step_val9)
    {
      //For each word box
      if (0 != (int(HTuple((hv_DLSample.TupleGetDictTuple("bbox_label_id"))[hv_WordIndex])==0)))
      {
        if (0 != (HTuple(HTuple(int(hv_BoxCutoff==0)).TupleOr(int(hv_WSWeightRenderThreshold==0))).TupleNot()))
        {
          hv_SigmaL2 = HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_WordIndex])*((-0.5/(hv_BoxCutoff.TupleLog())).TupleSqrt());
          hv_WordLength2Ext = hv_SigmaL2*((-2*(hv_WSWeightRenderThreshold.TupleLog())).TupleSqrt());
          hv_DilationRadius = hv_WordLength2Ext-HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_WordIndex]);
        }
        else
        {
          hv_DilationRadius = 0;
        }
        GenRectangle2(&ho_WordRegion, HTuple((hv_DLSample.TupleGetDictTuple("bbox_row"))[hv_WordIndex]), 
            HTuple((hv_DLSample.TupleGetDictTuple("bbox_col"))[hv_WordIndex]), HTuple((hv_DLSample.TupleGetDictTuple("bbox_phi"))[hv_WordIndex]), 
            HTuple((hv_DLSample.TupleGetDictTuple("bbox_length1"))[hv_WordIndex]), 
            HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_WordIndex]));
        //Slightly enlarge the weight region to suppress halos at the box borders.
        if (0 != (int(hv_DilationRadius>=0.5)))
        {
          DilationCircle(ho_WordRegion, &ho_WordRegionDilated, hv_DilationRadius);
        }
        else
        {
          ho_WordRegionDilated = ho_WordRegion;
        }
        //Set the confidence as weight for the word region.
        OverpaintRegion((*ho_TargetWeightText), ho_WordRegionDilated, hv_Confidence, 
            "fill");
      }
    }
    }
  }
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Store the given images in a tuple of dictionaries DLSamples. 
void gen_dl_samples_from_images (HObject ho_Images, HTuple *hv_DLSampleBatch)
{

  // Local iconic variables
  HObject  ho_Image;

  // Local control variables
  HTuple  hv_NumImages, hv_ImageIndex, hv_DLSample;

  //
  //This procedure creates DLSampleBatch, a tuple
  //containing a dictionary DLSample
  //for every image given in Images.
  //
  //Initialize output tuple.
  CountObj(ho_Images, &hv_NumImages);
  (*hv_DLSampleBatch) = HTuple(hv_NumImages,-1);
  //
  //Loop through all given images.
  {
  HTuple end_val10 = hv_NumImages-1;
  HTuple step_val10 = 1;
  for (hv_ImageIndex=0; hv_ImageIndex.Continue(end_val10, step_val10); hv_ImageIndex += step_val10)
  {
    SelectObj(ho_Images, &ho_Image, hv_ImageIndex+1);
    //Create DLSample from image.
    CreateDict(&hv_DLSample);
    SetDictObject(ho_Image, hv_DLSample, "image");
    //
    //Collect the DLSamples.
    (*hv_DLSampleBatch)[hv_ImageIndex] = hv_DLSample;
  }
  }
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Generate a word to characters mapping. 
void gen_words_chars_mapping (HTuple hv_DLSample, HTupleVector/*{eTupleVector,Dim=1}*/ *hvec_WordsCharsMapping)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_WordsIndices, hv_CharsIndices, hv_WordLengths;
  HTuple  hv_WordArea, hv_CharArea, hv_CharAreaThreshold;
  HTuple  hv_WordIndex, hv_AreaIntersection, hv_CIsInsideW;
  HTuple  hv_CIndex;

  //Procedure to generate the mapping: gen_words_chars_mapping
  if (0 != (int(((hv_DLSample.TupleGetDictTuple("bbox_label_id")).TupleLength())>0)))
  {
    TupleFind(hv_DLSample.TupleGetDictTuple("bbox_label_id"), 0, &hv_WordsIndices);
    TupleFind(hv_DLSample.TupleGetDictTuple("bbox_label_id"), 1, &hv_CharsIndices);
    if (0 != (HTuple(int(hv_CharsIndices!=-1)).TupleAnd(int(hv_WordsIndices!=-1))))
    {
      hv_WordLengths = HTuple((hv_DLSample.TupleGetDictTuple("word"))[hv_WordsIndices]).TupleStrlen();
      //Init vector.
      (*hvec_WordsCharsMapping)[((hv_DLSample.TupleGetDictTuple("bbox_label_id")).TupleLength())-1] = HTupleVector(HTuple());
      hv_WordArea = (4*HTuple((hv_DLSample.TupleGetDictTuple("bbox_length1"))[hv_WordsIndices]))*HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_WordsIndices]);
      hv_CharArea = (4*HTuple((hv_DLSample.TupleGetDictTuple("bbox_length1"))[hv_CharsIndices]))*HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_CharsIndices]);
      //TODO: This threshold is quite arbitrary and not stable.
      hv_CharAreaThreshold = hv_CharArea*0.8;
      {
      HTuple end_val12 = (hv_WordsIndices.TupleLength())-1;
      HTuple step_val12 = 1;
      for (hv_WordIndex=0; hv_WordIndex.Continue(end_val12, step_val12); hv_WordIndex += step_val12)
      {
        if (0 != (int(HTuple(hv_WordLengths[hv_WordIndex])!=0)))
        {
          AreaIntersectionRectangle2(HTuple((hv_DLSample.TupleGetDictTuple("bbox_row"))[HTuple(hv_WordsIndices[hv_WordIndex])]), 
              HTuple((hv_DLSample.TupleGetDictTuple("bbox_col"))[HTuple(hv_WordsIndices[hv_WordIndex])]), 
              HTuple((hv_DLSample.TupleGetDictTuple("bbox_phi"))[HTuple(hv_WordsIndices[hv_WordIndex])]), 
              HTuple((hv_DLSample.TupleGetDictTuple("bbox_length1"))[HTuple(hv_WordsIndices[hv_WordIndex])]), 
              HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[HTuple(hv_WordsIndices[hv_WordIndex])]), 
              HTuple((hv_DLSample.TupleGetDictTuple("bbox_row"))[hv_CharsIndices]), 
              HTuple((hv_DLSample.TupleGetDictTuple("bbox_col"))[hv_CharsIndices]), 
              HTuple((hv_DLSample.TupleGetDictTuple("bbox_phi"))[hv_CharsIndices]), 
              HTuple((hv_DLSample.TupleGetDictTuple("bbox_length1"))[hv_CharsIndices]), 
              HTuple((hv_DLSample.TupleGetDictTuple("bbox_length2"))[hv_CharsIndices]), 
              &hv_AreaIntersection);
          hv_CIsInsideW = hv_AreaIntersection.TupleGreaterElem(hv_CharAreaThreshold);
          hv_CIndex = hv_CIsInsideW.TupleFind(1);
          if (0 != (int(hv_CIndex!=-1)))
          {
            (*hvec_WordsCharsMapping)[HTuple(hv_WordsIndices[hv_WordIndex])] = HTupleVector(HTuple(hv_CharsIndices[hv_CIndex]));
          }
        }
        else
        {
          throw HException(((("Sample with image id "+(hv_DLSample.TupleGetDictTuple("image_id")))+" is not valid. The word bounding box at index ")+hv_WordIndex)+" has an empty string as the ground truth. This is not allowed. Please assign a word label to every word bounding box.");
        }
      }
      }
    }
  }
  return;
}

// Chapter: Deep Learning / Anomaly Detection and Global Context Anomaly Detection
// Short Description: Get the ground truth anomaly label and label ID. 
void get_anomaly_ground_truth_label (HTuple hv_SampleKeys, HTuple hv_DLSample, HTuple *hv_AnomalyLabelGroundTruth, 
    HTuple *hv_AnomalyLabelIDGroundTruth)
{

  //
  //This procedure returns the anomaly ground truth label.
  //
  if (0 != (int((hv_SampleKeys.TupleFind("anomaly_label"))!=-1)))
  {
    GetDictTuple(hv_DLSample, "anomaly_label", &(*hv_AnomalyLabelGroundTruth));
  }
  else
  {
    throw HException("Ground truth class label cannot be found in DLSample.");
  }
  if (0 != (int((hv_SampleKeys.TupleFind("anomaly_label_id"))!=-1)))
  {
    GetDictTuple(hv_DLSample, "anomaly_label_id", &(*hv_AnomalyLabelIDGroundTruth));
  }
  else
  {
    throw HException("Ground truth class label id cannot be found in DLSample.");
  }
  //
  return;
}

// Chapter: Deep Learning / Anomaly Detection and Global Context Anomaly Detection
// Short Description: Get the anomaly results out of DLResult and apply thresholds (if specified). 
void get_anomaly_result (HObject *ho_AnomalyImage, HObject *ho_AnomalyRegion, HTuple hv_DLResult, 
    HTuple hv_AnomalyClassThreshold, HTuple hv_AnomalyRegionThreshold, HTuple hv_AnomalyResultPostfix, 
    HTuple *hv_AnomalyScore, HTuple *hv_AnomalyClassID, HTuple *hv_AnomalyClassThresholdDisplay, 
    HTuple *hv_AnomalyRegionThresholdDisplay)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_AnomalyImageKey, hv_AnomalyScoreKey;
  HTuple  hv_AnomalyRegionKey, hv_AnomalyClassIdKey, hv_ErrorMsgPostfix;
  HTuple  hv_ResultKeys;

  //
  //This procedure returns the inference results in DLResult which are
  //the anomaly image and the anomaly score. It also returns the
  //classification of the anomaly score and segmentation of anomalous
  //pixels in the anomaly image by applying the specified thresholds if
  //given. Otherwise the results from DLResult are used.
  //
  //
  hv_AnomalyImageKey = "anomaly_image"+hv_AnomalyResultPostfix;
  hv_AnomalyScoreKey = "anomaly_score"+hv_AnomalyResultPostfix;
  hv_AnomalyRegionKey = "anomaly_region"+hv_AnomalyResultPostfix;
  hv_AnomalyClassIdKey = "anomaly_class_id"+hv_AnomalyResultPostfix;
  //
  TupleRegexpReplace(hv_AnomalyResultPostfix, "_", " ", &hv_ErrorMsgPostfix);
  //
  GetDictParam(hv_DLResult, "keys", HTuple(), &hv_ResultKeys);
  if (0 != (int(hv_ResultKeys==HTuple())))
  {
    throw HException(("Result anomaly image"+hv_ErrorMsgPostfix)+" could not be found in DLResult.");
  }
  //
  if (0 != (int((hv_ResultKeys.TupleFindFirst(hv_AnomalyImageKey))!=-1)))
  {
    GetDictObject(&(*ho_AnomalyImage), hv_DLResult, hv_AnomalyImageKey);
  }
  else
  {
    throw HException(("Result anomaly image"+hv_ErrorMsgPostfix)+" could not be found in DLResult.");
  }
  //
  if (0 != (int((hv_ResultKeys.TupleFindFirst(hv_AnomalyScoreKey))!=-1)))
  {
    GetDictTuple(hv_DLResult, hv_AnomalyScoreKey, &(*hv_AnomalyScore));
  }
  else
  {
    throw HException(("Result anomaly score"+hv_ErrorMsgPostfix)+" could not be found in DLResult.");
  }
  //
  (*hv_AnomalyRegionThresholdDisplay) = -1;
  if (0 != (int(hv_AnomalyRegionThreshold!=-1)))
  {
    //Apply threshold for segmentation result.
    if (0 != (int((hv_AnomalyRegionThreshold.TupleLength())!=1)))
    {
      throw HException("Selected 'anomaly_region_threshold' must be specified by exactly one value.");
    }
    Threshold((*ho_AnomalyImage), &(*ho_AnomalyRegion), hv_AnomalyRegionThreshold, 
        "max");
    (*hv_AnomalyRegionThresholdDisplay) = hv_AnomalyRegionThreshold;
  }
  else
  {
    //If no threshold is given, use the threshold and resulting anomaly region out of DLResult.
    if (0 != (int((hv_ResultKeys.TupleFindFirst(hv_AnomalyRegionKey))!=-1)))
    {
      GetDictObject(&(*ho_AnomalyRegion), hv_DLResult, hv_AnomalyRegionKey);
    }
    else
    {
      GenEmptyObj(&(*ho_AnomalyRegion));
    }
    if (0 != (int((hv_ResultKeys.TupleFind("anomaly_segmentation_threshold"))!=-1)))
    {
      GetDictTuple(hv_DLResult, "anomaly_segmentation_threshold", &(*hv_AnomalyRegionThresholdDisplay));
    }
  }
  //
  (*hv_AnomalyClassThresholdDisplay) = -1;
  (*hv_AnomalyClassID) = -1;
  if (0 != (int(hv_AnomalyClassThreshold!=-1)))
  {
    //Apply threshold for classification result.
    if (0 != (int((hv_AnomalyClassThreshold.TupleLength())!=1)))
    {
      throw HException("Selected 'anomaly_classification_threshold' must be specified by exactly one value.");
    }
    if (0 != (int((*hv_AnomalyScore)<hv_AnomalyClassThreshold)))
    {
      (*hv_AnomalyClassID) = 0;
    }
    else
    {
      (*hv_AnomalyClassID) = 1;
    }
    (*hv_AnomalyClassThresholdDisplay) = hv_AnomalyClassThreshold;
  }
  else
  {
    //If no threshold is given, use the threshold and resulting class id out of DLResult.
    if (0 != (int((hv_ResultKeys.TupleFindFirst(hv_AnomalyClassIdKey))!=-1)))
    {
      GetDictTuple(hv_DLResult, hv_AnomalyClassIdKey, &(*hv_AnomalyClassID));
    }
    else
    {
      (*hv_AnomalyClassID) = -1;
    }
    if (0 != (int((hv_ResultKeys.TupleFind("anomaly_classification_threshold"))!=-1)))
    {
      GetDictTuple(hv_DLResult, "anomaly_classification_threshold", &(*hv_AnomalyClassThresholdDisplay));
    }
  }
  //
  return;
}

// Chapter: Graphics / Window
// Short Description: Get the next child window that can be used for visualization. 
void get_child_window (HTuple hv_HeightImage, HTuple hv_Font, HTuple hv_FontSize, 
    HTuple hv_Text, HTuple hv_PrevWindowCoordinates, HTuple hv_WindowHandleDict, 
    HTuple hv_WindowHandleKey, HTuple *hv_WindowImageRatio, HTuple *hv_PrevWindowCoordinatesOut)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_OpenNewWindow, hv_WindowHandles, hv_ParentWindowHandle;
  HTuple  hv_ChildWindowHandle, hv_Exception, hv_MetaInfo;
  HTuple  hv_WindowRow, hv_WindowColumn, hv_WindowWidth, hv_WindowHeight;

  //
  //This procedure returns the next child window that
  //is used for visualization. If ReuseWindows is true
  //and WindowHandleList is suitable, the window handles
  //that are passed over are used. Else, this procedure
  //opens a new window, either next to the last ones, or
  //in a new row.
  //
  //First, check if the requested window is already available.
  hv_OpenNewWindow = 0;
  try
  {
    GetDictTuple(hv_WindowHandleDict, hv_WindowHandleKey, &hv_WindowHandles);
    hv_ParentWindowHandle = ((const HTuple&)hv_WindowHandles)[0];
    hv_ChildWindowHandle = ((const HTuple&)hv_WindowHandles)[1];
    //Check if window handle is valid.
    try
    {
      FlushBuffer(hv_ChildWindowHandle);
    }
    // catch (Exception) 
    catch (HException &HDevExpDefaultException)
    {
      HDevExpDefaultException.ToHTuple(&hv_Exception);
      //Since there is something wrong with the current window, create a new one.
      hv_OpenNewWindow = 1;
    }
  }
  // catch (Exception) 
  catch (HException &HDevExpDefaultException)
  {
    HDevExpDefaultException.ToHTuple(&hv_Exception);
    hv_OpenNewWindow = 1;
  }
  //
  //Get next child window.
  if (0 != (hv_OpenNewWindow.TupleNot()))
  {
    //
    //If possible, reuse existing window handles.
    HDevWindowStack::SetActive(hv_ChildWindowHandle);
    if (HDevWindowStack::IsOpen())
      ClearWindow(HDevWindowStack::GetActive());
    set_display_font(hv_ChildWindowHandle, hv_FontSize, hv_Font, "true", "false");
    //
    GetDictTuple(hv_WindowHandleDict, "meta_information", &hv_MetaInfo);
    //
    //Get previous window coordinates.
    GetWindowExtents(hv_ParentWindowHandle, &hv_WindowRow, &hv_WindowColumn, &hv_WindowWidth, 
        &hv_WindowHeight);
    (*hv_WindowImageRatio) = hv_WindowHeight/(hv_HeightImage*1.0);
    //
    try
    {
      //
      //Get WindowImageRatio from parent window.
      GetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_window_image_ratio_height", 
          &(*hv_WindowImageRatio));
      //
      //Get previous window coordinates.
      GetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_child_window_coordinates", &(*hv_PrevWindowCoordinatesOut));
    }
    // catch (Exception) 
    catch (HException &HDevExpDefaultException)
    {
      HDevExpDefaultException.ToHTuple(&hv_Exception);
      //
      //Set WindowImageRatio from parent window.
      GetWindowExtents(hv_ParentWindowHandle, &hv_WindowRow, &hv_WindowColumn, &hv_WindowWidth, 
          &hv_WindowHeight);
      (*hv_WindowImageRatio) = hv_WindowHeight/(hv_HeightImage*1.0);
      //
      //Set previous window coordinates.
      (*hv_PrevWindowCoordinatesOut)[0] = hv_WindowRow;
      (*hv_PrevWindowCoordinatesOut)[1] = hv_WindowColumn;
      (*hv_PrevWindowCoordinatesOut)[2] = hv_WindowWidth;
      (*hv_PrevWindowCoordinatesOut)[3] = hv_WindowHeight;
    }
  }
  else
  {
    //
    //Open a new child window.
    open_child_window(hv_ParentWindowHandle, hv_Font, hv_FontSize, hv_Text, hv_PrevWindowCoordinates, 
        hv_WindowHandleDict, hv_WindowHandleKey, &hv_ChildWindowHandle, &(*hv_PrevWindowCoordinatesOut));
    SetWindowParam(hv_ChildWindowHandle, "flush", "false");
    SetDictTuple(hv_WindowHandleDict, hv_WindowHandleKey, hv_ParentWindowHandle.TupleConcat(hv_ChildWindowHandle));
  }
  //
  return;
}

// Chapter: Deep Learning / Classification
// Short Description: Get the ground truth classification label id. 
void get_classification_ground_truth (HTuple hv_SampleKeys, HTuple hv_DLSample, HTuple *hv_ClassificationLabelIDGroundTruth)
{

  //
  //This procedure returns the classification ground truth label ID.
  //
  if (0 != (int((hv_SampleKeys.TupleFind("image_label_id"))!=-1)))
  {
    GetDictTuple(hv_DLSample, "image_label_id", &(*hv_ClassificationLabelIDGroundTruth));
  }
  else if (0 != (int((hv_SampleKeys.TupleFind("image_label_ids"))!=-1)))
  {
    GetDictTuple(hv_DLSample, "image_label_ids", &(*hv_ClassificationLabelIDGroundTruth));
  }
  else
  {
    throw HException("Ground truth class label cannot be found in DLSample.");
  }
  //
  return;
}

// Chapter: Deep Learning / Classification
// Short Description: Get the predicted classification class ID. 
void get_classification_result (HTuple hv_ResultKeys, HTuple hv_DLResult, HTuple *hv_ClassificationClassID)
{

  // Local iconic variables

  //
  //This procedure returns the predicted classification class ID.
  //
  if (0 != (int((hv_ResultKeys.TupleFind("classification_class_ids"))!=-1)))
  {
    GetDictTuple(hv_DLResult, "classification_class_ids", &(*hv_ClassificationClassID));
    if (0 != (int(((*hv_ClassificationClassID).TupleLength())>0)))
    {
      (*hv_ClassificationClassID) = ((const HTuple&)(*hv_ClassificationClassID))[0];
    }
  }
  else if (0 != (int((hv_ResultKeys.TupleFind("selected_class_ids"))!=-1)))
  {
    GetDictTuple(hv_DLResult, "selected_class_ids", &(*hv_ClassificationClassID));
  }
  else
  {
    throw HException("Key entry 'classification_class_ids' or 'selected_class_ids' could not be found in DLResult.");
  }
  //
  return;
}

// Chapter: Deep Learning / Semantic Segmentation and Edge Extraction
// Short Description: Get the confidences of the segmentation result. 
void get_confidence_image (HObject *ho_ImageConfidence, HTuple hv_ResultKeys, HTuple hv_DLResult)
{

  //
  //This procedure returns confidences of the segmentation result.
  //
  if (0 != (int((hv_ResultKeys.TupleFind("segmentation_confidence"))!=-1)))
  {
    GetDictObject(&(*ho_ImageConfidence), hv_DLResult, "segmentation_confidence");
  }
  else if (0 != (int((hv_ResultKeys.TupleFind("segmentation_confidences"))!=-1)))
  {
    GetDictObject(&(*ho_ImageConfidence), hv_DLResult, "segmentation_confidences");
  }
  else
  {
    throw HException("Confidence image could not be found in DLSample.");
  }
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Generate NumColors distinct colors 
void get_distinct_colors (HTuple hv_NumColors, HTuple hv_Random, HTuple hv_StartColor, 
    HTuple hv_EndColor, HTuple *hv_Colors)
{

  // Local iconic variables
  HObject  ho_HLSImageH, ho_HLSImageL, ho_HLSImageS;
  HObject  ho_ImageR, ho_ImageG, ho_ImageB;

  // Local control variables
  HTuple  hv_IsString, hv_Hue, hv_Lightness, hv_Saturation;
  HTuple  hv_Rows, hv_Columns, hv_Red, hv_Green, hv_Blue;

  //
  //We get distinct color-values first in HLS color-space.
  //Assumes hue [0, EndColor), lightness [0, 1), saturation [0, 1).
  //
  //Parameter checks.
  //NumColors.
  if (0 != (int(hv_NumColors<1)))
  {
    throw HException("NumColors should be at least 1");
  }
  if (0 != ((hv_NumColors.TupleIsInt()).TupleNot()))
  {
    throw HException("NumColors should be of type int");
  }
  if (0 != (int((hv_NumColors.TupleLength())!=1)))
  {
    throw HException("NumColors should have length 1");
  }
  //Random.
  if (0 != (HTuple(int(hv_Random!=0)).TupleAnd(int(hv_Random!=1))))
  {
    TupleIsString(hv_Random, &hv_IsString);
    if (0 != hv_IsString)
    {
      hv_Random = HTuple(int(hv_Random==HTuple("true"))).TupleOr("false");
    }
    else
    {
      throw HException("Random should be either true or false");
    }
  }
  //StartColor.
  if (0 != (int((hv_StartColor.TupleLength())!=1)))
  {
    throw HException("StartColor should have length 1");
  }
  if (0 != (HTuple(int(hv_StartColor<0)).TupleOr(int(hv_StartColor>255))))
  {
    throw HException(HTuple("StartColor should be in the range [0, 255]"));
  }
  if (0 != ((hv_StartColor.TupleIsInt()).TupleNot()))
  {
    throw HException("StartColor should be of type int");
  }
  //EndColor.
  if (0 != (int((hv_EndColor.TupleLength())!=1)))
  {
    throw HException("EndColor should have length 1");
  }
  if (0 != (HTuple(int(hv_EndColor<0)).TupleOr(int(hv_EndColor>255))))
  {
    throw HException(HTuple("EndColor should be in the range [0, 255]"));
  }
  if (0 != ((hv_EndColor.TupleIsInt()).TupleNot()))
  {
    throw HException("EndColor should be of type int");
  }
  //
  //Color generation.
  if (0 != (int(hv_StartColor>hv_EndColor)))
  {
    hv_EndColor += 255;
  }
  if (0 != (int(hv_NumColors!=1)))
  {
    hv_Hue = (hv_StartColor+((((hv_EndColor-hv_StartColor)*(HTuple::TupleGenSequence(0,hv_NumColors-1,1).TupleReal()))/((hv_NumColors-1).TupleReal())).TupleInt()))%255;
  }
  else
  {
    hv_Hue = (hv_StartColor.TupleConcat(hv_EndColor)).TupleMean();
  }
  if (0 != hv_Random)
  {
    hv_Hue = ((const HTuple&)hv_Hue)[HTuple::TupleRand(hv_NumColors).TupleSortIndex()];
    hv_Lightness = (((5.0+HTuple::TupleRand(hv_NumColors))*255.0)/10.0).TupleInt();
    hv_Saturation = (((9.0+HTuple::TupleRand(hv_NumColors))*255.0)/10.0).TupleInt();
  }
  else
  {
    hv_Lightness = (HTuple(hv_NumColors,0.55)*255.0).TupleInt();
    hv_Saturation = (HTuple(hv_NumColors,0.95)*255.0).TupleInt();
  }
  //
  //Write colors to a 3-channel image in order to transform easier.
  GenImageConst(&ho_HLSImageH, "byte", 1, hv_NumColors);
  GenImageConst(&ho_HLSImageL, "byte", 1, hv_NumColors);
  GenImageConst(&ho_HLSImageS, "byte", 1, hv_NumColors);
  GetRegionPoints(ho_HLSImageH, &hv_Rows, &hv_Columns);
  SetGrayval(ho_HLSImageH, hv_Rows, hv_Columns, hv_Hue);
  SetGrayval(ho_HLSImageL, hv_Rows, hv_Columns, hv_Lightness);
  SetGrayval(ho_HLSImageS, hv_Rows, hv_Columns, hv_Saturation);
  //
  //Convert from HLS to RGB.
  TransToRgb(ho_HLSImageH, ho_HLSImageL, ho_HLSImageS, &ho_ImageR, &ho_ImageG, &ho_ImageB, 
      "hls");
  //
  //Get RGB-values and transform to Hex.
  GetGrayval(ho_ImageR, hv_Rows, hv_Columns, &hv_Red);
  GetGrayval(ho_ImageG, hv_Rows, hv_Columns, &hv_Green);
  GetGrayval(ho_ImageB, hv_Rows, hv_Columns, &hv_Blue);
  (*hv_Colors) = (("#"+(hv_Red.TupleString("02x")))+(hv_Green.TupleString("02x")))+(hv_Blue.TupleString("02x"));
  return;
  //
}

// Chapter: Deep Learning / Model
// Short Description: Generate certain colors for different ClassNames 
void get_dl_class_colors (HTuple hv_ClassNames, HTuple hv_AdditionalGreenClassNames, 
    HTuple *hv_Colors)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_NumColors, hv_ColorsRainbow, hv_ClassNamesGood;
  HTuple  hv_IndexFind, hv_GoodIdx, hv_CurrentColor, hv_GreenIdx;

  //
  //This procedure returns for each class a certain color.
  //
  //Define distinct colors for the classes.
  hv_NumColors = hv_ClassNames.TupleLength();
  //Get distinct colors without randomness makes neighboring colors look very similar.
  //We use a workaround to get deterministic colors where subsequent colors are distinguishable.
  get_distinct_colors(hv_NumColors, 0, 0, 200, &hv_ColorsRainbow);
  TupleInverse(hv_ColorsRainbow, &hv_ColorsRainbow);
  make_neighboring_colors_distinguishable(hv_ColorsRainbow, &(*hv_Colors));
  //If a class 'OK','ok', 'good' or 'GOOD' or a class specified in AdditionalGreenClassNames is present set this class to green.
  //Only the first occurrence found is set to a green shade.
  TupleUnion((((HTuple("good").Append("GOOD")).Append("ok")).Append("OK")), hv_AdditionalGreenClassNames, 
      &hv_ClassNamesGood);
  {
  HTuple end_val13 = (hv_ClassNamesGood.TupleLength())-1;
  HTuple step_val13 = 1;
  for (hv_IndexFind=0; hv_IndexFind.Continue(end_val13, step_val13); hv_IndexFind += step_val13)
  {
    hv_GoodIdx = hv_ClassNames.TupleFindFirst(HTuple(hv_ClassNamesGood[hv_IndexFind]));
    if (0 != (HTuple(int(hv_GoodIdx!=-1)).TupleAnd(int((hv_ClassNames.TupleLength())<=8))))
    {
      //If number of classes is <= 8, swap color with a green color.
      hv_CurrentColor = HTuple((*hv_Colors)[hv_GoodIdx]);
      hv_GreenIdx = HTuple((hv_ClassNames.TupleLength())/2.0).TupleFloor();
      //Set to pure green.
      (*hv_Colors)[hv_GoodIdx] = "#00ff00";
      //Write original color to a green entry.
      (*hv_Colors)[hv_GreenIdx] = hv_CurrentColor;
      break;
    }
    else if (0 != (HTuple(int(hv_GoodIdx!=-1)).TupleAnd(int((hv_ClassNames.TupleLength())>8))))
    {
      //If number of classes is larger than 8, set the respective color to green.
      (*hv_Colors)[hv_GoodIdx] = "#00ff00";
      break;
    }
  }
  }
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Get an image of a sample with a certain key. 
void get_dl_sample_image (HObject *ho_Image, HTuple hv_SampleKeys, HTuple hv_DLSample, 
    HTuple hv_Key)
{

  //This procedure returns an image with key Key of a sample.
  //
  if (0 != (int((hv_SampleKeys.TupleFind(hv_Key))!=-1)))
  {
    GetDictObject(&(*ho_Image), hv_DLSample, hv_Key);
  }
  else
  {
    throw HException(("Image with key '"+hv_Key)+"' could not be found in DLSample.");
  }
  return;
}

// Chapter: 3D Matching / 3D Gripping Point Detection
// Short Description: Extract gripping points from a dictionary. 
void get_gripping_points_from_dict (HTuple hv_DLResult, HTuple *hv_Rows, HTuple *hv_Columns)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_KeyExists, hv_GrippingPoints, hv_NumberOfResults;
  HTuple  hv_IndexGrippingPointsPoint, hv_GrippingPointCoordDict;

  if (0 != (int(hv_DLResult==HTuple())))
  {
    throw HException("DLResult dict is empty.");
  }
  GetDictParam(hv_DLResult, "key_exists", "gripping_points", &hv_KeyExists);
  if (0 != hv_KeyExists)
  {
    GetDictTuple(hv_DLResult, "gripping_points", &hv_GrippingPoints);
  }
  else
  {
    throw HException("Result data could not be found in DLResult.");
  }
  hv_NumberOfResults = hv_GrippingPoints.TupleLength();
  TupleGenConst(hv_NumberOfResults, 0, &(*hv_Rows));
  TupleGenConst(hv_NumberOfResults, 0, &(*hv_Columns));
  {
  HTuple end_val12 = (hv_GrippingPoints.TupleLength())-1;
  HTuple step_val12 = 1;
  for (hv_IndexGrippingPointsPoint=0; hv_IndexGrippingPointsPoint.Continue(end_val12, step_val12); hv_IndexGrippingPointsPoint += step_val12)
  {
    hv_GrippingPointCoordDict = HTuple(hv_GrippingPoints[hv_IndexGrippingPointsPoint]);
    (*hv_Rows)[hv_IndexGrippingPointsPoint] = hv_GrippingPointCoordDict.TupleGetDictTuple("row");
    (*hv_Columns)[hv_IndexGrippingPointsPoint] = hv_GrippingPointCoordDict.TupleGetDictTuple("column");
  }
  }
  return;
}

// Chapter: Graphics / Window
// Short Description: Get the next window that can be used for visualization. 
void get_next_window (HTuple hv_Font, HTuple hv_FontSize, HTuple hv_ShowBottomDesc, 
    HTuple hv_WidthImage, HTuple hv_HeightImage, HTuple hv_MapColorBarWidth, HTuple hv_ScaleWindows, 
    HTuple hv_ThresholdWidth, HTuple hv_PrevWindowCoordinates, HTuple hv_WindowHandleDict, 
    HTuple hv_WindowHandleKey, HTuple *hv_CurrentWindowHandle, HTuple *hv_WindowImageRatioHeight, 
    HTuple *hv_PrevWindowCoordinatesOut)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_OpenNewWindow, hv_WindowHandles, hv_Value;
  HTuple  hv_Exception, hv_Ascent, hv_Descent, hv__, hv_NumLines;
  HTuple  hv_MarginBottom, hv_WindowImageRatioWidth, hv_SetPartRow2;
  HTuple  hv_SetPartColumn2, hv_MetaInfo;

  //
  //This procedure returns the next window that
  //is used for visualization. If ReuseWindows is true
  //and WindowHandleList is suitable, the window handles
  //that are passed over are used. Else, this procedure
  //opens a new window, either next to the last ones, or
  //in a new row.
  //
  //First, check if the requested window is already available.
  hv_OpenNewWindow = 0;
  try
  {
    GetDictTuple(hv_WindowHandleDict, hv_WindowHandleKey, &hv_WindowHandles);
    (*hv_CurrentWindowHandle) = ((const HTuple&)hv_WindowHandles)[0];
    //Check if window handle is valid.
    try
    {
      GetWindowParam((*hv_CurrentWindowHandle), "flush", &hv_Value);
    }
    // catch (Exception) 
    catch (HException &HDevExpDefaultException)
    {
      HDevExpDefaultException.ToHTuple(&hv_Exception);
      //If there is something wrong with the current window, create a new one.
      hv_OpenNewWindow = 1;
      RemoveDictKey(hv_WindowHandleDict, hv_WindowHandleKey);
    }
  }
  // catch (Exception) 
  catch (HException &HDevExpDefaultException)
  {
    HDevExpDefaultException.ToHTuple(&hv_Exception);
    hv_OpenNewWindow = 1;
  }
  //
  //Get next window.
  if (0 != (hv_OpenNewWindow.TupleNot()))
  {
    //
    //If possible, reuse existing window handles.
    HDevWindowStack::SetActive((*hv_CurrentWindowHandle));
    if (HDevWindowStack::IsOpen())
      ClearWindow(HDevWindowStack::GetActive());
    set_display_font((*hv_CurrentWindowHandle), hv_FontSize, hv_Font, "true", "false");
    //
    //Calculate MarginBottom.
    if (0 != hv_ShowBottomDesc)
    {
      GetStringExtents((*hv_CurrentWindowHandle), "test_string", &hv_Ascent, &hv_Descent, 
          &hv__, &hv__);
      hv_NumLines = hv_ShowBottomDesc;
      hv_MarginBottom = (hv_NumLines*(hv_Ascent+hv_Descent))+(2*12);
    }
    else
    {
      hv_MarginBottom = 0;
    }
    //
    //Get and set meta information for current window.
    update_window_meta_information((*hv_CurrentWindowHandle), hv_WidthImage, hv_HeightImage, 
        0, 0, hv_MapColorBarWidth, hv_MarginBottom, &(*hv_WindowImageRatioHeight), 
        &hv_WindowImageRatioWidth, &hv_SetPartRow2, &hv_SetPartColumn2, &(*hv_PrevWindowCoordinatesOut));
    //
    //Update meta information.
    GetDictTuple(hv_WindowHandleDict, "meta_information", &hv_MetaInfo);
    SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_window_image_ratio_height", (*hv_WindowImageRatioHeight));
    SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_window_image_ratio_width", hv_WindowImageRatioWidth);
    SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_set_part_row2", hv_SetPartRow2);
    SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_set_part_column2", hv_SetPartColumn2);
    SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_margin_bottom", hv_MarginBottom);
    SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_map_color_bar_with", hv_MapColorBarWidth);
    SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_window_coordinates", (*hv_PrevWindowCoordinatesOut));
  }
  else
  {
    //
    //Open a new window.
    open_next_window(hv_Font, hv_FontSize, hv_ShowBottomDesc, hv_WidthImage, hv_HeightImage, 
        hv_MapColorBarWidth, hv_ScaleWindows, hv_ThresholdWidth, hv_PrevWindowCoordinates, 
        hv_WindowHandleDict, hv_WindowHandleKey, &(*hv_CurrentWindowHandle), &(*hv_WindowImageRatioHeight), 
        &(*hv_PrevWindowCoordinatesOut));
    SetWindowParam((*hv_CurrentWindowHandle), "flush", "false");
  }
  //
  return;
}

// Chapter: Deep Learning / Semantic Segmentation and Edge Extraction
// Short Description: Get the ground truth segmentation image. 
void get_segmentation_image_ground_truth (HObject *ho_SegmentationImagGroundTruth, 
    HTuple hv_SampleKeys, HTuple hv_DLSample)
{

  //
  //This procedure returns the ground truth segmentation image.
  //
  if (0 != (int((hv_SampleKeys.TupleFind("segmentation_image"))!=-1)))
  {
    GetDictObject(&(*ho_SegmentationImagGroundTruth), hv_DLSample, "segmentation_image");
  }
  else
  {
    throw HException("Ground truth segmentation image could not be found in DLSample.");
  }
  return;
}

// Chapter: Deep Learning / Semantic Segmentation and Edge Extraction
// Short Description: Get the predicted segmentation result image. 
void get_segmentation_image_result (HObject *ho_SegmentationImageResult, HTuple hv_ResultKeys, 
    HTuple hv_DLResult)
{

  //
  //This procedure returns the predicted segmentation result image.
  //
  if (0 != (int((hv_ResultKeys.TupleFind("segmentation_image"))!=-1)))
  {
    GetDictObject(&(*ho_SegmentationImageResult), hv_DLResult, "segmentation_image");
  }
  else
  {
    throw HException("Result segmentation data could not be found in DLSample.");
  }
  return;
}

// Chapter: Deep Learning / Semantic Segmentation and Edge Extraction
// Short Description: Get the weight image of a sample. 
void get_weight_image (HObject *ho_ImageWeight, HTuple hv_SampleKeys, HTuple hv_DLSample)
{

  //
  //This procedure returns the segmentation weight image of a sample.
  //
  if (0 != (int((hv_SampleKeys.TupleFind("weight_image"))!=-1)))
  {
    GetDictObject(&(*ho_ImageWeight), hv_DLSample, "weight_image");
  }
  else
  {
    throw HException("Weight image could not be found in DLSample.");
  }
  return;
}

// Chapter: File / Misc
// Short Description: Get all image files under the given path 
void list_image_files (HTuple hv_ImageDirectory, HTuple hv_Extensions, HTuple hv_Options, 
    HTuple *hv_ImageFiles)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_ImageDirectoryIndex, hv_ImageFilesTmp;
  HTuple  hv_CurrentImageDirectory, hv_HalconImages, hv_OS;
  HTuple  hv_Directories, hv_Index, hv_Length, hv_NetworkDrive;
  HTuple  hv_Substring, hv_FileExists, hv_AllFiles, hv_i;
  HTuple  hv_Selection;

  //This procedure returns all files in a given directory
  //with one of the suffixes specified in Extensions.
  //
  //Input parameters:
  //ImageDirectory: Directory or a tuple of directories with images.
  //   If a directory is not found locally, the respective directory
  //   is searched under %HALCONIMAGES%/ImageDirectory.
  //   See the Installation Guide for further information
  //   in case %HALCONIMAGES% is not set.
  //Extensions: A string tuple containing the extensions to be found
  //   e.g. ['png','tif',jpg'] or others
  //If Extensions is set to 'default' or the empty string '',
  //   all image suffixes supported by HALCON are used.
  //Options: as in the operator list_files, except that the 'files'
  //   option is always used. Note that the 'directories' option
  //   has no effect but increases runtime, because only files are
  //   returned.
  //
  //Output parameter:
  //ImageFiles: A tuple of all found image file names
  //
  if (0 != (HTuple(HTuple(int(hv_Extensions==HTuple())).TupleOr(int(hv_Extensions==HTuple("")))).TupleOr(int(hv_Extensions==HTuple("default")))))
  {
    hv_Extensions.Clear();
    hv_Extensions[0] = "ima";
    hv_Extensions[1] = "tif";
    hv_Extensions[2] = "tiff";
    hv_Extensions[3] = "gif";
    hv_Extensions[4] = "bmp";
    hv_Extensions[5] = "jpg";
    hv_Extensions[6] = "jpeg";
    hv_Extensions[7] = "jp2";
    hv_Extensions[8] = "jxr";
    hv_Extensions[9] = "png";
    hv_Extensions[10] = "pcx";
    hv_Extensions[11] = "ras";
    hv_Extensions[12] = "xwd";
    hv_Extensions[13] = "pbm";
    hv_Extensions[14] = "pnm";
    hv_Extensions[15] = "pgm";
    hv_Extensions[16] = "ppm";
    //
  }
  (*hv_ImageFiles) = HTuple();
  //Loop through all given image directories.
  {
  HTuple end_val27 = (hv_ImageDirectory.TupleLength())-1;
  HTuple step_val27 = 1;
  for (hv_ImageDirectoryIndex=0; hv_ImageDirectoryIndex.Continue(end_val27, step_val27); hv_ImageDirectoryIndex += step_val27)
  {
    hv_ImageFilesTmp = HTuple();
    hv_CurrentImageDirectory = HTuple(hv_ImageDirectory[hv_ImageDirectoryIndex]);
    if (0 != (int(hv_CurrentImageDirectory==HTuple(""))))
    {
      hv_CurrentImageDirectory = ".";
    }
    GetSystem("image_dir", &hv_HalconImages);
    GetSystem("operating_system", &hv_OS);
    if (0 != (int((hv_OS.TupleSubstr(0,2))==HTuple("Win"))))
    {
      hv_HalconImages = hv_HalconImages.TupleSplit(";");
    }
    else
    {
      hv_HalconImages = hv_HalconImages.TupleSplit(":");
    }
    hv_Directories = hv_CurrentImageDirectory;
    {
    HTuple end_val41 = (hv_HalconImages.TupleLength())-1;
    HTuple step_val41 = 1;
    for (hv_Index=0; hv_Index.Continue(end_val41, step_val41); hv_Index += step_val41)
    {
      hv_Directories = hv_Directories.TupleConcat((HTuple(hv_HalconImages[hv_Index])+"/")+hv_CurrentImageDirectory);
    }
    }
    TupleStrlen(hv_Directories, &hv_Length);
    TupleGenConst(hv_Length.TupleLength(), 0, &hv_NetworkDrive);
    if (0 != (int((hv_OS.TupleSubstr(0,2))==HTuple("Win"))))
    {
      {
      HTuple end_val47 = (hv_Length.TupleLength())-1;
      HTuple step_val47 = 1;
      for (hv_Index=0; hv_Index.Continue(end_val47, step_val47); hv_Index += step_val47)
      {
        if (0 != (int((HTuple(hv_Directories[hv_Index]).TupleStrlen())>1)))
        {
          TupleStrFirstN(HTuple(hv_Directories[hv_Index]), 1, &hv_Substring);
          if (0 != (HTuple(int(hv_Substring==HTuple("//"))).TupleOr(int(hv_Substring==HTuple("\\\\")))))
          {
            hv_NetworkDrive[hv_Index] = 1;
          }
        }
      }
      }
    }
    hv_ImageFilesTmp = HTuple();
    {
    HTuple end_val57 = (hv_Directories.TupleLength())-1;
    HTuple step_val57 = 1;
    for (hv_Index=0; hv_Index.Continue(end_val57, step_val57); hv_Index += step_val57)
    {
      FileExists(HTuple(hv_Directories[hv_Index]), &hv_FileExists);
      if (0 != hv_FileExists)
      {
        ListFiles(HTuple(hv_Directories[hv_Index]), HTuple("files").TupleConcat(hv_Options), 
            &hv_AllFiles);
        hv_ImageFilesTmp = HTuple();
        {
        HTuple end_val62 = (hv_Extensions.TupleLength())-1;
        HTuple step_val62 = 1;
        for (hv_i=0; hv_i.Continue(end_val62, step_val62); hv_i += step_val62)
        {
          TupleRegexpSelect(hv_AllFiles, ((".*"+HTuple(hv_Extensions[hv_i]))+"$").TupleConcat("ignore_case"), 
              &hv_Selection);
          hv_ImageFilesTmp = hv_ImageFilesTmp.TupleConcat(hv_Selection);
        }
        }
        TupleRegexpReplace(hv_ImageFilesTmp, (HTuple("\\\\").Append("replace_all")), 
            "/", &hv_ImageFilesTmp);
        if (0 != (HTuple(hv_NetworkDrive[hv_Index])))
        {
          TupleRegexpReplace(hv_ImageFilesTmp, (HTuple("//").Append("replace_all")), 
              "/", &hv_ImageFilesTmp);
          hv_ImageFilesTmp = "/"+hv_ImageFilesTmp;
        }
        else
        {
          TupleRegexpReplace(hv_ImageFilesTmp, (HTuple("//").Append("replace_all")), 
              "/", &hv_ImageFilesTmp);
        }
        break;
      }
    }
    }
    //Concatenate the output image paths.
    (*hv_ImageFiles) = (*hv_ImageFiles).TupleConcat(hv_ImageFilesTmp);
  }
  }
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Shuffle the input colors in a deterministic way 
void make_neighboring_colors_distinguishable (HTuple hv_ColorsRainbow, HTuple *hv_Colors)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_NumColors, hv_NumChunks, hv_NumLeftOver;
  HTuple  hv_ColorsPerChunk, hv_StartIdx, hv_S, hv_EndIdx;
  HTuple  hv_IdxsLeft, hv_IdxsRight;

  //
  //Shuffle the input colors in a deterministic way
  //to make adjacent colors more distinguishable.
  //Neighboring colors from the input are distributed to every NumChunks
  //position in the output.
  //Depending on the number of colors, increase NumChunks.
  hv_NumColors = hv_ColorsRainbow.TupleLength();
  if (0 != (int(hv_NumColors>=8)))
  {
    hv_NumChunks = 3;
    if (0 != (int(hv_NumColors>=40)))
    {
      hv_NumChunks = 6;
    }
    else if (0 != (int(hv_NumColors>=20)))
    {
      hv_NumChunks = 4;
    }
    (*hv_Colors) = HTuple(hv_NumColors,-1);
    //Check if the Number of Colors is dividable by NumChunks.
    hv_NumLeftOver = hv_NumColors%hv_NumChunks;
    hv_ColorsPerChunk = (hv_NumColors/hv_NumChunks).TupleInt();
    hv_StartIdx = 0;
    {
    HTuple end_val19 = hv_NumChunks-1;
    HTuple step_val19 = 1;
    for (hv_S=0; hv_S.Continue(end_val19, step_val19); hv_S += step_val19)
    {
      hv_EndIdx = (hv_StartIdx+hv_ColorsPerChunk)-1;
      if (0 != (int(hv_S<hv_NumLeftOver)))
      {
        hv_EndIdx += 1;
      }
      hv_IdxsLeft = HTuple::TupleGenSequence(hv_S,hv_NumColors-1,hv_NumChunks);
      hv_IdxsRight = HTuple::TupleGenSequence(hv_StartIdx,hv_EndIdx,1);
      (*hv_Colors)[HTuple::TupleGenSequence(hv_S,hv_NumColors-1,hv_NumChunks)] = hv_ColorsRainbow.TupleSelectRange(hv_StartIdx,hv_EndIdx);
      hv_StartIdx = hv_EndIdx+1;
    }
    }
  }
  else
  {
    (*hv_Colors) = hv_ColorsRainbow;
  }
  return;
}

// Chapter: Graphics / Window
// Short Description: Open a window next to the given WindowHandleFather.  
void open_child_window (HTuple hv_WindowHandleFather, HTuple hv_Font, HTuple hv_FontSize, 
    HTuple hv_Text, HTuple hv_PrevWindowCoordinates, HTuple hv_WindowHandleDict, 
    HTuple hv_WindowHandleKey, HTuple *hv_WindowHandleChild, HTuple *hv_PrevWindowCoordinatesOut)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_StringWidth, hv_IndexText, hv__, hv_TextWidth;
  HTuple  hv_WindowRow, hv_WindowColumn, hv_WindowWidth, hv_WindowHeight;
  HTuple  hv_MetaInfo;

  //
  //This procedure opens a window next to the given WindowHandleFather.
  //
  //Get the maximum width of the text to be displayed.
  //The width should be at least 200.
  hv_StringWidth = 150;
  {
  HTuple end_val6 = (hv_Text.TupleLength())-1;
  HTuple step_val6 = 1;
  for (hv_IndexText=0; hv_IndexText.Continue(end_val6, step_val6); hv_IndexText += step_val6)
  {
    GetStringExtents(hv_WindowHandleFather, HTuple(hv_Text[hv_IndexText]), &hv__, 
        &hv__, &hv_TextWidth, &hv__);
    hv_StringWidth = hv_StringWidth.TupleMax2(hv_TextWidth);
  }
  }
  //
  //Define window coordinates.
  hv_WindowRow = ((const HTuple&)hv_PrevWindowCoordinates)[0];
  hv_WindowColumn = (HTuple(hv_PrevWindowCoordinates[1])+HTuple(hv_PrevWindowCoordinates[2]))+5;
  hv_WindowWidth = hv_StringWidth+(2*12.0);
  hv_WindowHeight = ((const HTuple&)hv_PrevWindowCoordinates)[3];
  //
  SetWindowAttr("background_color","black");
  OpenWindow(hv_WindowRow,hv_WindowColumn,hv_WindowWidth,hv_WindowHeight,0,"visible","",&(*hv_WindowHandleChild));
  HDevWindowStack::Push((*hv_WindowHandleChild));
  set_display_font((*hv_WindowHandleChild), hv_FontSize, hv_Font, "true", "false");
  //
  //Return the coordinates of the new window.
  (*hv_PrevWindowCoordinatesOut).Clear();
  (*hv_PrevWindowCoordinatesOut).Append(hv_WindowRow);
  (*hv_PrevWindowCoordinatesOut).Append(hv_WindowColumn);
  (*hv_PrevWindowCoordinatesOut).Append(hv_WindowWidth);
  (*hv_PrevWindowCoordinatesOut).Append(hv_WindowHeight);
  //
  //Set some meta information about the new child window handle.
  GetDictTuple(hv_WindowHandleDict, "meta_information", &hv_MetaInfo);
  SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_child_window_coordinates", (*hv_PrevWindowCoordinatesOut));
  SetDictTuple(hv_WindowHandleDict, "meta_information", hv_MetaInfo);
  //
  return;
}

// Chapter: Graphics / Window
// Short Description: Open a new window, either next to the last ones, or in a new row. 
void open_next_window (HTuple hv_Font, HTuple hv_FontSize, HTuple hv_ShowBottomDesc, 
    HTuple hv_WidthImage, HTuple hv_HeightImage, HTuple hv_MapColorBarWidth, HTuple hv_ScaleWindows, 
    HTuple hv_ThresholdWidth, HTuple hv_PrevWindowCoordinates, HTuple hv_WindowHandleDict, 
    HTuple hv_WindowHandleKey, HTuple *hv_WindowHandleNew, HTuple *hv_WindowImageRatioHeight, 
    HTuple *hv_PrevWindowCoordinatesOut)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_PrevWindowRow, hv_PrevWindowColumn;
  HTuple  hv_PrevWindowWidth, hv_PrevWindowHeight, hv_WindowRow;
  HTuple  hv_WindowColumn, hv_Ascent, hv_Descent, hv__, hv_NumLines;
  HTuple  hv_MarginBottom, hv_WindowWidth, hv_WindowHeight;
  HTuple  hv_WindowImageRatioWidth, hv_SetPartRow2, hv_SetPartColumn2;
  HTuple  hv_MetaInfo;

  //
  //This procedure opens a new window, either next to
  //the last ones, or in a new row.
  //
  //Get coordinates of previous window.
  hv_PrevWindowRow = ((const HTuple&)hv_PrevWindowCoordinates)[0];
  hv_PrevWindowColumn = ((const HTuple&)hv_PrevWindowCoordinates)[1];
  hv_PrevWindowWidth = ((const HTuple&)hv_PrevWindowCoordinates)[2];
  hv_PrevWindowHeight = ((const HTuple&)hv_PrevWindowCoordinates)[3];
  //
  if (0 != (int((hv_PrevWindowColumn+hv_PrevWindowWidth)>hv_ThresholdWidth)))
  {
    //Open window in new row.
    hv_WindowRow = (hv_PrevWindowRow+hv_PrevWindowHeight)+55;
    hv_WindowColumn = 0;
  }
  else
  {
    //Open window in same row.
    hv_WindowRow = hv_PrevWindowRow;
    hv_WindowColumn = hv_PrevWindowColumn+hv_PrevWindowWidth;
    if (0 != (int(hv_WindowColumn!=0)))
    {
      hv_WindowColumn += 5;
    }
  }
  //
  dev_open_window_fit_size(hv_WindowRow, hv_WindowColumn, hv_WidthImage, hv_HeightImage, 
      (HTuple(500).Append(800))*hv_ScaleWindows, (HTuple(400).Append(600))*hv_ScaleWindows, 
      &(*hv_WindowHandleNew));
  set_display_font((*hv_WindowHandleNew), hv_FontSize, hv_Font, "true", "false");
  //
  //Add MarginBottom and MapColorBarWidth to window.
  if (0 != hv_ShowBottomDesc)
  {
    GetStringExtents((*hv_WindowHandleNew), "Test_string", &hv_Ascent, &hv_Descent, 
        &hv__, &hv__);
    hv_NumLines = hv_ShowBottomDesc;
    hv_MarginBottom = (hv_NumLines*(hv_Ascent+hv_Descent))+(2*12);
  }
  else
  {
    hv_MarginBottom = 0;
  }
  GetWindowExtents((*hv_WindowHandleNew), &hv__, &hv__, &hv_WindowWidth, &hv_WindowHeight);
  if (HDevWindowStack::IsOpen())
    SetWindowExtents(HDevWindowStack::GetActive(),hv_WindowRow, hv_WindowColumn, 
        hv_WindowWidth+hv_MapColorBarWidth, hv_WindowHeight+hv_MarginBottom);
  //
  //Get and set meta information of new window handle.
  update_window_meta_information((*hv_WindowHandleNew), hv_WidthImage, hv_HeightImage, 
      hv_WindowRow, hv_WindowColumn, hv_MapColorBarWidth, hv_MarginBottom, &(*hv_WindowImageRatioHeight), 
      &hv_WindowImageRatioWidth, &hv_SetPartRow2, &hv_SetPartColumn2, &(*hv_PrevWindowCoordinatesOut));
  //
  //Set window handle and some meta information about the new window handle.
  SetDictTuple(hv_WindowHandleDict, hv_WindowHandleKey, (*hv_WindowHandleNew));
  GetDictTuple(hv_WindowHandleDict, "meta_information", &hv_MetaInfo);
  SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_window_image_ratio_height", (*hv_WindowImageRatioHeight));
  SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_window_image_ratio_width", hv_WindowImageRatioWidth);
  SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_set_part_row2", hv_SetPartRow2);
  SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_set_part_column2", hv_SetPartColumn2);
  SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_margin_bottom", hv_MarginBottom);
  SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_map_color_bar_with", hv_MapColorBarWidth);
  SetDictTuple(hv_MetaInfo, hv_WindowHandleKey+"_window_coordinates", (*hv_PrevWindowCoordinatesOut));
  SetDictTuple(hv_WindowHandleDict, "meta_information", hv_MetaInfo);
  //
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Preprocess 3D data for deep-learning-based training and inference. 
void preprocess_dl_model_3d_data (HTuple hv_DLSample, HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_ImageZ, ho_Domain, ho_Region, ho_ImageReduced;
  HObject  ho_DomainComplement, ho_ImageX, ho_ImageY, ho_ImageXYZ;
  HObject  ho_NXImage, ho_NYImage, ho_NZImage, ho_MultiChannelImage;
  HObject  ho___Tmp_Obj_0;

  // Local control variables
  HTuple  hv_HasNormals, hv_XYZKeys, hv_HasXYZ;
  HTuple  hv_HasX, hv_HasY, hv_HasZ, hv_HasFullXYZ, hv_NumChannels;
  HTuple  hv_Type, hv_Index, hv_Key, hv_ZMinMaxExist, hv_GrayvalOutsideInit;
  HTuple  hv_NormalSizeExists, hv_NormalWidth, hv_NormalHeight;
  HTuple  hv_WidthZ, hv_HeightZ, hv_ZoomNormals, hv_Width;
  HTuple  hv_Height, hv_ScaleWidth, hv_ScaleHeight, hv_XIndex;
  HTuple  hv_YIndex;

  //
  //This procedure preprocesses 3D data of a DLSample.
  //
  //Check presence of inputs in DLSample.
  //
  GetDictParam(hv_DLSample, "key_exists", "normals", &hv_HasNormals);
  hv_XYZKeys.Clear();
  hv_XYZKeys[0] = "x";
  hv_XYZKeys[1] = "y";
  hv_XYZKeys[2] = "z";
  GetDictParam(hv_DLSample, "key_exists", hv_XYZKeys, &hv_HasXYZ);
  hv_HasX = ((const HTuple&)hv_HasXYZ)[0];
  hv_HasY = ((const HTuple&)hv_HasXYZ)[1];
  hv_HasZ = ((const HTuple&)hv_HasXYZ)[2];
  TupleMin(hv_HasXYZ, &hv_HasFullXYZ);
  if (0 != (hv_HasNormals.TupleNot()))
  {
    //XYZ are required because normals would need to be computed.
    if (0 != (hv_HasFullXYZ.TupleNot()))
    {
      throw HException(HTuple("The given input DLSample does not contain necessary images 'x','y' and 'z'. This is required if no normals are provided."));
    }
  }
  else
  {
    //At least Z is required if normals are given.
    if (0 != (hv_HasZ.TupleNot()))
    {
      throw HException(HTuple("The given input DLSample does not contain at least the depth image 'z'. This is required because normals are provided. Optionally, 'x' and 'y' images might be provided additionally."));
    }
    CountChannels(hv_DLSample.TupleGetDictObject("normals"), &hv_NumChannels);
    if (0 != (int(hv_NumChannels!=3)))
    {
      throw HException("The given input DLSample.normals has to have three channels.");
    }
    GetImageType(hv_DLSample.TupleGetDictObject("normals"), &hv_Type);
    if (0 != (int(hv_Type!=HTuple("real"))))
    {
      throw HException("The given input DLSample.normals is not a real image.");
    }
  }
  {
  HTuple end_val31 = (hv_HasXYZ.TupleLength())-1;
  HTuple step_val31 = 1;
  for (hv_Index=0; hv_Index.Continue(end_val31, step_val31); hv_Index += step_val31)
  {
    if (0 != (HTuple(hv_HasXYZ[hv_Index])))
    {
      hv_Key = HTuple(hv_XYZKeys[hv_Index]);
      CountChannels(hv_DLSample.TupleGetDictObject(hv_Key), &hv_NumChannels);
      if (0 != (int(hv_NumChannels!=1)))
      {
        throw HException(("The given input DLSample."+hv_Key)+" needs to have a single channel.");
      }
      GetImageType(hv_DLSample.TupleGetDictObject(hv_Key), &hv_Type);
      if (0 != (int(hv_Type!=HTuple("real"))))
      {
        throw HException(("The given input DLSample."+hv_Key)+" is not a real image.");
      }
    }
  }
  }
  //
  ho_ImageZ = hv_DLSample.TupleGetDictObject("z");
  GetDomain(ho_ImageZ, &ho_Domain);
  //Reduce Z domain to user-defined min/max values for Z.
  GetDictParam(hv_DLPreprocessParam, "key_exists", (HTuple("min_z").Append("max_z")), 
      &hv_ZMinMaxExist);
  if (0 != (HTuple(hv_ZMinMaxExist[0])))
  {
    Threshold(ho_ImageZ, &ho_Region, "min", hv_DLPreprocessParam.TupleGetDictTuple("min_z"));
    Difference(ho_Domain, ho_Region, &ho_Domain);
  }
  if (0 != (HTuple(hv_ZMinMaxExist[1])))
  {
    Threshold(ho_ImageZ, &ho_Region, hv_DLPreprocessParam.TupleGetDictTuple("max_z"), 
        "max");
    Difference(ho_Domain, ho_Region, &ho_Domain);
  }
  //Reduce domain because it might have changed
  if (0 != (hv_ZMinMaxExist.TupleMax()))
  {
    ReduceDomain(ho_ImageZ, ho_Domain, &ho_ImageReduced);
  }
  Complement(ho_Domain, &ho_DomainComplement);
  //
  //Before we zoom any 3D images we want to set all pixels outside of the domain to
  //an invalid value.
  hv_GrayvalOutsideInit = 0;

  if (0 != hv_HasFullXYZ)
  {
    ho_ImageX = hv_DLSample.TupleGetDictObject("x");
    ho_ImageY = hv_DLSample.TupleGetDictObject("y");
    ho_ImageZ = hv_DLSample.TupleGetDictObject("z");

    FullDomain(ho_ImageX, &ho_ImageX);
    FullDomain(ho_ImageY, &ho_ImageY);
    FullDomain(ho_ImageZ, &ho_ImageZ);

    OverpaintRegion(ho_ImageX, ho_DomainComplement, hv_GrayvalOutsideInit, "fill");
    OverpaintRegion(ho_ImageY, ho_DomainComplement, hv_GrayvalOutsideInit, "fill");
    OverpaintRegion(ho_ImageZ, ho_DomainComplement, hv_GrayvalOutsideInit, "fill");

    ReduceDomain(ho_ImageX, ho_Domain, &ho_ImageX);
    ReduceDomain(ho_ImageY, ho_Domain, &ho_ImageY);
    ReduceDomain(ho_ImageZ, ho_Domain, &ho_ImageZ);

    if (0 != (hv_HasNormals.TupleNot()))
    {
      //Get optional user-defined resolution of normal computation.
      GetDictParam(hv_DLPreprocessParam, "key_exists", (HTuple("normal_image_width").Append("normal_image_height")), 
          &hv_NormalSizeExists);
      if (0 != (HTuple(hv_NormalSizeExists[0]).TupleNot()))
      {
        hv_NormalWidth = ((hv_DLPreprocessParam.TupleGetDictTuple("image_width"))*1.5).TupleInt();
      }
      else
      {
        hv_NormalWidth = hv_DLPreprocessParam.TupleGetDictTuple("normal_image_width");
      }
      if (0 != (HTuple(hv_NormalSizeExists[1]).TupleNot()))
      {
        hv_NormalHeight = ((hv_DLPreprocessParam.TupleGetDictTuple("image_height"))*1.5).TupleInt();
      }
      else
      {
        hv_NormalHeight = hv_DLPreprocessParam.TupleGetDictTuple("normal_image_height");
      }

      GetImageSize(ho_ImageZ, &hv_WidthZ, &hv_HeightZ);
      hv_ZoomNormals = HTuple(int(hv_NormalWidth!=hv_WidthZ)).TupleOr(int(hv_NormalHeight!=hv_HeightZ));

      if (0 != hv_ZoomNormals)
      {
        Compose3(ho_ImageX, ho_ImageY, ho_ImageZ, &ho_ImageXYZ);
        GetImageSize(ho_ImageXYZ, &hv_Width, &hv_Height);
        ZoomImageSize(ho_ImageXYZ, &ho_ImageXYZ, hv_NormalWidth, hv_NormalHeight, 
            "nearest_neighbor");
        AccessChannel(ho_ImageXYZ, &ho_ImageX, 1);
        AccessChannel(ho_ImageXYZ, &ho_ImageY, 2);
        AccessChannel(ho_ImageXYZ, &ho_ImageZ, 3);
        hv_ScaleWidth = hv_NormalWidth/(hv_Width.TupleReal());
        hv_ScaleHeight = hv_NormalHeight/(hv_Height.TupleReal());
        ZoomRegion(ho_Domain, &ho_Domain, hv_ScaleWidth, hv_ScaleHeight);
        remove_invalid_3d_pixels(ho_ImageX, ho_ImageY, ho_ImageZ, ho_Domain, &ho_Domain, 
            hv_GrayvalOutsideInit);
        Complement(ho_Domain, &ho_DomainComplement);
      }

      compute_normals_xyz(ho_ImageX, ho_ImageY, ho_ImageZ, &ho_NXImage, &ho_NYImage, 
          &ho_NZImage, 1);
    }
    else
    {
      AccessChannel(hv_DLSample.TupleGetDictObject("normals"), &ho_NXImage, 1);
      AccessChannel(hv_DLSample.TupleGetDictObject("normals"), &ho_NYImage, 2);
      AccessChannel(hv_DLSample.TupleGetDictObject("normals"), &ho_NZImage, 3);
    }
  }
  else
  {
    GenEmptyObj(&ho_ImageX);
    GenEmptyObj(&ho_ImageY);

    AccessChannel(hv_DLSample.TupleGetDictObject("normals"), &ho_NXImage, 1);
    AccessChannel(hv_DLSample.TupleGetDictObject("normals"), &ho_NYImage, 2);
    AccessChannel(hv_DLSample.TupleGetDictObject("normals"), &ho_NZImage, 3);
  }

  FullDomain(ho_ImageZ, &ho_ImageZ);

  FullDomain(ho_NXImage, &ho_NXImage);
  FullDomain(ho_NYImage, &ho_NYImage);
  FullDomain(ho_NZImage, &ho_NZImage);

  //full_domain does not change the pixels outside of the existing domain.
  //Hence we have to set a specific value
  OverpaintRegion(ho_NXImage, ho_DomainComplement, hv_GrayvalOutsideInit, "fill");
  OverpaintRegion(ho_NYImage, ho_DomainComplement, hv_GrayvalOutsideInit, "fill");
  OverpaintRegion(ho_NZImage, ho_DomainComplement, hv_GrayvalOutsideInit, "fill");
  OverpaintRegion(ho_ImageZ, ho_DomainComplement, hv_GrayvalOutsideInit, "fill");

  Compose4(ho_NXImage, ho_NYImage, ho_NZImage, ho_ImageZ, &ho_MultiChannelImage);

  CountObj(ho_ImageX, &hv_HasX);
  if (0 != hv_HasX)
  {
    FullDomain(ho_ImageX, &ho_ImageX);
    AppendChannel(ho_MultiChannelImage, ho_ImageX, &ho_MultiChannelImage);
    CountChannels(ho_MultiChannelImage, &hv_XIndex);
  }
  CountObj(ho_ImageY, &hv_HasY);
  if (0 != hv_HasY)
  {
    FullDomain(ho_ImageY, &ho_ImageY);
    AppendChannel(ho_MultiChannelImage, ho_ImageY, &ho_MultiChannelImage);
    CountChannels(ho_MultiChannelImage, &hv_YIndex);
  }
  GetImageSize(ho_MultiChannelImage, &hv_Width, &hv_Height);
  ZoomImageSize(ho_MultiChannelImage, &ho_MultiChannelImage, hv_DLPreprocessParam.TupleGetDictTuple("image_width"), 
      hv_DLPreprocessParam.TupleGetDictTuple("image_height"), "nearest_neighbor");

  Decompose4(ho_MultiChannelImage, &ho_NXImage, &ho_NYImage, &ho_NZImage, &ho_ImageZ
      );
  if (0 != hv_HasX)
  {
    AccessChannel(ho_MultiChannelImage, &ho_ImageX, hv_XIndex);
  }
  if (0 != hv_HasY)
  {
    AccessChannel(ho_MultiChannelImage, &ho_ImageY, hv_YIndex);
  }


  //Zoom the domain
  hv_ScaleWidth = (hv_DLPreprocessParam.TupleGetDictTuple("image_width"))/(hv_Width.TupleReal());
  hv_ScaleHeight = (hv_DLPreprocessParam.TupleGetDictTuple("image_height"))/(hv_Height.TupleReal());
  ZoomRegion(ho_Domain, &ho_Domain, hv_ScaleWidth, hv_ScaleHeight);
  remove_invalid_3d_pixels(ho_NXImage, ho_NYImage, ho_NZImage, ho_Domain, &ho_Domain, 
      hv_GrayvalOutsideInit);

  ReduceDomain(ho_ImageX, ho_Domain, &ho_ImageX);
  ReduceDomain(ho_ImageY, ho_Domain, &ho_ImageY);
  ReduceDomain(ho_ImageZ, ho_Domain, &ho_ImageZ);
  Compose3(ho_NXImage, ho_NYImage, ho_NZImage, &ho___Tmp_Obj_0);
  SetDictObject(ho___Tmp_Obj_0, hv_DLSample, "normals");
  ReduceDomain(hv_DLSample.TupleGetDictObject("normals"), ho_Domain, &ho___Tmp_Obj_0
      );
  SetDictObject(ho___Tmp_Obj_0, hv_DLSample, "normals");

  //Overwrite preprocessed 3D data
  if (0 != hv_HasX)
  {
    SetDictObject(ho_ImageX, hv_DLSample, "x");
  }
  if (0 != hv_HasY)
  {
    SetDictObject(ho_ImageY, hv_DLSample, "y");
  }
  if (0 != hv_HasZ)
  {
    SetDictObject(ho_ImageZ, hv_DLSample, "z");
  }

  return;
}

// Chapter: Deep Learning / Model
// Short Description: Preprocess anomaly images for evaluation and visualization of deep-learning-based anomaly detection or Global Context Anomaly Detection. 
void preprocess_dl_model_anomaly (HObject ho_AnomalyImages, HObject *ho_AnomalyImagesPreprocessed, 
    HTuple hv_DLPreprocessParam)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_ImageWidth, hv_ImageHeight, hv_ImageRangeMin;
  HTuple  hv_ImageRangeMax, hv_DomainHandling, hv_ModelType;
  HTuple  hv_ImageNumChannels, hv_Min, hv_Max, hv_Range, hv_ImageWidthInput;
  HTuple  hv_ImageHeightInput, hv_EqualWidth, hv_EqualHeight;
  HTuple  hv_Type, hv_NumMatches, hv_NumImages, hv_EqualByte;
  HTuple  hv_NumChannelsAllImages, hv_ImageNumChannelsTuple;
  HTuple  hv_IndicesWrongChannels;

  //
  //This procedure preprocesses the anomaly images given by AnomalyImages
  //according to the parameters in the dictionary DLPreprocessParam.
  //Note that depending on the images,
  //additional preprocessing steps might be beneficial.
  //
  //Check the validity of the preprocessing parameters.
  check_dl_preprocess_param(hv_DLPreprocessParam);
  //
  //Get the preprocessing parameters.
  GetDictTuple(hv_DLPreprocessParam, "image_width", &hv_ImageWidth);
  GetDictTuple(hv_DLPreprocessParam, "image_height", &hv_ImageHeight);
  GetDictTuple(hv_DLPreprocessParam, "image_range_min", &hv_ImageRangeMin);
  GetDictTuple(hv_DLPreprocessParam, "image_range_max", &hv_ImageRangeMax);
  GetDictTuple(hv_DLPreprocessParam, "domain_handling", &hv_DomainHandling);
  GetDictTuple(hv_DLPreprocessParam, "model_type", &hv_ModelType);
  //
  hv_ImageNumChannels = 1;
  //
  //Preprocess the images.
  //
  if (0 != (int(hv_DomainHandling==HTuple("full_domain"))))
  {
    FullDomain(ho_AnomalyImages, &ho_AnomalyImages);
  }
  else if (0 != (int(hv_DomainHandling==HTuple("crop_domain"))))
  {
    CropDomain(ho_AnomalyImages, &ho_AnomalyImages);
  }
  else if (0 != (HTuple(int(hv_DomainHandling==HTuple("keep_domain"))).TupleAnd(int(hv_ModelType==HTuple("anomaly_detection")))))
  {
    //The option 'keep_domain' is only supported for models of 'type' = 'anomaly_detection'
  }
  else
  {
    throw HException("Unsupported parameter value for 'domain_handling'");
  }
  //
  MinMaxGray(ho_AnomalyImages, ho_AnomalyImages, 0, &hv_Min, &hv_Max, &hv_Range);
  if (0 != (int(hv_Min<0.0)))
  {
    throw HException("Values of anomaly image must not be smaller than 0.0.");
  }
  //
  //Zoom images only if they have a different size than the specified size.
  GetImageSize(ho_AnomalyImages, &hv_ImageWidthInput, &hv_ImageHeightInput);
  hv_EqualWidth = hv_ImageWidth.TupleEqualElem(hv_ImageWidthInput);
  hv_EqualHeight = hv_ImageHeight.TupleEqualElem(hv_ImageHeightInput);
  if (0 != (HTuple(int((hv_EqualWidth.TupleMin())==0)).TupleOr(int((hv_EqualHeight.TupleMin())==0))))
  {
    ZoomImageSize(ho_AnomalyImages, &ho_AnomalyImages, hv_ImageWidth, hv_ImageHeight, 
        "nearest_neighbor");
  }
  //
  //Check the type of the input images.
  GetImageType(ho_AnomalyImages, &hv_Type);
  TupleRegexpTest(hv_Type, "byte|real", &hv_NumMatches);
  CountObj(ho_AnomalyImages, &hv_NumImages);
  if (0 != (int(hv_NumMatches!=hv_NumImages)))
  {
    throw HException("Please provide only images of type 'byte' or 'real'.");
  }
  //
  //If the type is 'byte', convert it to 'real' and scale it.
  //The gray value scaling does not work on 'byte' images.
  //For 'real' images it is assumed that the range is already correct.
  hv_EqualByte = hv_Type.TupleEqualElem("byte");
  if (0 != (int((hv_EqualByte.TupleMax())==1)))
  {
    if (0 != (int((hv_EqualByte.TupleMin())==0)))
    {
      throw HException("Passing mixed type images is not supported.");
    }
    //Convert the image type from 'byte' to 'real',
    //because the model expects 'real' images.
    ConvertImageType(ho_AnomalyImages, &ho_AnomalyImages, "real");
  }
  //
  //Check the number of channels.
  CountObj(ho_AnomalyImages, &hv_NumImages);
  //Check all images for number of channels.
  CountChannels(ho_AnomalyImages, &hv_NumChannelsAllImages);
  TupleGenConst(hv_NumImages, hv_ImageNumChannels, &hv_ImageNumChannelsTuple);
  TupleFind(hv_NumChannelsAllImages.TupleNotEqualElem(hv_ImageNumChannelsTuple), 
      1, &hv_IndicesWrongChannels);
  //
  //Check for anomaly image channels.
  //Only single channel images are accepted.
  if (0 != (int(hv_IndicesWrongChannels!=-1)))
  {
    throw HException("Number of channels in anomaly image is not supported. Please check for anomaly images with a number of channels different from 1.");
  }
  //
  //Write preprocessed image to output variable.
  (*ho_AnomalyImagesPreprocessed) = ho_AnomalyImages;
  //
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Preprocess the provided DLSample image for augmentation purposes. 
void preprocess_dl_model_augmentation_data (HTuple hv_DLSample, HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_InputImage, ho_ImageHighRes;

  // Local control variables
  HTuple  hv_ImageWidth, hv_ImageHeight, hv_ImageNumChannels;
  HTuple  hv_ModelType, hv_AugmentationKeyExists, hv_ImageKeyExists;
  HTuple  hv_NumImages, hv_NumChannels, hv_ImageType, hv_InputImageWidth;
  HTuple  hv_InputImageHeight, hv_InputImageWidthHeightRatio;
  HTuple  hv_ZoomHeight, hv_ZoomWidth, hv_HasPadding, hv_ZoomFactorWidth;
  HTuple  hv_ZoomFactorHeight, hv_UseZoomImage, hv_DLSampleHighRes;
  HTuple  hv_DLPreprocessParamHighRes, hv___Tmp_Ctrl_Dict_Init_0;
  HTuple  hv___Tmp_Ctrl_Dict_Init_1, hv___Tmp_Ctrl_Dict_Init_2;

  //This procedure preprocesses the provided DLSample image for augmentation purposes.
  //
  //Check the validity of the preprocessing parameters.
  check_dl_preprocess_param(hv_DLPreprocessParam);
  //
  //Get the required preprocessing parameters.
  hv_ImageWidth = hv_DLPreprocessParam.TupleGetDictTuple("image_width");
  hv_ImageHeight = hv_DLPreprocessParam.TupleGetDictTuple("image_height");
  hv_ImageNumChannels = hv_DLPreprocessParam.TupleGetDictTuple("image_num_channels");
  hv_ModelType = hv_DLPreprocessParam.TupleGetDictTuple("model_type");
  //
  //Determine whether the preprocessing is required or not.
  GetDictParam(hv_DLPreprocessParam, "key_exists", "augmentation", &hv_AugmentationKeyExists);
  if (0 != (hv_AugmentationKeyExists.TupleNot()))
  {
    return;
  }
  CreateDict(&hv___Tmp_Ctrl_Dict_Init_0);
  SetDictTuple(hv___Tmp_Ctrl_Dict_Init_0, "comp", "true");
  if (0 != (((hv_DLPreprocessParam.TupleConcat(hv___Tmp_Ctrl_Dict_Init_0)).TupleTestEqualDictItem("augmentation","comp")).TupleNot()))
  {
    return;
  }
  hv___Tmp_Ctrl_Dict_Init_0 = HTuple::TupleConstant("HNULL");
  if (0 != (HTuple(int(hv_ModelType!=HTuple("ocr_detection"))).TupleAnd(int(hv_ModelType!=HTuple("ocr_recognition")))))
  {
    return;
  }
  //
  //Get the input image and its properties.
  GetDictParam(hv_DLSample, "key_exists", "image", &hv_ImageKeyExists);
  if (0 != (hv_ImageKeyExists.TupleNot()))
  {
    throw HException("The sample to process needs to include an image.");
  }
  ho_InputImage = hv_DLSample.TupleGetDictObject("image");
  CountObj(ho_InputImage, &hv_NumImages);
  if (0 != (int(hv_NumImages!=1)))
  {
    throw HException("The sample to process needs to include exactly 1 image.");
  }
  CountChannels(ho_InputImage, &hv_NumChannels);
  GetImageType(ho_InputImage, &hv_ImageType);
  GetImageSize(ho_InputImage, &hv_InputImageWidth, &hv_InputImageHeight);
  //
  //Execute model specific preprocessing.
  if (0 != (int(hv_ModelType==HTuple("ocr_recognition"))))
  {
    if (0 != (int(hv_ImageNumChannels!=1)))
    {
      throw HException("The only 'image_num_channels' value supported for ocr_recognition models is 1.");
    }
    if (0 != (int((hv_ImageType.TupleRegexpTest("byte|real"))!=1)))
    {
      throw HException("Please provide only images of type 'byte' or 'real' for ocr_recognition models.");
    }
    if (0 != (int((HTuple((hv_NumChannels.TupleEqualElem(1)).TupleOr(hv_NumChannels.TupleEqualElem(3))).TupleSum())!=1)))
    {
      throw HException("Please provide only 1- or 3-channels images for ocr_recognition models.");
    }
    //
    FullDomain(ho_InputImage, &ho_ImageHighRes);
    if (0 != (int(hv_NumChannels==3)))
    {
      Rgb1ToGray(ho_ImageHighRes, &ho_ImageHighRes);
    }
    hv_InputImageWidthHeightRatio = hv_InputImageWidth/(hv_InputImageHeight.TupleReal());
    hv_ZoomHeight = hv_InputImageHeight.TupleMin2(2*hv_ImageHeight);
    hv_ZoomWidth = (hv_ZoomHeight*hv_InputImageWidthHeightRatio).TupleInt();
    hv_HasPadding = int(((hv_ImageHeight*hv_InputImageWidthHeightRatio).TupleInt())<hv_ImageWidth);
    if (0 != (HTuple(int(hv_ZoomHeight>hv_ImageHeight)).TupleOr(hv_HasPadding)))
    {
      ZoomImageSize(ho_ImageHighRes, &ho_ImageHighRes, hv_ZoomWidth, hv_ZoomHeight, 
          "constant");
      CreateDict(&hv___Tmp_Ctrl_Dict_Init_1);
      SetDictTuple(hv_DLSample, "augmentation_data", hv___Tmp_Ctrl_Dict_Init_1);
      hv___Tmp_Ctrl_Dict_Init_1 = HTuple::TupleConstant("HNULL");
      SetDictObject(ho_ImageHighRes, hv_DLSample.TupleGetDictTuple("augmentation_data"), 
          "image_high_res");
      SetDictTuple(hv_DLSample.TupleGetDictTuple("augmentation_data"), "preprocess_params", 
          hv_DLPreprocessParam);
    }
  }
  else if (0 != (int(hv_ModelType==HTuple("ocr_detection"))))
  {
    if (0 != (int(hv_ImageNumChannels!=3)))
    {
      throw HException("The only 'image_num_channels' value supported for ocr_detection models is 3.");
    }
    if (0 != (int((hv_ImageType.TupleRegexpTest("byte|real"))!=1)))
    {
      throw HException("Please provide only images of type 'byte' or 'real' for ocr_detection models.");
    }
    if (0 != (int((HTuple((hv_NumChannels.TupleEqualElem(1)).TupleOr(hv_NumChannels.TupleEqualElem(3))).TupleSum())!=1)))
    {
      throw HException("Please provide only 1- or 3-channels images for ocr_detection models.");
    }
    //
    //Calculate aspect-ratio preserving zoom dimensions for high resolution.
    calculate_dl_image_zoom_factors(hv_InputImageWidth, hv_InputImageHeight, 2*hv_ImageWidth, 
        2*hv_ImageHeight, hv_DLPreprocessParam, &hv_ZoomFactorWidth, &hv_ZoomFactorHeight);
    hv_ZoomHeight = (hv_ZoomFactorHeight*hv_InputImageHeight).TupleRound();
    hv_ZoomWidth = (hv_ZoomFactorWidth*hv_InputImageWidth).TupleRound();
    //
    //Use the better size for high resolution: 2x resolution size of preprocess image or input image size.
    hv_UseZoomImage = HTuple(int(hv_ZoomWidth<hv_InputImageWidth)).TupleOr(int(hv_ZoomHeight<hv_InputImageHeight));
    CopyDict(hv_DLSample, HTuple(), HTuple(), &hv_DLSampleHighRes);
    CopyDict(hv_DLPreprocessParam, HTuple(), HTuple(), &hv_DLPreprocessParamHighRes);
    //
    FullDomain(ho_InputImage, &ho_ImageHighRes);
    if (0 != hv_UseZoomImage)
    {
      SetDictTuple(hv_DLPreprocessParamHighRes, "image_width", hv_ZoomWidth);
      SetDictTuple(hv_DLPreprocessParamHighRes, "image_height", hv_ZoomHeight);
      preprocess_dl_model_bbox_rect2(ho_ImageHighRes, hv_DLSampleHighRes, hv_DLPreprocessParamHighRes);
      gen_dl_ocr_detection_targets(hv_DLSampleHighRes, hv_DLPreprocessParamHighRes);
      ZoomImageSize(ho_ImageHighRes, &ho_ImageHighRes, hv_ZoomWidth, hv_ZoomHeight, 
          "constant");
    }
    else
    {
      SetDictTuple(hv_DLPreprocessParamHighRes, "image_width", hv_InputImageWidth);
      SetDictTuple(hv_DLPreprocessParamHighRes, "image_height", hv_InputImageHeight);
      gen_dl_ocr_detection_targets(hv_DLSampleHighRes, hv_DLPreprocessParamHighRes);
    }
    SetDictObject(ho_ImageHighRes, hv_DLSampleHighRes, "image");
    //
    CreateDict(&hv___Tmp_Ctrl_Dict_Init_2);
    SetDictTuple(hv_DLSample, "augmentation_data", hv___Tmp_Ctrl_Dict_Init_2);
    hv___Tmp_Ctrl_Dict_Init_2 = HTuple::TupleConstant("HNULL");
    SetDictTuple(hv_DLSample.TupleGetDictTuple("augmentation_data"), "sample_high_res", 
        hv_DLSampleHighRes);
    SetDictTuple(hv_DLSample.TupleGetDictTuple("augmentation_data"), "preprocess_params", 
        hv_DLPreprocessParam);
  }
  //
  return;
}

// Chapter: Deep Learning / Object Detection and Instance Segmentation
// Short Description: Preprocess the bounding boxes of type 'rectangle1' for a given sample. 
void preprocess_dl_model_bbox_rect1 (HObject ho_ImageRaw, HTuple hv_DLSample, HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_DomainRaw;

  // Local control variables
  HTuple  hv_ImageWidth, hv_ImageHeight, hv_DomainHandling;
  HTuple  hv_BBoxCol1, hv_BBoxCol2, hv_BBoxRow1, hv_BBoxRow2;
  HTuple  hv_BBoxLabel, hv_Exception, hv_ImageId, hv_ExceptionMessage;
  HTuple  hv_BoxesInvalid, hv_DomainRow1, hv_DomainColumn1;
  HTuple  hv_DomainRow2, hv_DomainColumn2, hv_WidthRaw, hv_HeightRaw;
  HTuple  hv_Row1, hv_Col1, hv_Row2, hv_Col2, hv_MaskDelete;
  HTuple  hv_MaskNewBbox, hv_BBoxCol1New, hv_BBoxCol2New;
  HTuple  hv_BBoxRow1New, hv_BBoxRow2New, hv_BBoxLabelNew;
  HTuple  hv_FactorResampleWidth, hv_FactorResampleHeight;

  //
  //This procedure preprocesses the bounding boxes of type 'rectangle1' for a given sample.
  //
  //Check the validity of the preprocessing parameters.
  check_dl_preprocess_param(hv_DLPreprocessParam);
  //
  //Get the preprocessing parameters.
  GetDictTuple(hv_DLPreprocessParam, "image_width", &hv_ImageWidth);
  GetDictTuple(hv_DLPreprocessParam, "image_height", &hv_ImageHeight);
  GetDictTuple(hv_DLPreprocessParam, "domain_handling", &hv_DomainHandling);
  //
  //Get bounding box coordinates and labels.
  try
  {
    GetDictTuple(hv_DLSample, "bbox_col1", &hv_BBoxCol1);
    GetDictTuple(hv_DLSample, "bbox_col2", &hv_BBoxCol2);
    GetDictTuple(hv_DLSample, "bbox_row1", &hv_BBoxRow1);
    GetDictTuple(hv_DLSample, "bbox_row2", &hv_BBoxRow2);
    GetDictTuple(hv_DLSample, "bbox_label_id", &hv_BBoxLabel);
  }
  // catch (Exception) 
  catch (HException &HDevExpDefaultException)
  {
    HDevExpDefaultException.ToHTuple(&hv_Exception);
    GetDictTuple(hv_DLSample, "image_id", &hv_ImageId);
    if (0 != (int(HTuple(hv_Exception[0])==1302)))
    {
      hv_ExceptionMessage = "A bounding box coordinate key is missing.";
    }
    else
    {
      hv_ExceptionMessage = ((const HTuple&)hv_Exception)[2];
    }
    throw HException((("An error has occurred during preprocessing image_id "+hv_ImageId)+" when getting bounding box coordinates : ")+hv_ExceptionMessage);
  }
  //
  //Check that there are no invalid boxes.
  if (0 != (int((hv_BBoxRow1.TupleLength())>0)))
  {
    hv_BoxesInvalid = (hv_BBoxRow1.TupleGreaterEqualElem(hv_BBoxRow2)).TupleOr(hv_BBoxCol1.TupleGreaterEqualElem(hv_BBoxCol2));
    if (0 != (int((hv_BoxesInvalid.TupleSum())>0)))
    {
      GetDictTuple(hv_DLSample, "image_id", &hv_ImageId);
      throw HException(("An error has occurred during preprocessing image_id "+hv_ImageId)+HTuple(": Sample contains at least one box with zero-area, i.e. bbox_col1 >= bbox_col2 or bbox_row1 >= bbox_row2."));
    }
  }
  else
  {
    //There are no bounding boxes, hence nothing to do.
    return;
  }
  //
  //If the domain is cropped, crop bounding boxes.
  if (0 != (int(hv_DomainHandling==HTuple("crop_domain"))))
  {
    //
    //Get domain.
    GetDomain(ho_ImageRaw, &ho_DomainRaw);
    //
    //Set the size of the raw image to the domain extensions.
    SmallestRectangle1(ho_DomainRaw, &hv_DomainRow1, &hv_DomainColumn1, &hv_DomainRow2, 
        &hv_DomainColumn2);
    //The domain is always given as a pixel-precise region.
    hv_WidthRaw = (hv_DomainColumn2-hv_DomainColumn1)+1.0;
    hv_HeightRaw = (hv_DomainRow2-hv_DomainRow1)+1.0;
    //
    //Crop the bounding boxes.
    hv_Row1 = hv_BBoxRow1.TupleMax2(hv_DomainRow1-.5);
    hv_Col1 = hv_BBoxCol1.TupleMax2(hv_DomainColumn1-.5);
    hv_Row2 = hv_BBoxRow2.TupleMin2(hv_DomainRow2+.5);
    hv_Col2 = hv_BBoxCol2.TupleMin2(hv_DomainColumn2+.5);
    hv_MaskDelete = (hv_Row1.TupleGreaterEqualElem(hv_Row2)).TupleOr(hv_Col1.TupleGreaterEqualElem(hv_Col2));
    hv_MaskNewBbox = 1-hv_MaskDelete;
    //Store the preprocessed bounding box entries.
    hv_BBoxCol1New = (hv_Col1.TupleSelectMask(hv_MaskNewBbox))-hv_DomainColumn1;
    hv_BBoxCol2New = (hv_Col2.TupleSelectMask(hv_MaskNewBbox))-hv_DomainColumn1;
    hv_BBoxRow1New = (hv_Row1.TupleSelectMask(hv_MaskNewBbox))-hv_DomainRow1;
    hv_BBoxRow2New = (hv_Row2.TupleSelectMask(hv_MaskNewBbox))-hv_DomainRow1;
    hv_BBoxLabelNew = hv_BBoxLabel.TupleSelectMask(hv_MaskNewBbox);
    //
    //If we remove/select bounding boxes we also need to filter the corresponding
    //instance segmentation masks if they exist.
    filter_dl_sample_instance_segmentation_masks(hv_DLSample, hv_MaskNewBbox);
  }
  else if (0 != (int(hv_DomainHandling==HTuple("full_domain"))))
  {
    //If the entire image is used, set the variables accordingly.
    //Get the original size.
    GetImageSize(ho_ImageRaw, &hv_WidthRaw, &hv_HeightRaw);
    //Set new coordinates to input coordinates.
    hv_BBoxCol1New = hv_BBoxCol1;
    hv_BBoxCol2New = hv_BBoxCol2;
    hv_BBoxRow1New = hv_BBoxRow1;
    hv_BBoxRow2New = hv_BBoxRow2;
    hv_BBoxLabelNew = hv_BBoxLabel;
  }
  else
  {
    throw HException("Unsupported parameter value for 'domain_handling'");
  }
  //
  //Rescale the bounding boxes.
  //
  //Get required images width and height.
  //
  //Only rescale bounding boxes if the required image dimensions are not the raw dimensions.
  if (0 != (HTuple(int(hv_ImageHeight!=hv_HeightRaw)).TupleOr(int(hv_ImageWidth!=hv_WidthRaw))))
  {
    //Calculate rescaling factor.
    hv_FactorResampleWidth = (hv_ImageWidth.TupleReal())/hv_WidthRaw;
    hv_FactorResampleHeight = (hv_ImageHeight.TupleReal())/hv_HeightRaw;
    //Rescale the bounding box coordinates.
    //As we use XLD-coordinates we temporarily move the boxes by (.5,.5) for rescaling.
    //Doing so, the center of the XLD-coordinate system (-0.5,-0.5) is used
    //for scaling, hence the scaling is performed w.r.t. the pixel coordinate system.
    hv_BBoxCol1New = ((hv_BBoxCol1New+.5)*hv_FactorResampleWidth)-.5;
    hv_BBoxCol2New = ((hv_BBoxCol2New+.5)*hv_FactorResampleWidth)-.5;
    hv_BBoxRow1New = ((hv_BBoxRow1New+.5)*hv_FactorResampleHeight)-.5;
    hv_BBoxRow2New = ((hv_BBoxRow2New+.5)*hv_FactorResampleHeight)-.5;
    //
  }
  //
  //Make a final check and remove bounding boxes that have zero area.
  if (0 != (int((hv_BBoxRow1New.TupleLength())>0)))
  {
    hv_MaskDelete = (hv_BBoxRow1New.TupleGreaterEqualElem(hv_BBoxRow2New)).TupleOr(hv_BBoxCol1New.TupleGreaterEqualElem(hv_BBoxCol2New));
    hv_BBoxCol1New = hv_BBoxCol1New.TupleSelectMask(1-hv_MaskDelete);
    hv_BBoxCol2New = hv_BBoxCol2New.TupleSelectMask(1-hv_MaskDelete);
    hv_BBoxRow1New = hv_BBoxRow1New.TupleSelectMask(1-hv_MaskDelete);
    hv_BBoxRow2New = hv_BBoxRow2New.TupleSelectMask(1-hv_MaskDelete);
    hv_BBoxLabelNew = hv_BBoxLabelNew.TupleSelectMask(1-hv_MaskDelete);
    //
    //If we remove/select bounding boxes we also need to filter the corresponding
    //instance segmentation masks if they exist.
    filter_dl_sample_instance_segmentation_masks(hv_DLSample, 1-hv_MaskDelete);
  }
  //
  //Set new bounding box coordinates in the dictionary.
  SetDictTuple(hv_DLSample, "bbox_col1", hv_BBoxCol1New);
  SetDictTuple(hv_DLSample, "bbox_col2", hv_BBoxCol2New);
  SetDictTuple(hv_DLSample, "bbox_row1", hv_BBoxRow1New);
  SetDictTuple(hv_DLSample, "bbox_row2", hv_BBoxRow2New);
  SetDictTuple(hv_DLSample, "bbox_label_id", hv_BBoxLabelNew);
  //
  return;
}

// Chapter: Deep Learning / Object Detection and Instance Segmentation
// Short Description: Preprocess the bounding boxes of type 'rectangle2' for a given sample. 
void preprocess_dl_model_bbox_rect2 (HObject ho_ImageRaw, HTuple hv_DLSample, HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_DomainRaw, ho_Rectangle2XLD, ho_Rectangle2XLDSheared;

  // Local control variables
  HTuple  hv_ImageWidth, hv_ImageHeight, hv_DomainHandling;
  HTuple  hv_IgnoreDirection, hv_ClassIDsNoOrientation, hv_KeyExists;
  HTuple  hv_BBoxRow, hv_BBoxCol, hv_BBoxLength1, hv_BBoxLength2;
  HTuple  hv_BBoxPhi, hv_BBoxLabel, hv_Exception, hv_ImageId;
  HTuple  hv_ExceptionMessage, hv_BoxesInvalid, hv_DomainRow1;
  HTuple  hv_DomainColumn1, hv_DomainRow2, hv_DomainColumn2;
  HTuple  hv_WidthRaw, hv_HeightRaw, hv_MaskDelete, hv_MaskNewBbox;
  HTuple  hv_BBoxRowNew, hv_BBoxColNew, hv_BBoxLength1New;
  HTuple  hv_BBoxLength2New, hv_BBoxPhiNew, hv_BBoxLabelNew;
  HTuple  hv_ClassIDsNoOrientationIndices, hv_Index, hv_ClassIDsNoOrientationIndicesTmp;
  HTuple  hv_DirectionLength1Row, hv_DirectionLength1Col;
  HTuple  hv_DirectionLength2Row, hv_DirectionLength2Col;
  HTuple  hv_Corner1Row, hv_Corner1Col, hv_Corner2Row, hv_Corner2Col;
  HTuple  hv_FactorResampleWidth, hv_FactorResampleHeight;
  HTuple  hv_BBoxRow1, hv_BBoxCol1, hv_BBoxRow2, hv_BBoxCol2;
  HTuple  hv_BBoxRow3, hv_BBoxCol3, hv_BBoxRow4, hv_BBoxCol4;
  HTuple  hv_BBoxCol1New, hv_BBoxCol2New, hv_BBoxCol3New;
  HTuple  hv_BBoxCol4New, hv_BBoxRow1New, hv_BBoxRow2New;
  HTuple  hv_BBoxRow3New, hv_BBoxRow4New, hv_HomMat2DIdentity;
  HTuple  hv_HomMat2DScale, hv__, hv_BBoxPhiTmp, hv_PhiDelta;
  HTuple  hv_PhiDeltaNegativeIndices, hv_IndicesRot90, hv_IndicesRot180;
  HTuple  hv_IndicesRot270, hv_SwapIndices, hv_Tmp, hv_BBoxPhiNewIndices;
  HTuple  hv_PhiThreshold, hv_PhiToCorrect, hv_NumCorrections;

  //This procedure preprocesses the bounding boxes of type 'rectangle2' for a given sample.
  //
  check_dl_preprocess_param(hv_DLPreprocessParam);
  //
  //Get preprocess parameters.
  GetDictTuple(hv_DLPreprocessParam, "image_width", &hv_ImageWidth);
  GetDictTuple(hv_DLPreprocessParam, "image_height", &hv_ImageHeight);
  GetDictTuple(hv_DLPreprocessParam, "domain_handling", &hv_DomainHandling);
  //The keys 'ignore_direction' and 'class_ids_no_orientation' are optional.
  hv_IgnoreDirection = 0;
  hv_ClassIDsNoOrientation = HTuple();
  GetDictParam(hv_DLPreprocessParam, "key_exists", (HTuple("ignore_direction").Append("class_ids_no_orientation")), 
      &hv_KeyExists);
  if (0 != (HTuple(hv_KeyExists[0])))
  {
    GetDictTuple(hv_DLPreprocessParam, "ignore_direction", &hv_IgnoreDirection);
    if (0 != (int(hv_IgnoreDirection==HTuple("true"))))
    {
      hv_IgnoreDirection = 1;
    }
    else if (0 != (int(hv_IgnoreDirection==HTuple("false"))))
    {
      hv_IgnoreDirection = 0;
    }
  }
  if (0 != (HTuple(hv_KeyExists[1])))
  {
    GetDictTuple(hv_DLPreprocessParam, "class_ids_no_orientation", &hv_ClassIDsNoOrientation);
  }
  //
  //Get bounding box coordinates and labels.
  try
  {
    GetDictTuple(hv_DLSample, "bbox_row", &hv_BBoxRow);
    GetDictTuple(hv_DLSample, "bbox_col", &hv_BBoxCol);
    GetDictTuple(hv_DLSample, "bbox_length1", &hv_BBoxLength1);
    GetDictTuple(hv_DLSample, "bbox_length2", &hv_BBoxLength2);
    GetDictTuple(hv_DLSample, "bbox_phi", &hv_BBoxPhi);
    GetDictTuple(hv_DLSample, "bbox_label_id", &hv_BBoxLabel);
  }
  // catch (Exception) 
  catch (HException &HDevExpDefaultException)
  {
    HDevExpDefaultException.ToHTuple(&hv_Exception);
    GetDictTuple(hv_DLSample, "image_id", &hv_ImageId);
    if (0 != (int(HTuple(hv_Exception[0])==1302)))
    {
      hv_ExceptionMessage = "A bounding box coordinate key is missing.";
    }
    else
    {
      hv_ExceptionMessage = ((const HTuple&)hv_Exception)[2];
    }
    throw HException((("An error has occurred during preprocessing image_id "+hv_ImageId)+" when getting bounding box coordinates : ")+hv_ExceptionMessage);
  }
  //
  //Check that there are no invalid boxes.
  if (0 != (int((hv_BBoxRow.TupleLength())>0)))
  {
    hv_BoxesInvalid = ((hv_BBoxLength1.TupleEqualElem(0)).TupleSum())+((hv_BBoxLength2.TupleEqualElem(0)).TupleSum());
    if (0 != (int(hv_BoxesInvalid>0)))
    {
      GetDictTuple(hv_DLSample, "image_id", &hv_ImageId);
      throw HException(("An error has occurred during preprocessing image_id "+hv_ImageId)+HTuple(": Sample contains at least one bounding box with zero-area, i.e. bbox_length1 == 0 or bbox_length2 == 0!"));
    }
  }
  else
  {
    //There are no bounding boxes, hence nothing to do.
    return;
  }
  //
  //If the domain is cropped, crop bounding boxes.
  if (0 != (int(hv_DomainHandling==HTuple("crop_domain"))))
  {
    //
    //Get domain.
    GetDomain(ho_ImageRaw, &ho_DomainRaw);
    //
    //Set the size of the raw image to the domain extensions.
    SmallestRectangle1(ho_DomainRaw, &hv_DomainRow1, &hv_DomainColumn1, &hv_DomainRow2, 
        &hv_DomainColumn2);
    hv_WidthRaw = (hv_DomainColumn2-hv_DomainColumn1)+1;
    hv_HeightRaw = (hv_DomainRow2-hv_DomainRow1)+1;
    //
    //Crop the bounding boxes.
    //Remove the boxes with center outside of the domain.
    hv_MaskDelete = HTuple(HTuple((hv_BBoxRow.TupleLessElem(hv_DomainRow1)).TupleOr(hv_BBoxCol.TupleLessElem(hv_DomainColumn1))).TupleOr(hv_BBoxRow.TupleGreaterElem(hv_DomainRow2))).TupleOr(hv_BBoxCol.TupleGreaterElem(hv_DomainColumn2));
    hv_MaskNewBbox = 1-hv_MaskDelete;
    //Store the preprocessed bounding box entries.
    hv_BBoxRowNew = (hv_BBoxRow.TupleSelectMask(hv_MaskNewBbox))-hv_DomainRow1;
    hv_BBoxColNew = (hv_BBoxCol.TupleSelectMask(hv_MaskNewBbox))-hv_DomainColumn1;
    hv_BBoxLength1New = hv_BBoxLength1.TupleSelectMask(hv_MaskNewBbox);
    hv_BBoxLength2New = hv_BBoxLength2.TupleSelectMask(hv_MaskNewBbox);
    hv_BBoxPhiNew = hv_BBoxPhi.TupleSelectMask(hv_MaskNewBbox);
    hv_BBoxLabelNew = hv_BBoxLabel.TupleSelectMask(hv_MaskNewBbox);
    //
    //If we remove/select bounding boxes we also need to filter the corresponding
    //instance segmentation masks if they exist.
    filter_dl_sample_instance_segmentation_masks(hv_DLSample, hv_MaskNewBbox);
    //
  }
  else if (0 != (int(hv_DomainHandling==HTuple("full_domain"))))
  {
    //If the entire image is used, set the variables accordingly.
    //Get the original size.
    GetImageSize(ho_ImageRaw, &hv_WidthRaw, &hv_HeightRaw);
    //Set new coordinates to input coordinates.
    hv_BBoxRowNew = hv_BBoxRow;
    hv_BBoxColNew = hv_BBoxCol;
    hv_BBoxLength1New = hv_BBoxLength1;
    hv_BBoxLength2New = hv_BBoxLength2;
    hv_BBoxPhiNew = hv_BBoxPhi;
    hv_BBoxLabelNew = hv_BBoxLabel;
  }
  else
  {
    throw HException("Unsupported parameter value for 'domain_handling'");
  }
  //
  //Generate smallest enclosing axis-aligned bounding box for classes in ClassIDsNoOrientation.
  hv_ClassIDsNoOrientationIndices = HTuple();
  {
  HTuple end_val98 = (hv_ClassIDsNoOrientation.TupleLength())-1;
  HTuple step_val98 = 1;
  for (hv_Index=0; hv_Index.Continue(end_val98, step_val98); hv_Index += step_val98)
  {
    hv_ClassIDsNoOrientationIndicesTmp = (hv_BBoxLabelNew.TupleEqualElem(HTuple(hv_ClassIDsNoOrientation[hv_Index]))).TupleFind(1);
    if (0 != (int(hv_ClassIDsNoOrientationIndicesTmp!=-1)))
    {
      hv_ClassIDsNoOrientationIndices = hv_ClassIDsNoOrientationIndices.TupleConcat(hv_ClassIDsNoOrientationIndicesTmp);
    }
  }
  }
  if (0 != (int((hv_ClassIDsNoOrientationIndices.TupleLength())>0)))
  {
    //Calculate length1 and length2 using position of corners.
    hv_DirectionLength1Row = -(HTuple(hv_BBoxPhiNew[hv_ClassIDsNoOrientationIndices]).TupleSin());
    hv_DirectionLength1Col = HTuple(hv_BBoxPhiNew[hv_ClassIDsNoOrientationIndices]).TupleCos();
    hv_DirectionLength2Row = -hv_DirectionLength1Col;
    hv_DirectionLength2Col = hv_DirectionLength1Row;
    hv_Corner1Row = (HTuple(hv_BBoxLength1New[hv_ClassIDsNoOrientationIndices])*hv_DirectionLength1Row)+(HTuple(hv_BBoxLength2New[hv_ClassIDsNoOrientationIndices])*hv_DirectionLength2Row);
    hv_Corner1Col = (HTuple(hv_BBoxLength1New[hv_ClassIDsNoOrientationIndices])*hv_DirectionLength1Col)+(HTuple(hv_BBoxLength2New[hv_ClassIDsNoOrientationIndices])*hv_DirectionLength2Col);
    hv_Corner2Row = (HTuple(hv_BBoxLength1New[hv_ClassIDsNoOrientationIndices])*hv_DirectionLength1Row)-(HTuple(hv_BBoxLength2New[hv_ClassIDsNoOrientationIndices])*hv_DirectionLength2Row);
    hv_Corner2Col = (HTuple(hv_BBoxLength1New[hv_ClassIDsNoOrientationIndices])*hv_DirectionLength1Col)-(HTuple(hv_BBoxLength2New[hv_ClassIDsNoOrientationIndices])*hv_DirectionLength2Col);
    //
    hv_BBoxPhiNew[hv_ClassIDsNoOrientationIndices] = 0.0;
    hv_BBoxLength1New[hv_ClassIDsNoOrientationIndices] = (hv_Corner1Col.TupleAbs()).TupleMax2(hv_Corner2Col.TupleAbs());
    hv_BBoxLength2New[hv_ClassIDsNoOrientationIndices] = (hv_Corner1Row.TupleAbs()).TupleMax2(hv_Corner2Row.TupleAbs());
  }
  //
  //Rescale bounding boxes.
  //
  //Get required images width and height.
  //
  //Only rescale bounding boxes if the required image dimensions are not the raw dimensions.
  if (0 != (HTuple(int(hv_ImageHeight!=hv_HeightRaw)).TupleOr(int(hv_ImageWidth!=hv_WidthRaw))))
  {
    //
    //Calculate rescaling factor.
    calculate_dl_image_zoom_factors(hv_WidthRaw, hv_HeightRaw, hv_ImageWidth, hv_ImageHeight, 
        hv_DLPreprocessParam, &hv_FactorResampleWidth, &hv_FactorResampleHeight);
    //
    if (0 != (HTuple(int(hv_FactorResampleHeight!=hv_FactorResampleWidth)).TupleAnd(int((hv_BBoxRowNew.TupleLength())>0))))
    {
      //In order to preserve the correct orientation we have to transform the points individually.
      //Get the coordinates of the four corner points.
      convert_rect2_5to8param(hv_BBoxRowNew, hv_BBoxColNew, hv_BBoxLength1New, hv_BBoxLength2New, 
          hv_BBoxPhiNew, &hv_BBoxRow1, &hv_BBoxCol1, &hv_BBoxRow2, &hv_BBoxCol2, 
          &hv_BBoxRow3, &hv_BBoxCol3, &hv_BBoxRow4, &hv_BBoxCol4);
      //
      //Rescale the coordinates.
      hv_BBoxCol1New = hv_BBoxCol1*hv_FactorResampleWidth;
      hv_BBoxCol2New = hv_BBoxCol2*hv_FactorResampleWidth;
      hv_BBoxCol3New = hv_BBoxCol3*hv_FactorResampleWidth;
      hv_BBoxCol4New = hv_BBoxCol4*hv_FactorResampleWidth;
      hv_BBoxRow1New = hv_BBoxRow1*hv_FactorResampleHeight;
      hv_BBoxRow2New = hv_BBoxRow2*hv_FactorResampleHeight;
      hv_BBoxRow3New = hv_BBoxRow3*hv_FactorResampleHeight;
      hv_BBoxRow4New = hv_BBoxRow4*hv_FactorResampleHeight;
      //
      //The rectangles will get sheared, that is why new rectangles have to be found.
      //Generate homography to scale rectangles.
      HomMat2dIdentity(&hv_HomMat2DIdentity);
      HomMat2dScale(hv_HomMat2DIdentity, hv_FactorResampleHeight, hv_FactorResampleWidth, 
          0, 0, &hv_HomMat2DScale);
      //Generate XLD contours for the rectangles.
      GenRectangle2ContourXld(&ho_Rectangle2XLD, hv_BBoxRowNew, hv_BBoxColNew, hv_BBoxPhiNew, 
          hv_BBoxLength1New, hv_BBoxLength2New);
      //Scale the XLD contours --> results in sheared regions.
      AffineTransContourXld(ho_Rectangle2XLD, &ho_Rectangle2XLDSheared, hv_HomMat2DScale);
      SmallestRectangle2Xld(ho_Rectangle2XLDSheared, &hv_BBoxRowNew, &hv_BBoxColNew, 
          &hv_BBoxPhiNew, &hv_BBoxLength1New, &hv_BBoxLength2New);
      //
      //smallest_rectangle2_xld might change the orientation of the bounding box.
      //Hence, take the orientation that is closest to the one obtained out of the 4 corner points.
      convert_rect2_8to5param(hv_BBoxRow1New, hv_BBoxCol1New, hv_BBoxRow2New, hv_BBoxCol2New, 
          hv_BBoxRow3New, hv_BBoxCol3New, hv_BBoxRow4New, hv_BBoxCol4New, hv_IgnoreDirection, 
          &hv__, &hv__, &hv__, &hv__, &hv_BBoxPhiTmp);
      hv_PhiDelta = (hv_BBoxPhiTmp-hv_BBoxPhiNew).TupleFmod(HTuple(360).TupleRad());
      //Guarantee that angles are positive.
      hv_PhiDeltaNegativeIndices = (hv_PhiDelta.TupleLessElem(0.0)).TupleFind(1);
      if (0 != (int(hv_PhiDeltaNegativeIndices!=-1)))
      {
        hv_PhiDelta[hv_PhiDeltaNegativeIndices] = HTuple(hv_PhiDelta[hv_PhiDeltaNegativeIndices])+(HTuple(360).TupleRad());
      }
      hv_IndicesRot90 = HTuple((hv_PhiDelta.TupleGreaterElem(HTuple(45).TupleRad())).TupleAnd(hv_PhiDelta.TupleLessEqualElem(HTuple(135).TupleRad()))).TupleFind(1);
      hv_IndicesRot180 = HTuple((hv_PhiDelta.TupleGreaterElem(HTuple(135).TupleRad())).TupleAnd(hv_PhiDelta.TupleLessEqualElem(HTuple(225).TupleRad()))).TupleFind(1);
      hv_IndicesRot270 = HTuple((hv_PhiDelta.TupleGreaterElem(HTuple(225).TupleRad())).TupleAnd(hv_PhiDelta.TupleLessEqualElem(HTuple(315).TupleRad()))).TupleFind(1);
      hv_SwapIndices = HTuple();
      if (0 != (int(hv_IndicesRot90!=-1)))
      {
        hv_BBoxPhiNew[hv_IndicesRot90] = HTuple(hv_BBoxPhiNew[hv_IndicesRot90])+(HTuple(90).TupleRad());
        hv_SwapIndices = hv_SwapIndices.TupleConcat(hv_IndicesRot90);
      }
      if (0 != (int(hv_IndicesRot180!=-1)))
      {
        hv_BBoxPhiNew[hv_IndicesRot180] = HTuple(hv_BBoxPhiNew[hv_IndicesRot180])+(HTuple(180).TupleRad());
      }
      if (0 != (int(hv_IndicesRot270!=-1)))
      {
        hv_BBoxPhiNew[hv_IndicesRot270] = HTuple(hv_BBoxPhiNew[hv_IndicesRot270])+(HTuple(270).TupleRad());
        hv_SwapIndices = hv_SwapIndices.TupleConcat(hv_IndicesRot270);
      }
      if (0 != (int(hv_SwapIndices!=HTuple())))
      {
        hv_Tmp = HTuple(hv_BBoxLength1New[hv_SwapIndices]);
        hv_BBoxLength1New[hv_SwapIndices] = HTuple(hv_BBoxLength2New[hv_SwapIndices]);
        hv_BBoxLength2New[hv_SwapIndices] = hv_Tmp;
      }
      //Change angles such that they lie in the range (-180��, 180��].
      hv_BBoxPhiNewIndices = (hv_BBoxPhiNew.TupleGreaterElem(HTuple(180).TupleRad())).TupleFind(1);
      if (0 != (int(hv_BBoxPhiNewIndices!=-1)))
      {
        hv_BBoxPhiNew[hv_BBoxPhiNewIndices] = HTuple(hv_BBoxPhiNew[hv_BBoxPhiNewIndices])-(HTuple(360).TupleRad());
      }
      //
    }
    else
    {
      hv_BBoxColNew = hv_BBoxColNew*hv_FactorResampleWidth;
      hv_BBoxRowNew = hv_BBoxRowNew*hv_FactorResampleWidth;
      hv_BBoxLength1New = hv_BBoxLength1New*hv_FactorResampleWidth;
      hv_BBoxLength2New = hv_BBoxLength2New*hv_FactorResampleWidth;
      //Phi stays the same.
    }
    //
  }
  //
  //Adapt the bounding box angles such that they are within the correct range,
  //which is (-180��,180��] for 'ignore_direction'==false and (-90��,90��] else.
  hv_PhiThreshold = (HTuple(180).TupleRad())-(hv_IgnoreDirection*(HTuple(90).TupleRad()));
  hv_PhiDelta = 2*hv_PhiThreshold;
  //Correct angles that are too large.
  hv_PhiToCorrect = (hv_BBoxPhiNew.TupleGreaterElem(hv_PhiThreshold)).TupleFind(1);
  if (0 != (HTuple(int(hv_PhiToCorrect!=-1)).TupleAnd(int(hv_PhiToCorrect!=HTuple()))))
  {
    hv_NumCorrections = (((HTuple(hv_BBoxPhiNew[hv_PhiToCorrect])-hv_PhiThreshold)/hv_PhiDelta).TupleInt())+1;
    hv_BBoxPhiNew[hv_PhiToCorrect] = HTuple(hv_BBoxPhiNew[hv_PhiToCorrect])-(hv_NumCorrections*hv_PhiDelta);
  }
  //Correct angles that are too small.
  hv_PhiToCorrect = (hv_BBoxPhiNew.TupleLessEqualElem(-hv_PhiThreshold)).TupleFind(1);
  if (0 != (HTuple(int(hv_PhiToCorrect!=-1)).TupleAnd(int(hv_PhiToCorrect!=HTuple()))))
  {
    hv_NumCorrections = ((((HTuple(hv_BBoxPhiNew[hv_PhiToCorrect])+hv_PhiThreshold).TupleAbs())/hv_PhiDelta).TupleInt())+1;
    hv_BBoxPhiNew[hv_PhiToCorrect] = HTuple(hv_BBoxPhiNew[hv_PhiToCorrect])+(hv_NumCorrections*hv_PhiDelta);
  }
  //
  //Check that there are no invalid boxes.
  if (0 != (int((hv_BBoxRowNew.TupleLength())>0)))
  {
    hv_BoxesInvalid = ((hv_BBoxLength1New.TupleEqualElem(0)).TupleSum())+((hv_BBoxLength2New.TupleEqualElem(0)).TupleSum());
    if (0 != (int(hv_BoxesInvalid>0)))
    {
      GetDictTuple(hv_DLSample, "image_id", &hv_ImageId);
      throw HException(("An error has occurred during preprocessing image_id "+hv_ImageId)+HTuple(": Sample contains at least one box with zero-area, i.e. bbox_length1 == 0 or bbox_length2 == 0!"));
    }
  }
  SetDictTuple(hv_DLSample, "bbox_row", hv_BBoxRowNew);
  SetDictTuple(hv_DLSample, "bbox_col", hv_BBoxColNew);
  SetDictTuple(hv_DLSample, "bbox_length1", hv_BBoxLength1New);
  SetDictTuple(hv_DLSample, "bbox_length2", hv_BBoxLength2New);
  SetDictTuple(hv_DLSample, "bbox_phi", hv_BBoxPhiNew);
  SetDictTuple(hv_DLSample, "bbox_label_id", hv_BBoxLabelNew);
  //
  return;

}

// Chapter: Deep Learning / Model
// Short Description: Preprocess images for deep-learning-based training and inference. 
void preprocess_dl_model_images (HObject ho_Images, HObject *ho_ImagesPreprocessed, 
    HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_PreservedDomains, ho_ImageSelected;
  HObject  ho_DomainSelected, ho_ImagesScaled, ho_ImageScaled;
  HObject  ho_Channel, ho_ChannelScaled, ho_ThreeChannelImage;
  HObject  ho_SingleChannelImage;

  // Local control variables
  HTuple  hv_ImageWidth, hv_ImageHeight, hv_ImageNumChannels;
  HTuple  hv_ImageRangeMin, hv_ImageRangeMax, hv_DomainHandling;
  HTuple  hv_NormalizationType, hv_ModelType, hv_NumImages;
  HTuple  hv_Type, hv_NumMatches, hv_InputNumChannels, hv_OutputNumChannels;
  HTuple  hv_NumChannels1, hv_NumChannels3, hv_AreInputNumChannels1;
  HTuple  hv_AreInputNumChannels3, hv_AreInputNumChannels1Or3;
  HTuple  hv_ValidNumChannels, hv_ValidNumChannelsText, hv_PreserveDomain;
  HTuple  hv_Row1, hv_Column1, hv_Row2, hv_Column2, hv_UniqRow1;
  HTuple  hv_UniqColumn1, hv_UniqRow2, hv_UniqColumn2, hv_RectangleIndex;
  HTuple  hv_OriginalWidth, hv_OriginalHeight, hv_UniqWidth;
  HTuple  hv_UniqHeight, hv_ScaleWidth, hv_ScaleHeight, hv_ScaleIndex;
  HTuple  hv_ImageIndex, hv_NumChannels, hv_ChannelIndex;
  HTuple  hv_Min, hv_Max, hv_Range, hv_Scale, hv_Shift, hv_MeanValues;
  HTuple  hv_DeviationValues, hv_UseDefaultNormalizationValues;
  HTuple  hv_Exception, hv_Indices, hv_RescaleRange, hv_CurrentNumChannels;
  HTuple  hv_DiffNumChannelsIndices, hv_Index, hv_DiffNumChannelsIndex;
  HTuple  hv_NumDomains, hv_DomainIndex;

  //
  //This procedure preprocesses the provided Images according to the parameters in
  //the dictionary DLPreprocessParam. Note that depending on the images, additional
  //preprocessing steps might be beneficial.
  //
  //Validate the preprocessing parameters.
  check_dl_preprocess_param(hv_DLPreprocessParam);
  //
  //Get the preprocessing parameters.
  GetDictTuple(hv_DLPreprocessParam, "image_width", &hv_ImageWidth);
  GetDictTuple(hv_DLPreprocessParam, "image_height", &hv_ImageHeight);
  GetDictTuple(hv_DLPreprocessParam, "image_num_channels", &hv_ImageNumChannels);
  GetDictTuple(hv_DLPreprocessParam, "image_range_min", &hv_ImageRangeMin);
  GetDictTuple(hv_DLPreprocessParam, "image_range_max", &hv_ImageRangeMax);
  GetDictTuple(hv_DLPreprocessParam, "domain_handling", &hv_DomainHandling);
  GetDictTuple(hv_DLPreprocessParam, "normalization_type", &hv_NormalizationType);
  GetDictTuple(hv_DLPreprocessParam, "model_type", &hv_ModelType);
  //
  //Validate the type of the input images.
  CountObj(ho_Images, &hv_NumImages);
  if (0 != (int(hv_NumImages==0)))
  {
    throw HException("Please provide some images to preprocess.");
  }
  GetImageType(ho_Images, &hv_Type);
  TupleRegexpTest(hv_Type, "byte|int|real", &hv_NumMatches);
  if (0 != (int(hv_NumMatches!=hv_NumImages)))
  {
    throw HException(HTuple("Please provide only images of type 'byte', 'int1', 'int2', 'uint2', 'int4', 'int8', or 'real'."));
  }
  //
  //Handle ocr_recognition models.
  if (0 != (int(hv_ModelType==HTuple("ocr_recognition"))))
  {
    preprocess_dl_model_images_ocr_recognition(ho_Images, &(*ho_ImagesPreprocessed), 
        hv_DLPreprocessParam);
    return;
  }
  //
  //Handle ocr_detection models.
  if (0 != (int(hv_ModelType==HTuple("ocr_detection"))))
  {
    preprocess_dl_model_images_ocr_detection(ho_Images, &(*ho_ImagesPreprocessed), 
        hv_DLPreprocessParam);
    return;
  }
  //
  //Validate the number channels of the input images.
  CountChannels(ho_Images, &hv_InputNumChannels);
  hv_OutputNumChannels = HTuple(hv_NumImages,hv_ImageNumChannels);
  //Only for 'image_num_channels' 1 and 3 combinations of 1- and 3-channel images are allowed.
  if (0 != (HTuple(int(hv_ImageNumChannels==1)).TupleOr(int(hv_ImageNumChannels==3))))
  {
    hv_NumChannels1 = HTuple(hv_NumImages,1);
    hv_NumChannels3 = HTuple(hv_NumImages,3);
    hv_AreInputNumChannels1 = hv_InputNumChannels.TupleEqualElem(hv_NumChannels1);
    hv_AreInputNumChannels3 = hv_InputNumChannels.TupleEqualElem(hv_NumChannels3);
    hv_AreInputNumChannels1Or3 = hv_AreInputNumChannels1+hv_AreInputNumChannels3;
    hv_ValidNumChannels = int(hv_AreInputNumChannels1Or3==hv_NumChannels1);
    hv_ValidNumChannelsText = "Valid numbers of channels for the specified model are 1 or 3.";
  }
  else
  {
    hv_ValidNumChannels = int(hv_InputNumChannels==hv_OutputNumChannels);
    hv_ValidNumChannelsText = ("Valid number of channels for the specified model is "+hv_ImageNumChannels)+".";
  }
  if (0 != (hv_ValidNumChannels.TupleNot()))
  {
    throw HException("Please provide images with a valid number of channels. "+hv_ValidNumChannelsText);
  }
  //Preprocess the images.
  //
  //For models of type '3d_gripping_point_detection', the preprocessing steps need to be performed on full
  //domain images while the domains are preserved and set back into the images after the preprocessing.
  hv_PreserveDomain = 0;
  if (0 != (HTuple(int(hv_ModelType==HTuple("3d_gripping_point_detection"))).TupleAnd(HTuple(int(hv_DomainHandling==HTuple("crop_domain"))).TupleOr(int(hv_DomainHandling==HTuple("keep_domain"))))))
  {
    hv_PreserveDomain = 1;
    GetDomain(ho_Images, &ho_PreservedDomains);
    FullDomain(ho_Images, &ho_Images);
  }
  //
  //Apply the domain to the images.
  if (0 != (int(hv_DomainHandling==HTuple("full_domain"))))
  {
    FullDomain(ho_Images, &ho_Images);
  }
  else if (0 != (int(hv_DomainHandling==HTuple("crop_domain"))))
  {
    if (0 != hv_PreserveDomain)
    {
      //In case of preserved domain, the crop is performed with the smallest rectangle of the
      //domain to avoid out of domain pixels being set to 0.
      SmallestRectangle1(ho_PreservedDomains, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);
      TupleUniq(hv_Row1, &hv_UniqRow1);
      TupleUniq(hv_Column1, &hv_UniqColumn1);
      TupleUniq(hv_Row2, &hv_UniqRow2);
      TupleUniq(hv_Column2, &hv_UniqColumn2);
      if (0 != (HTuple(HTuple(HTuple(int((hv_UniqRow1.TupleLength())==1)).TupleAnd(int((hv_UniqColumn1.TupleLength())==1))).TupleAnd(int((hv_UniqRow2.TupleLength())==1))).TupleAnd(int((hv_UniqColumn2.TupleLength())==1))))
      {
        CropRectangle1(ho_Images, &ho_Images, hv_UniqRow1, hv_UniqColumn1, hv_UniqRow2, 
            hv_UniqColumn2);
        MoveRegion(ho_PreservedDomains, &ho_PreservedDomains, -hv_UniqRow1, -hv_UniqColumn1);
      }
      else
      {
        {
        HTuple end_val87 = (hv_Row1.TupleLength())-1;
        HTuple step_val87 = 1;
        for (hv_RectangleIndex=0; hv_RectangleIndex.Continue(end_val87, step_val87); hv_RectangleIndex += step_val87)
        {
          SelectObj(ho_Images, &ho_ImageSelected, hv_RectangleIndex+1);
          CropRectangle1(ho_ImageSelected, &ho_ImageSelected, HTuple(hv_Row1[hv_RectangleIndex]), 
              HTuple(hv_Column1[hv_RectangleIndex]), HTuple(hv_Row2[hv_RectangleIndex]), 
              HTuple(hv_Column2[hv_RectangleIndex]));
          ReplaceObj(ho_Images, ho_ImageSelected, &ho_Images, hv_RectangleIndex+1);
          SelectObj(ho_PreservedDomains, &ho_DomainSelected, hv_RectangleIndex+1);
          MoveRegion(ho_DomainSelected, &ho_DomainSelected, -HTuple(hv_Row1[hv_RectangleIndex]), 
              -HTuple(hv_Column1[hv_RectangleIndex]));
          ReplaceObj(ho_PreservedDomains, ho_DomainSelected, &ho_PreservedDomains, 
              hv_RectangleIndex+1);
        }
        }
      }
    }
    else
    {
      CropDomain(ho_Images, &ho_Images);
    }
  }
  else if (0 != (HTuple(int(hv_DomainHandling==HTuple("keep_domain"))).TupleAnd(HTuple(int(hv_ModelType==HTuple("anomaly_detection"))).TupleOr(int(hv_ModelType==HTuple("3d_gripping_point_detection"))))))
  {
    //The option 'keep_domain' is only supported for models of 'type' = 'anomaly_detection' or '3d_gripping_point_detection'.
  }
  else
  {
    throw HException("Unsupported parameter value for 'domain_handling'.");
  }
  //
  //Zoom preserved domains before zooming the images.
  if (0 != hv_PreserveDomain)
  {
    GetImageSize(ho_Images, &hv_OriginalWidth, &hv_OriginalHeight);
    TupleUniq(hv_OriginalWidth, &hv_UniqWidth);
    TupleUniq(hv_OriginalHeight, &hv_UniqHeight);
    if (0 != (HTuple(int((hv_UniqWidth.TupleLength())==1)).TupleAnd(int((hv_UniqHeight.TupleLength())==1))))
    {
      hv_ScaleWidth = hv_ImageWidth/(hv_UniqWidth.TupleReal());
      hv_ScaleHeight = hv_ImageHeight/(hv_UniqHeight.TupleReal());
      ZoomRegion(ho_PreservedDomains, &ho_PreservedDomains, hv_ScaleWidth, hv_ScaleHeight);
    }
    else
    {
      hv_ScaleWidth = hv_ImageWidth/(hv_OriginalWidth.TupleReal());
      hv_ScaleHeight = hv_ImageHeight/(hv_OriginalHeight.TupleReal());
      {
      HTuple end_val117 = (hv_ScaleWidth.TupleLength())-1;
      HTuple step_val117 = 1;
      for (hv_ScaleIndex=0; hv_ScaleIndex.Continue(end_val117, step_val117); hv_ScaleIndex += step_val117)
      {
        SelectObj(ho_PreservedDomains, &ho_DomainSelected, hv_ScaleIndex+1);
        ZoomRegion(ho_DomainSelected, &ho_DomainSelected, HTuple(hv_ScaleWidth[hv_ScaleIndex]), 
            HTuple(hv_ScaleHeight[hv_ScaleIndex]));
        ReplaceObj(ho_PreservedDomains, ho_DomainSelected, &ho_PreservedDomains, 
            hv_ScaleIndex+1);
      }
      }
    }
  }
  //
  //Convert the images to real and zoom the images.
  //Zoom first to speed up if all image types are supported by zoom_image_size.
  if (0 != (int((hv_Type.TupleRegexpTest("int1|int4|int8"))==0)))
  {
    ZoomImageSize(ho_Images, &ho_Images, hv_ImageWidth, hv_ImageHeight, "constant");
    ConvertImageType(ho_Images, &ho_Images, "real");
  }
  else
  {
    ConvertImageType(ho_Images, &ho_Images, "real");
    ZoomImageSize(ho_Images, &ho_Images, hv_ImageWidth, hv_ImageHeight, "constant");
  }
  //
  if (0 != (int(hv_NormalizationType==HTuple("all_channels"))))
  {
    //Scale for each image the gray values of all channels to ImageRangeMin-ImageRangeMax.
    GenEmptyObj(&ho_ImagesScaled);
    {
    HTuple end_val138 = hv_NumImages;
    HTuple step_val138 = 1;
    for (hv_ImageIndex=1; hv_ImageIndex.Continue(end_val138, step_val138); hv_ImageIndex += step_val138)
    {
      SelectObj(ho_Images, &ho_ImageSelected, hv_ImageIndex);
      CountChannels(ho_ImageSelected, &hv_NumChannels);
      GenEmptyObj(&ho_ImageScaled);
      {
      HTuple end_val142 = hv_NumChannels;
      HTuple step_val142 = 1;
      for (hv_ChannelIndex=1; hv_ChannelIndex.Continue(end_val142, step_val142); hv_ChannelIndex += step_val142)
      {
        AccessChannel(ho_ImageSelected, &ho_Channel, hv_ChannelIndex);
        MinMaxGray(ho_Channel, ho_Channel, 0, &hv_Min, &hv_Max, &hv_Range);
        if (0 != (int((hv_Max-hv_Min)==0)))
        {
          hv_Scale = 1;
        }
        else
        {
          hv_Scale = (hv_ImageRangeMax-hv_ImageRangeMin)/(hv_Max-hv_Min);
        }
        hv_Shift = ((-hv_Scale)*hv_Min)+hv_ImageRangeMin;
        ScaleImage(ho_Channel, &ho_ChannelScaled, hv_Scale, hv_Shift);
        AppendChannel(ho_ImageScaled, ho_ChannelScaled, &ho_ImageScaled);
      }
      }
      ConcatObj(ho_ImagesScaled, ho_ImageScaled, &ho_ImagesScaled);
    }
    }
    ho_Images = ho_ImagesScaled;
  }
  else if (0 != (int(hv_NormalizationType==HTuple("first_channel"))))
  {
    //Scale for each image the gray values of first channel to ImageRangeMin-ImageRangeMax.
    GenEmptyObj(&ho_ImagesScaled);
    {
    HTuple end_val160 = hv_NumImages;
    HTuple step_val160 = 1;
    for (hv_ImageIndex=1; hv_ImageIndex.Continue(end_val160, step_val160); hv_ImageIndex += step_val160)
    {
      SelectObj(ho_Images, &ho_ImageSelected, hv_ImageIndex);
      MinMaxGray(ho_ImageSelected, ho_ImageSelected, 0, &hv_Min, &hv_Max, &hv_Range);
      if (0 != (int((hv_Max-hv_Min)==0)))
      {
        hv_Scale = 1;
      }
      else
      {
        hv_Scale = (hv_ImageRangeMax-hv_ImageRangeMin)/(hv_Max-hv_Min);
      }
      hv_Shift = ((-hv_Scale)*hv_Min)+hv_ImageRangeMin;
      ScaleImage(ho_ImageSelected, &ho_ImageSelected, hv_Scale, hv_Shift);
      ConcatObj(ho_ImagesScaled, ho_ImageSelected, &ho_ImagesScaled);
    }
    }
    ho_Images = ho_ImagesScaled;
  }
  else if (0 != (int(hv_NormalizationType==HTuple("constant_values"))))
  {
    //Scale for each image the gray values of all channels to the corresponding channel DeviationValues[].
    try
    {
      GetDictTuple(hv_DLPreprocessParam, "mean_values_normalization", &hv_MeanValues);
      GetDictTuple(hv_DLPreprocessParam, "deviation_values_normalization", &hv_DeviationValues);
      hv_UseDefaultNormalizationValues = 0;
    }
    // catch (Exception) 
    catch (HException &HDevExpDefaultException)
    {
      HDevExpDefaultException.ToHTuple(&hv_Exception);
      hv_MeanValues.Clear();
      hv_MeanValues[0] = 123.675;
      hv_MeanValues[1] = 116.28;
      hv_MeanValues[2] = 103.53;
      hv_DeviationValues.Clear();
      hv_DeviationValues[0] = 58.395;
      hv_DeviationValues[1] = 57.12;
      hv_DeviationValues[2] = 57.375;
      hv_UseDefaultNormalizationValues = 1;
    }
    GenEmptyObj(&ho_ImagesScaled);
    {
    HTuple end_val185 = hv_NumImages;
    HTuple step_val185 = 1;
    for (hv_ImageIndex=1; hv_ImageIndex.Continue(end_val185, step_val185); hv_ImageIndex += step_val185)
    {
      SelectObj(ho_Images, &ho_ImageSelected, hv_ImageIndex);
      CountChannels(ho_ImageSelected, &hv_NumChannels);
      //Ensure that the number of channels is equal |DeviationValues| and |MeanValues|
      if (0 != hv_UseDefaultNormalizationValues)
      {
        if (0 != (int(hv_NumChannels==1)))
        {
          Compose3(ho_ImageSelected, ho_ImageSelected, ho_ImageSelected, &ho_ImageSelected
              );
          CountChannels(ho_ImageSelected, &hv_NumChannels);
        }
        else if (0 != (int(hv_NumChannels!=3)))
        {
          throw HException("Using default values for normalization type 'constant_values' is allowed only for 1- and 3-channel images.");
        }
      }
      if (0 != (HTuple(int((hv_MeanValues.TupleLength())!=hv_NumChannels)).TupleOr(int((hv_DeviationValues.TupleLength())!=hv_NumChannels))))
      {
        throw HException("The length of mean and deviation values for normalization type 'constant_values' have to be the same size as the number of channels of the image.");
      }
      GenEmptyObj(&ho_ImageScaled);
      {
      HTuple end_val201 = hv_NumChannels;
      HTuple step_val201 = 1;
      for (hv_ChannelIndex=1; hv_ChannelIndex.Continue(end_val201, step_val201); hv_ChannelIndex += step_val201)
      {
        AccessChannel(ho_ImageSelected, &ho_Channel, hv_ChannelIndex);
        hv_Scale = 1.0/HTuple(hv_DeviationValues[hv_ChannelIndex-1]);
        hv_Shift = (-hv_Scale)*HTuple(hv_MeanValues[hv_ChannelIndex-1]);
        ScaleImage(ho_Channel, &ho_ChannelScaled, hv_Scale, hv_Shift);
        AppendChannel(ho_ImageScaled, ho_ChannelScaled, &ho_ImageScaled);
      }
      }
      ConcatObj(ho_ImagesScaled, ho_ImageScaled, &ho_ImagesScaled);
    }
    }
    ho_Images = ho_ImagesScaled;
  }
  else if (0 != (int(hv_NormalizationType==HTuple("none"))))
  {
    TupleFind(hv_Type, "byte", &hv_Indices);
    if (0 != (int(hv_Indices!=-1)))
    {
      //Shift the gray values from [0-255] to the expected range for byte images.
      hv_RescaleRange = (hv_ImageRangeMax-hv_ImageRangeMin)/255.0;
      SelectObj(ho_Images, &ho_ImageSelected, hv_Indices+1);
      ScaleImage(ho_ImageSelected, &ho_ImageSelected, hv_RescaleRange, hv_ImageRangeMin);
      ReplaceObj(ho_Images, ho_ImageSelected, &ho_Images, hv_Indices+1);
    }
  }
  else if (0 != (int(hv_NormalizationType!=HTuple("none"))))
  {
    throw HException("Unsupported parameter value for 'normalization_type'");
  }
  //
  //Ensure that the number of channels of the resulting images is consistent with the
  //number of channels of the given model. The only exceptions that are adapted below
  //are combinations of 1- and 3-channel images if ImageNumChannels is either 1 or 3.
  if (0 != (HTuple(int(hv_ImageNumChannels==1)).TupleOr(int(hv_ImageNumChannels==3))))
  {
    CountChannels(ho_Images, &hv_CurrentNumChannels);
    TupleFind(hv_CurrentNumChannels.TupleNotEqualElem(hv_OutputNumChannels), 1, &hv_DiffNumChannelsIndices);
    if (0 != (int(hv_DiffNumChannelsIndices!=-1)))
    {
      {
      HTuple end_val231 = (hv_DiffNumChannelsIndices.TupleLength())-1;
      HTuple step_val231 = 1;
      for (hv_Index=0; hv_Index.Continue(end_val231, step_val231); hv_Index += step_val231)
      {
        hv_DiffNumChannelsIndex = HTuple(hv_DiffNumChannelsIndices[hv_Index]);
        hv_ImageIndex = hv_DiffNumChannelsIndex+1;
        hv_NumChannels = HTuple(hv_CurrentNumChannels[hv_ImageIndex-1]);
        SelectObj(ho_Images, &ho_ImageSelected, hv_ImageIndex);
        if (0 != (HTuple(int(hv_NumChannels==1)).TupleAnd(int(hv_ImageNumChannels==3))))
        {
          //Conversion from 1- to 3-channel image required
          Compose3(ho_ImageSelected, ho_ImageSelected, ho_ImageSelected, &ho_ThreeChannelImage
              );
          ReplaceObj(ho_Images, ho_ThreeChannelImage, &ho_Images, hv_ImageIndex);
        }
        else if (0 != (HTuple(int(hv_NumChannels==3)).TupleAnd(int(hv_ImageNumChannels==1))))
        {
          //Conversion from 3- to 1-channel image required
          Rgb1ToGray(ho_ImageSelected, &ho_SingleChannelImage);
          ReplaceObj(ho_Images, ho_SingleChannelImage, &ho_Images, hv_ImageIndex);
        }
        else
        {
          throw HException(((("Unexpected error adapting the number of channels. The number of channels of the resulting image is "+hv_NumChannels)+HTuple(", but the number of channels of the model is "))+hv_ImageNumChannels)+".");
        }
      }
      }
    }
  }
  //
  //In case the image domains were preserved, they need to be set back into the images.
  if (0 != hv_PreserveDomain)
  {
    CountObj(ho_PreservedDomains, &hv_NumDomains);
    {
    HTuple end_val254 = hv_NumDomains;
    HTuple step_val254 = 1;
    for (hv_DomainIndex=1; hv_DomainIndex.Continue(end_val254, step_val254); hv_DomainIndex += step_val254)
    {
      SelectObj(ho_Images, &ho_ImageSelected, hv_DomainIndex);
      SelectObj(ho_PreservedDomains, &ho_DomainSelected, hv_DomainIndex);
      ReduceDomain(ho_ImageSelected, ho_DomainSelected, &ho_ImageSelected);
      ReplaceObj(ho_Images, ho_ImageSelected, &ho_Images, hv_DomainIndex);
    }
    }
  }
  //
  //Write preprocessed images to output variable.
  (*ho_ImagesPreprocessed) = ho_Images;
  //
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Preprocess images for deep-learning-based training and inference of Deep OCR detection models. 
void preprocess_dl_model_images_ocr_detection (HObject ho_Images, HObject *ho_ImagesPreprocessed, 
    HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_Image, ho_ImageScaled, ho_Channel;
  HObject  ho_ChannelScaled, ho_ImageG, ho_ImageB;

  // Local control variables
  HTuple  hv_ImageWidth, hv_ImageHeight, hv_ImageNumChannels;
  HTuple  hv_ImageRangeMin, hv_ImageRangeMax, hv_DomainHandling;
  HTuple  hv_NormalizationType, hv_ModelType, hv_NumImages;
  HTuple  hv_NumChannels, hv_ImageTypes, hv_InputImageWidths;
  HTuple  hv_InputImageHeights, hv_ImageRange, hv_I, hv_InputImageWidth;
  HTuple  hv_InputImageHeight, hv_ZoomFactorWidth, hv_ZoomFactorHeight;
  HTuple  hv_ZoomHeight, hv_ZoomWidth, hv_ChannelIndex, hv_Min;
  HTuple  hv_Max, hv_Range, hv_Scale, hv_Shift;

  //This procedure preprocesses the provided images according to the parameters
  //in the dictionary DLPreprocessParam for an ocr_detection model.
  //
  //Check the validity of the preprocessing parameters.
  check_dl_preprocess_param(hv_DLPreprocessParam);
  //
  //Get the preprocessing parameters.
  GetDictTuple(hv_DLPreprocessParam, "image_width", &hv_ImageWidth);
  GetDictTuple(hv_DLPreprocessParam, "image_height", &hv_ImageHeight);
  GetDictTuple(hv_DLPreprocessParam, "image_num_channels", &hv_ImageNumChannels);
  GetDictTuple(hv_DLPreprocessParam, "image_range_min", &hv_ImageRangeMin);
  GetDictTuple(hv_DLPreprocessParam, "image_range_max", &hv_ImageRangeMax);
  GetDictTuple(hv_DLPreprocessParam, "domain_handling", &hv_DomainHandling);
  GetDictTuple(hv_DLPreprocessParam, "normalization_type", &hv_NormalizationType);
  GetDictTuple(hv_DLPreprocessParam, "model_type", &hv_ModelType);
  //
  //Check the preprocessing parameters.
  if (0 != (int(hv_ModelType!=HTuple("ocr_detection"))))
  {
    throw HException("The only 'model_type' value supported is'ocr_detection'.");
  }
  if (0 != (int(hv_ImageNumChannels!=3)))
  {
    throw HException("The only 'image_num_channels' value supported for ocr_detection models is 3.");
  }
  if (0 != (int(hv_DomainHandling!=HTuple("full_domain"))))
  {
    throw HException("The only 'domain_handling' value supported for ocr_detection models is 'full_domain'.");
  }
  if (0 != (HTuple(int(hv_NormalizationType!=HTuple("none"))).TupleAnd(int(hv_NormalizationType!=HTuple("all_channels")))))
  {
    throw HException("The 'normalization_type' values supported for ocr_detection models are 'all_channels' and 'none'.");
  }
  //
  //Get the image properties.
  CountObj(ho_Images, &hv_NumImages);
  CountChannels(ho_Images, &hv_NumChannels);
  GetImageType(ho_Images, &hv_ImageTypes);
  GetImageSize(ho_Images, &hv_InputImageWidths, &hv_InputImageHeights);
  //
  //Check the image properties.
  if (0 != (int(hv_NumImages==0)))
  {
    throw HException("Please provide some images to preprocess.");
  }
  if (0 != (int(hv_NumImages!=(hv_ImageTypes.TupleRegexpTest("byte")))))
  {
    throw HException("Please provide only images of type 'byte'.");
  }
  if (0 != (int(hv_NumImages!=(HTuple((hv_NumChannels.TupleEqualElem(1)).TupleOr(hv_NumChannels.TupleEqualElem(3))).TupleSum()))))
  {
    throw HException("Please provide only 1- or 3-channels images for ocr_detection models.");
  }
  //
  //Preprocess the images.
  hv_ImageRange = (hv_ImageRangeMax-hv_ImageRangeMin).TupleReal();
  {
  HTuple end_val49 = hv_NumImages-1;
  HTuple step_val49 = 1;
  for (hv_I=0; hv_I.Continue(end_val49, step_val49); hv_I += step_val49)
  {
    hv_InputImageWidth = HTuple(hv_InputImageWidths[hv_I]);
    hv_InputImageHeight = HTuple(hv_InputImageHeights[hv_I]);
    //
    SelectObj(ho_Images, &ho_Image, hv_I+1);
    //
    //Calculate aspect-ratio preserving zoom factors
    calculate_dl_image_zoom_factors(hv_InputImageWidth, hv_InputImageHeight, hv_ImageWidth, 
        hv_ImageHeight, hv_DLPreprocessParam, &hv_ZoomFactorWidth, &hv_ZoomFactorHeight);
    //
    //Zoom image
    hv_ZoomHeight = (hv_ZoomFactorHeight*hv_InputImageHeight).TupleRound();
    hv_ZoomWidth = (hv_ZoomFactorWidth*hv_InputImageWidth).TupleRound();
    ZoomImageSize(ho_Image, &ho_Image, hv_ZoomWidth, hv_ZoomHeight, "constant");
    //
    //Convert to real and normalize
    ConvertImageType(ho_Image, &ho_Image, "real");
    if (0 != (int(hv_NormalizationType==HTuple("all_channels"))))
    {
      GenEmptyObj(&ho_ImageScaled);
      {
      HTuple end_val67 = HTuple(hv_NumChannels[hv_I]);
      HTuple step_val67 = 1;
      for (hv_ChannelIndex=1; hv_ChannelIndex.Continue(end_val67, step_val67); hv_ChannelIndex += step_val67)
      {
        AccessChannel(ho_Image, &ho_Channel, hv_ChannelIndex);
        MinMaxGray(ho_Channel, ho_Channel, 0, &hv_Min, &hv_Max, &hv_Range);
        if (0 != (int((hv_Max-hv_Min)==0)))
        {
          hv_Scale = 1;
        }
        else
        {
          hv_Scale = (hv_ImageRangeMax-hv_ImageRangeMin)/(hv_Max-hv_Min);
        }
        hv_Shift = ((-hv_Scale)*hv_Min)+hv_ImageRangeMin;
        ScaleImage(ho_Channel, &ho_ChannelScaled, hv_Scale, hv_Shift);
        AppendChannel(ho_ImageScaled, ho_ChannelScaled, &ho_ImageScaled);
      }
      }
      ho_Image = ho_ImageScaled;
    }
    else if (0 != (int(hv_NormalizationType==HTuple("none"))))
    {
      ScaleImage(ho_Image, &ho_Image, hv_ImageRange/255.0, hv_ImageRangeMin);
    }
    //
    //Obtain an RGB image.
    if (0 != (int(HTuple(hv_NumChannels[hv_I])==1)))
    {
      CopyImage(ho_Image, &ho_ImageG);
      CopyImage(ho_Image, &ho_ImageB);
      Compose3(ho_Image, ho_ImageG, ho_ImageB, &ho_Image);
    }
    //
    //Apply padding to fit the desired image size.
    //The padding value is zero, corresponding to the
    //border handling of the convolution layers.
    ChangeFormat(ho_Image, &ho_Image, hv_ImageWidth, hv_ImageHeight);
    ReplaceObj(ho_Images, ho_Image, &ho_Images, hv_I+1);
  }
  }
  //
  //Return the preprocessed images.
  (*ho_ImagesPreprocessed) = ho_Images;
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Preprocess images for deep-learning-based training and inference of Deep OCR recognition models. 
void preprocess_dl_model_images_ocr_recognition (HObject ho_Images, HObject *ho_ImagesPreprocessed, 
    HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_TargetImage, ho_Image;

  // Local control variables
  HTuple  hv_ImageWidth, hv_ImageHeight, hv_ImageNumChannels;
  HTuple  hv_ImageRangeMin, hv_ImageRangeMax, hv_DomainHandling;
  HTuple  hv_NormalizationType, hv_ModelType, hv_NumImages;
  HTuple  hv_NumChannels, hv_ImageTypes, hv_InputImageWidths;
  HTuple  hv_InputImageHeights, hv_PaddingGrayval, hv_ImageRange;
  HTuple  hv_I, hv_InputImageWidth, hv_InputImageHeight, hv_InputImageWidthHeightRatio;
  HTuple  hv_ZoomHeight, hv_ZoomWidth, hv_GrayvalMin, hv_GrayvalMax;
  HTuple  hv_Range, hv_GrayvalRange, hv_Scale, hv_Shift;

  //This procedure preprocesses the provided Images according to the parameters
  //in the dictionary DLPreprocessParam for an ocr_recognition model.
  //
  //Check the validity of the preprocessing parameters.
  check_dl_preprocess_param(hv_DLPreprocessParam);
  //
  //Get the preprocessing parameters.
  GetDictTuple(hv_DLPreprocessParam, "image_width", &hv_ImageWidth);
  GetDictTuple(hv_DLPreprocessParam, "image_height", &hv_ImageHeight);
  GetDictTuple(hv_DLPreprocessParam, "image_num_channels", &hv_ImageNumChannels);
  GetDictTuple(hv_DLPreprocessParam, "image_range_min", &hv_ImageRangeMin);
  GetDictTuple(hv_DLPreprocessParam, "image_range_max", &hv_ImageRangeMax);
  GetDictTuple(hv_DLPreprocessParam, "domain_handling", &hv_DomainHandling);
  GetDictTuple(hv_DLPreprocessParam, "normalization_type", &hv_NormalizationType);
  GetDictTuple(hv_DLPreprocessParam, "model_type", &hv_ModelType);
  //
  //Check the preprocessing parameters.
  if (0 != (int(hv_ModelType!=HTuple("ocr_recognition"))))
  {
    throw HException("The only 'model_type' value supported is'ocr_recognition'.");
  }
  if (0 != (int(hv_ImageNumChannels!=1)))
  {
    throw HException("The only 'image_num_channels' value supported for ocr_recognition models is 1.");
  }
  if (0 != (int(hv_DomainHandling!=HTuple("full_domain"))))
  {
    throw HException("The only 'domain_handling' value supported for ocr_recognition models is 'full_domain'.");
  }
  if (0 != (HTuple(HTuple(int(hv_NormalizationType!=HTuple("none"))).TupleAnd(int(hv_NormalizationType!=HTuple("first_channel")))).TupleAnd(int(hv_NormalizationType!=HTuple("all_channels")))))
  {
    throw HException(HTuple("The 'normalization_type' values supported for ocr_recognition models are 'first_channel', 'all_channels' and 'none'."));
  }
  //
  //Get the image properties.
  CountObj(ho_Images, &hv_NumImages);
  CountChannels(ho_Images, &hv_NumChannels);
  GetImageType(ho_Images, &hv_ImageTypes);
  GetImageSize(ho_Images, &hv_InputImageWidths, &hv_InputImageHeights);
  //
  //Check the image properties.
  if (0 != (int(hv_NumImages==0)))
  {
    throw HException("Please provide some images to preprocess.");
  }
  if (0 != (int(hv_NumImages!=(hv_ImageTypes.TupleRegexpTest("byte|real")))))
  {
    throw HException("Please provide only images of type 'byte' or 'real'.");
  }
  if (0 != (int(hv_NumImages!=(HTuple((hv_NumChannels.TupleEqualElem(1)).TupleOr(hv_NumChannels.TupleEqualElem(3))).TupleSum()))))
  {
    throw HException("Please provide only 1- or 3-channels images for ocr_recognition models.");
  }
  //
  //Preprocess the images.
  hv_PaddingGrayval = 0.0;
  hv_ImageRange = (hv_ImageRangeMax-hv_ImageRangeMin).TupleReal();
  GenImageConst(&ho_TargetImage, "real", hv_ImageWidth, hv_ImageHeight);
  OverpaintRegion(ho_TargetImage, ho_TargetImage, hv_PaddingGrayval, "fill");
  {
  HTuple end_val52 = hv_NumImages-1;
  HTuple step_val52 = 1;
  for (hv_I=0; hv_I.Continue(end_val52, step_val52); hv_I += step_val52)
  {
    hv_InputImageWidth = HTuple(hv_InputImageWidths[hv_I]);
    hv_InputImageHeight = HTuple(hv_InputImageHeights[hv_I]);
    hv_InputImageWidthHeightRatio = hv_InputImageWidth/(hv_InputImageHeight.TupleReal());
    //
    SelectObj(ho_Images, &ho_Image, hv_I+1);
    FullDomain(ho_Image, &ho_Image);
    if (0 != (int(HTuple(hv_NumChannels[hv_I])==3)))
    {
      Rgb1ToGray(ho_Image, &ho_Image);
    }
    //
    hv_ZoomHeight = hv_ImageHeight;
    hv_ZoomWidth = hv_ImageWidth.TupleMin2((hv_ImageHeight*hv_InputImageWidthHeightRatio).TupleInt());
    ZoomImageSize(ho_Image, &ho_Image, hv_ZoomWidth, hv_ZoomHeight, "constant");
    if (0 != (int(HTuple(hv_ImageTypes[hv_I])==HTuple("byte"))))
    {
      ConvertImageType(ho_Image, &ho_Image, "real");
    }
    if (0 != (HTuple(int(hv_NormalizationType==HTuple("first_channel"))).TupleOr(int(hv_NormalizationType==HTuple("all_channels")))))
    {
      MinMaxGray(ho_Image, ho_Image, 0, &hv_GrayvalMin, &hv_GrayvalMax, &hv_Range);
      hv_GrayvalRange = (hv_GrayvalMax-hv_GrayvalMin).TupleReal();
      if (0 != (int(hv_GrayvalRange==0.0)))
      {
        hv_Scale = 1.0;
      }
      else
      {
        hv_Scale = hv_ImageRange/hv_GrayvalRange;
      }
      hv_Shift = ((-hv_Scale)*hv_GrayvalMin)+hv_ImageRangeMin;
      ScaleImage(ho_Image, &ho_Image, hv_Scale, hv_Shift);
    }
    else if (0 != (int(hv_NormalizationType==HTuple("none"))))
    {
      if (0 != (int(HTuple(hv_ImageTypes[hv_I])==HTuple("byte"))))
      {
        ScaleImage(ho_Image, &ho_Image, hv_ImageRange/255.0, hv_ImageRangeMin);
      }
    }
    //
    OverpaintGray(ho_TargetImage, ho_Image);
    ReduceDomain(ho_TargetImage, ho_Image, &ho_TargetImage);
    ReplaceObj(ho_Images, ho_TargetImage, &ho_Images, hv_I+1);
  }
  }
  //
  //Return the preprocessed images.
  (*ho_ImagesPreprocessed) = ho_Images;
  return;
}

// Chapter: Deep Learning / Object Detection and Instance Segmentation
// Short Description: Preprocess the instance segmentation masks for a sample given by the dictionary DLSample. 
void preprocess_dl_model_instance_masks (HObject ho_ImageRaw, HTuple hv_DLSample, 
    HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_InstanceMasks, ho_Domain;

  // Local control variables
  HTuple  hv_ImageWidth, hv_ImageHeight, hv_DomainHandling;
  HTuple  hv_NumMasks, hv_WidthRaw, hv_HeightRaw, hv_DomainRow1;
  HTuple  hv_DomainColumn1, hv_DomainRow2, hv_DomainColumn2;
  HTuple  hv_FactorResampleWidth, hv_FactorResampleHeight;

  //
  //This procedure preprocesses the instance masks of a DLSample.
  //
  //Check preprocess parameters.
  check_dl_preprocess_param(hv_DLPreprocessParam);
  //
  //Get relevant preprocess parameters.
  GetDictTuple(hv_DLPreprocessParam, "image_width", &hv_ImageWidth);
  GetDictTuple(hv_DLPreprocessParam, "image_height", &hv_ImageHeight);
  GetDictTuple(hv_DLPreprocessParam, "domain_handling", &hv_DomainHandling);
  //
  //Get the preprocessed instance masks.
  GetDictObject(&ho_InstanceMasks, hv_DLSample, "mask");
  //
  //Get the number of instance masks.
  CountObj(ho_InstanceMasks, &hv_NumMasks);
  //
  //Domain handling of the image to be preprocessed.
  //
  GetImageSize(ho_ImageRaw, &hv_WidthRaw, &hv_HeightRaw);
  if (0 != (int(hv_DomainHandling==HTuple("crop_domain"))))
  {
    //Clip and translate masks w.r.t. the image domain
    GetDomain(ho_ImageRaw, &ho_Domain);
    SmallestRectangle1(ho_Domain, &hv_DomainRow1, &hv_DomainColumn1, &hv_DomainRow2, 
        &hv_DomainColumn2);
    //
    //Clip the remaining regions to the domain.
    ClipRegion(ho_InstanceMasks, &ho_InstanceMasks, hv_DomainRow1, hv_DomainColumn1, 
        hv_DomainRow2, hv_DomainColumn2);
    hv_WidthRaw = (hv_DomainColumn2-hv_DomainColumn1)+1.0;
    hv_HeightRaw = (hv_DomainRow2-hv_DomainRow1)+1.0;
    //We need to move the remaining regions back to the origin,
    //because crop_domain will be applied to the image
    MoveRegion(ho_InstanceMasks, &ho_InstanceMasks, -hv_DomainRow1, -hv_DomainColumn1);
  }
  else if (0 != (int(hv_DomainHandling!=HTuple("full_domain"))))
  {
    throw HException("Unsupported parameter value for 'domain_handling'");
  }
  //
  //Zoom masks only if the image has a different size than the specified size.
  if (0 != ((hv_ImageWidth.TupleNotEqualElem(hv_WidthRaw)).TupleOr(hv_ImageHeight.TupleNotEqualElem(hv_HeightRaw))))
  {
    //Calculate rescaling factor.
    hv_FactorResampleWidth = (hv_ImageWidth.TupleReal())/hv_WidthRaw;
    hv_FactorResampleHeight = (hv_ImageHeight.TupleReal())/hv_HeightRaw;

    //Zoom the masks.
    ZoomRegion(ho_InstanceMasks, &ho_InstanceMasks, hv_FactorResampleWidth, hv_FactorResampleHeight);
  }
  //
  //Set the preprocessed instance masks.
  SetDictObject(ho_InstanceMasks, hv_DLSample, "mask");
  //
  //
  return;
}

// Chapter: Deep Learning / Semantic Segmentation and Edge Extraction
// Short Description: Preprocess segmentation and weight images for deep-learning-based segmentation training and inference. 
void preprocess_dl_model_segmentations (HObject ho_ImagesRaw, HObject ho_Segmentations, 
    HObject *ho_SegmentationsPreprocessed, HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_Domain, ho_SelectedSeg, ho_SelectedDomain;

  // Local control variables
  HTuple  hv_NumberImages, hv_NumberSegmentations;
  HTuple  hv_Width, hv_Height, hv_WidthSeg, hv_HeightSeg;
  HTuple  hv_DLModelType, hv_ImageWidth, hv_ImageHeight, hv_ImageNumChannels;
  HTuple  hv_ImageRangeMin, hv_ImageRangeMax, hv_DomainHandling;
  HTuple  hv_SetBackgroundID, hv_ClassesToBackground, hv_IgnoreClassIDs;
  HTuple  hv_IsInt, hv_IndexImage, hv_ImageWidthRaw, hv_ImageHeightRaw;
  HTuple  hv_EqualWidth, hv_EqualHeight, hv_Type, hv_EqualReal;

  //
  //This procedure preprocesses the segmentation or weight images
  //given by Segmentations so that they can be handled by
  //train_dl_model_batch and apply_dl_model.
  //
  //Check input data.
  //Examine number of images.
  CountObj(ho_ImagesRaw, &hv_NumberImages);
  CountObj(ho_Segmentations, &hv_NumberSegmentations);
  if (0 != (int(hv_NumberImages!=hv_NumberSegmentations)))
  {
    throw HException("Equal number of images given in ImagesRaw and Segmentations required");
  }
  //Size of images.
  GetImageSize(ho_ImagesRaw, &hv_Width, &hv_Height);
  GetImageSize(ho_Segmentations, &hv_WidthSeg, &hv_HeightSeg);
  if (0 != (HTuple(int(hv_Width!=hv_WidthSeg)).TupleOr(int(hv_Height!=hv_HeightSeg))))
  {
    throw HException("Equal size of the images given in ImagesRaw and Segmentations required.");
  }
  //Check the validity of the preprocessing parameters.
  check_dl_preprocess_param(hv_DLPreprocessParam);
  //
  //Get the relevant preprocessing parameters.
  GetDictTuple(hv_DLPreprocessParam, "model_type", &hv_DLModelType);
  GetDictTuple(hv_DLPreprocessParam, "image_width", &hv_ImageWidth);
  GetDictTuple(hv_DLPreprocessParam, "image_height", &hv_ImageHeight);
  GetDictTuple(hv_DLPreprocessParam, "image_num_channels", &hv_ImageNumChannels);
  GetDictTuple(hv_DLPreprocessParam, "image_range_min", &hv_ImageRangeMin);
  GetDictTuple(hv_DLPreprocessParam, "image_range_max", &hv_ImageRangeMax);
  GetDictTuple(hv_DLPreprocessParam, "domain_handling", &hv_DomainHandling);
  //Segmentation specific parameters.
  GetDictTuple(hv_DLPreprocessParam, "set_background_id", &hv_SetBackgroundID);
  GetDictTuple(hv_DLPreprocessParam, "class_ids_background", &hv_ClassesToBackground);
  GetDictTuple(hv_DLPreprocessParam, "ignore_class_ids", &hv_IgnoreClassIDs);
  //
  //Check the input parameter for setting the background ID.
  if (0 != (int(hv_SetBackgroundID!=HTuple())))
  {
    //Check that the model is a segmentation model.
    if (0 != (int(hv_DLModelType!=HTuple("segmentation"))))
    {
      throw HException("Setting class IDs to background is only implemented for segmentation.");
    }
    //Check the background ID.
    TupleIsIntElem(hv_SetBackgroundID, &hv_IsInt);
    if (0 != (int((hv_SetBackgroundID.TupleLength())!=1)))
    {
      throw HException("Only one class_id as 'set_background_id' allowed.");
    }
    else if (0 != (hv_IsInt.TupleNot()))
    {
      //Given class_id has to be of type int.
      throw HException("The class_id given as 'set_background_id' has to be of type int.");
    }
    //Check the values of ClassesToBackground.
    if (0 != (int((hv_ClassesToBackground.TupleLength())==0)))
    {
      //Check that the given classes are of length > 0.
      throw HException(HTuple("If 'set_background_id' is given, 'class_ids_background' must at least contain this class ID."));
    }
    else if (0 != (int((hv_ClassesToBackground.TupleIntersection(hv_IgnoreClassIDs))!=HTuple())))
    {
      //Check that class_ids_background is not included in the ignore_class_ids of the DLModel.
      throw HException("The given 'class_ids_background' must not be included in the 'ignore_class_ids' of the model.");
    }
  }
  //
  //Domain handling of the image to be preprocessed.
  //
  if (0 != (HTuple(int(hv_DomainHandling==HTuple("full_domain"))).TupleOr(int(hv_DomainHandling==HTuple("keep_domain")))))
  {
    FullDomain(ho_Segmentations, &ho_Segmentations);
  }
  else if (0 != (int(hv_DomainHandling==HTuple("crop_domain"))))
  {
    //If the domain should be cropped the domain has to be transferred
    //from the raw image to the segmentation image.
    GetDomain(ho_ImagesRaw, &ho_Domain);
    {
    HTuple end_val66 = hv_NumberImages;
    HTuple step_val66 = 1;
    for (hv_IndexImage=1; hv_IndexImage.Continue(end_val66, step_val66); hv_IndexImage += step_val66)
    {
      SelectObj(ho_Segmentations, &ho_SelectedSeg, hv_IndexImage);
      SelectObj(ho_Domain, &ho_SelectedDomain, hv_IndexImage);
      ChangeDomain(ho_SelectedSeg, ho_SelectedDomain, &ho_SelectedSeg);
      ReplaceObj(ho_Segmentations, ho_SelectedSeg, &ho_Segmentations, hv_IndexImage);
    }
    }
    CropDomain(ho_Segmentations, &ho_Segmentations);
  }
  else
  {
    throw HException("Unsupported parameter value for 'domain_handling'");
  }
  //
  //Preprocess the segmentation images.
  //
  //Set all background classes to the given background class ID.
  if (0 != (int(hv_SetBackgroundID!=HTuple())))
  {
    reassign_pixel_values(ho_Segmentations, &ho_Segmentations, hv_ClassesToBackground, 
        hv_SetBackgroundID);
  }
  //
  //Zoom images only if they have a different size than the specified size.
  GetImageSize(ho_Segmentations, &hv_ImageWidthRaw, &hv_ImageHeightRaw);
  hv_EqualWidth = hv_ImageWidth.TupleEqualElem(hv_ImageWidthRaw);
  hv_EqualHeight = hv_ImageHeight.TupleEqualElem(hv_ImageHeightRaw);
  if (0 != (HTuple(int((hv_EqualWidth.TupleMin())==0)).TupleOr(int((hv_EqualHeight.TupleMin())==0))))
  {
    ZoomImageSize(ho_Segmentations, &ho_Segmentations, hv_ImageWidth, hv_ImageHeight, 
        "nearest_neighbor");
  }
  //
  //Check the type of the input images
  //and convert if necessary.
  GetImageType(ho_Segmentations, &hv_Type);
  hv_EqualReal = hv_Type.TupleEqualElem("real");
  //
  if (0 != (int((hv_EqualReal.TupleMin())==0)))
  {
    //Convert the image type to 'real',
    //because the model expects 'real' images.
    ConvertImageType(ho_Segmentations, &ho_Segmentations, "real");
  }
  //
  //Write preprocessed Segmentations to output variable.
  (*ho_SegmentationsPreprocessed) = ho_Segmentations;
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Preprocess given DLSamples according to the preprocessing parameters given in DLPreprocessParam. 
void preprocess_dl_samples (HTuple hv_DLSampleBatch, HTuple hv_DLPreprocessParam)
{

  // Local iconic variables
  HObject  ho_ImageRaw, ho_ImagePreprocessed, ho_AnomalyImageRaw;
  HObject  ho_AnomalyImagePreprocessed, ho_SegmentationRaw;
  HObject  ho_SegmentationPreprocessed, ho_ImageRawDomain;

  // Local control variables
  HTuple  hv_SampleIndex, hv_DLSample, hv_ImageExists;
  HTuple  hv_KeysExists, hv_AnomalyParamExist, hv_Rectangle1ParamExist;
  HTuple  hv_Rectangle2ParamExist, hv_InstanceMaskParamExist;
  HTuple  hv_SegmentationParamExist, hv_OCRParamExist;

  //
  //This procedure preprocesses all images of the sample dictionaries
  //in the tuple DLSampleBatch.
  //The images are preprocessed according to the parameters provided
  //in DLPreprocessParam.
  //
  //Check the validity of the preprocessing parameters.
  //The procedure check_dl_preprocess_param might change DLPreprocessParam.
  //To avoid race conditions when preprocess_dl_samples is used from
  //multiple threads with the same DLPreprocessParam dictionary,
  //work on a copy.
  CopyDict(hv_DLPreprocessParam, HTuple(), HTuple(), &hv_DLPreprocessParam);
  check_dl_preprocess_param(hv_DLPreprocessParam);
  //
  //
  //
  //Preprocess the sample entries.
  //
  {
  HTuple end_val18 = (hv_DLSampleBatch.TupleLength())-1;
  HTuple step_val18 = 1;
  for (hv_SampleIndex=0; hv_SampleIndex.Continue(end_val18, step_val18); hv_SampleIndex += step_val18)
  {
    hv_DLSample = HTuple(hv_DLSampleBatch[hv_SampleIndex]);
    //
    //Preprocess augmentation data.
    preprocess_dl_model_augmentation_data(hv_DLSample, hv_DLPreprocessParam);
    //
    //Check the existence of the sample keys.
    GetDictParam(hv_DLSample, "key_exists", "image", &hv_ImageExists);
    //
    //Preprocess the images.
    if (0 != hv_ImageExists)
    {
      //
      //Get the image.
      GetDictObject(&ho_ImageRaw, hv_DLSample, "image");
      //
      //Preprocess the image.
      preprocess_dl_model_images(ho_ImageRaw, &ho_ImagePreprocessed, hv_DLPreprocessParam);
      //
      //Replace the image in the dictionary.
      SetDictObject(ho_ImagePreprocessed, hv_DLSample, "image");
      //
      //Check existence of model specific sample keys:
      //- 'anomaly_ground_truth':
      //  For model 'type' = 'anomaly_detection' and
      //  model 'type' = 'gc_anomaly_detection'
      //- 'bbox_row1':
      //  For 'instance_type' = 'rectangle1' and
      //  model 'type' = 'detection'
      //- 'bbox_phi':
      //  For 'instance_type' = 'rectangle2' and
      //  model 'type' = 'detection'
      //- 'mask':
      //  For 'instance_type' = 'rectangle1',
      //  model 'type' = 'detection', and
      //  'instance_segmentation' = true
      //- 'segmentation_image':
      //  For model 'type' = 'segmentation'
      GetDictParam(hv_DLSample, "key_exists", (((((HTuple("anomaly_ground_truth").Append("bbox_row1")).Append("bbox_phi")).Append("mask")).Append("segmentation_image")).Append("word")), 
          &hv_KeysExists);
      hv_AnomalyParamExist = ((const HTuple&)hv_KeysExists)[0];
      hv_Rectangle1ParamExist = ((const HTuple&)hv_KeysExists)[1];
      hv_Rectangle2ParamExist = ((const HTuple&)hv_KeysExists)[2];
      hv_InstanceMaskParamExist = ((const HTuple&)hv_KeysExists)[3];
      hv_SegmentationParamExist = ((const HTuple&)hv_KeysExists)[4];
      hv_OCRParamExist = ((const HTuple&)hv_KeysExists)[5];
      //
      //Preprocess the anomaly ground truth for
      //model 'type' = 'anomaly_detection' or
      //model 'type' = 'gc_anomaly_detection' if present.
      if (0 != hv_AnomalyParamExist)
      {
        //
        //Get the anomaly image.
        GetDictObject(&ho_AnomalyImageRaw, hv_DLSample, "anomaly_ground_truth");
        //
        //Preprocess the anomaly image.
        preprocess_dl_model_anomaly(ho_AnomalyImageRaw, &ho_AnomalyImagePreprocessed, 
            hv_DLPreprocessParam);
        //
        //Set preprocessed anomaly image.
        SetDictObject(ho_AnomalyImagePreprocessed, hv_DLSample, "anomaly_ground_truth");
      }
      //
      //Preprocess depending on the model type.
      //If bounding boxes are given, rescale them as well.
      if (0 != hv_Rectangle1ParamExist)
      {
        //
        //Preprocess the bounding boxes of type 'rectangle1'.
        preprocess_dl_model_bbox_rect1(ho_ImageRaw, hv_DLSample, hv_DLPreprocessParam);
      }
      else if (0 != hv_Rectangle2ParamExist)
      {
        //
        //Preprocess the bounding boxes of type 'rectangle2'.
        preprocess_dl_model_bbox_rect2(ho_ImageRaw, hv_DLSample, hv_DLPreprocessParam);
      }
      if (0 != hv_InstanceMaskParamExist)
      {
        //
        //Preprocess the instance masks.
        preprocess_dl_model_instance_masks(ho_ImageRaw, hv_DLSample, hv_DLPreprocessParam);
      }
      //
      //Preprocess the segmentation image if present.
      if (0 != hv_SegmentationParamExist)
      {
        //
        //Get the segmentation image.
        GetDictObject(&ho_SegmentationRaw, hv_DLSample, "segmentation_image");
        //
        //Preprocess the segmentation image.
        preprocess_dl_model_segmentations(ho_ImageRaw, ho_SegmentationRaw, &ho_SegmentationPreprocessed, 
            hv_DLPreprocessParam);
        //
        //Set preprocessed segmentation image.
        SetDictObject(ho_SegmentationPreprocessed, hv_DLSample, "segmentation_image");
      }
      //
      //Preprocess the word bounding boxes and generate targets.
      if (0 != (hv_OCRParamExist.TupleAnd(hv_Rectangle2ParamExist)))
      {
        //
        //Preprocess Sample.
        gen_dl_ocr_detection_targets(hv_DLSample, hv_DLPreprocessParam);
      }
      //
      //Preprocess 3D relevant data if present.
      GetDictParam(hv_DLSample, "key_exists", (((HTuple("x").Append("y")).Append("z")).Append("normals")), 
          &hv_KeysExists);
      if (0 != (hv_KeysExists.TupleMax()))
      {
        //We need to handle crop_domain before preprocess_dl_model_3d_data
        //if it is necessary.
        //Note, we are not cropping the image of DLSample because it has
        //been done by preprocess_dl_model_images.
        //Since we always keep the domain of 3D data we do not need to handle
        //'keep_domain' or 'full_domain'.
        GetDomain(ho_ImageRaw, &ho_ImageRawDomain);
        crop_dl_sample_image(ho_ImageRawDomain, hv_DLSample, "x", hv_DLPreprocessParam);
        crop_dl_sample_image(ho_ImageRawDomain, hv_DLSample, "y", hv_DLPreprocessParam);
        crop_dl_sample_image(ho_ImageRawDomain, hv_DLSample, "z", hv_DLPreprocessParam);
        crop_dl_sample_image(ho_ImageRawDomain, hv_DLSample, "normals", hv_DLPreprocessParam);
        //
        preprocess_dl_model_3d_data(hv_DLSample, hv_DLPreprocessParam);
      }
    }
    else
    {
      throw HException((HTuple("All samples processed need to include an image, but the sample with index ")+hv_SampleIndex)+" does not.");
    }
  }
  }
  //
  return;
}

// Chapter: Image / Manipulation
// Short Description: Change value of ValuesToChange in Image to NewValue. 
void reassign_pixel_values (HObject ho_Image, HObject *ho_ImageOut, HTuple hv_ValuesToChange, 
    HTuple hv_NewValue)
{

  // Local iconic variables
  HObject  ho_RegionToChange, ho_RegionClass;

  // Local control variables
  HTuple  hv_IndexReset;

  //
  //This procedure sets all pixels of Image
  //with the values given in ValuesToChange to the given value NewValue.
  //
  GenEmptyRegion(&ho_RegionToChange);
  {
  HTuple end_val5 = (hv_ValuesToChange.TupleLength())-1;
  HTuple step_val5 = 1;
  for (hv_IndexReset=0; hv_IndexReset.Continue(end_val5, step_val5); hv_IndexReset += step_val5)
  {
    Threshold(ho_Image, &ho_RegionClass, HTuple(hv_ValuesToChange[hv_IndexReset]), 
        HTuple(hv_ValuesToChange[hv_IndexReset]));
    Union2(ho_RegionToChange, ho_RegionClass, &ho_RegionToChange);
  }
  }
  OverpaintRegion(ho_Image, ho_RegionToChange, hv_NewValue, "fill");
  (*ho_ImageOut) = ho_Image;
  return;
}

// Chapter: File / Misc
// Short Description: Remove a directory recursively. 
void remove_dir_recursively (HTuple hv_DirName)
{

  // Local control variables
  HTuple  hv_Dirs, hv_I, hv_Files;

  //Recursively delete all subdirectories.
  ListFiles(hv_DirName, "directories", &hv_Dirs);
  {
  HTuple end_val2 = (hv_Dirs.TupleLength())-1;
  HTuple step_val2 = 1;
  for (hv_I=0; hv_I.Continue(end_val2, step_val2); hv_I += step_val2)
  {
    remove_dir_recursively(HTuple(hv_Dirs[hv_I]));
  }
  }
  //Delete all files.
  ListFiles(hv_DirName, "files", &hv_Files);
  {
  HTuple end_val7 = (hv_Files.TupleLength())-1;
  HTuple step_val7 = 1;
  for (hv_I=0; hv_I.Continue(end_val7, step_val7); hv_I += step_val7)
  {
    DeleteFile(HTuple(hv_Files[hv_I]));
  }
  }
  //Remove empty directory.
  RemoveDir(hv_DirName);
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Remove invalid 3D pixels from a given domain. 
void remove_invalid_3d_pixels (HObject ho_ImageX, HObject ho_ImageY, HObject ho_ImageZ, 
    HObject ho_Domain, HObject *ho_DomainOut, HTuple hv_InvalidPixelValue)
{

  // Local iconic variables
  HObject  ho_ImageXOut, ho_ImageYOut, ho_ImageZOut;
  HObject  ho_RegionInvalX, ho_RegionInvalY, ho_RegionInvalZ;
  HObject  ho_RegionInvalXY, ho_RegionInval, ho_RegionInvalComplement;

  (*ho_DomainOut) = ho_Domain;
  ho_ImageXOut = ho_ImageX;
  ho_ImageYOut = ho_ImageY;
  ho_ImageZOut = ho_ImageZ;
  ReduceDomain(ho_ImageXOut, (*ho_DomainOut), &ho_ImageXOut);
  ReduceDomain(ho_ImageYOut, (*ho_DomainOut), &ho_ImageYOut);
  ReduceDomain(ho_ImageZOut, (*ho_DomainOut), &ho_ImageZOut);
  Threshold(ho_ImageXOut, &ho_RegionInvalX, hv_InvalidPixelValue, hv_InvalidPixelValue);
  Threshold(ho_ImageYOut, &ho_RegionInvalY, hv_InvalidPixelValue, hv_InvalidPixelValue);
  Threshold(ho_ImageZOut, &ho_RegionInvalZ, hv_InvalidPixelValue, hv_InvalidPixelValue);
  Intersection(ho_RegionInvalX, ho_RegionInvalY, &ho_RegionInvalXY);
  Intersection(ho_RegionInvalXY, ho_RegionInvalZ, &ho_RegionInval);
  Complement(ho_RegionInval, &ho_RegionInvalComplement);
  Intersection((*ho_DomainOut), ho_RegionInvalComplement, &(*ho_DomainOut));
  return;
}

// Chapter: Deep Learning / Model
// Short Description: Replace legacy preprocessing parameters or values. 
void replace_legacy_preprocessing_parameters (HTuple hv_DLPreprocessParam)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_Exception, hv_NormalizationTypeExists;
  HTuple  hv_NormalizationType, hv_LegacyNormalizationKeyExists;
  HTuple  hv_ContrastNormalization;

  //
  //This procedure adapts the dictionary DLPreprocessParam
  //if a legacy preprocessing parameter is set.
  //
  //Map legacy value set to new parameter.
  hv_Exception = 0;
  try
  {
    GetDictParam(hv_DLPreprocessParam, "key_exists", "normalization_type", &hv_NormalizationTypeExists);
    //
    if (0 != hv_NormalizationTypeExists)
    {
      GetDictTuple(hv_DLPreprocessParam, "normalization_type", &hv_NormalizationType);
      if (0 != (int(hv_NormalizationType==HTuple("true"))))
      {
        hv_NormalizationType = "first_channel";
      }
      else if (0 != (int(hv_NormalizationType==HTuple("false"))))
      {
        hv_NormalizationType = "none";
      }
      SetDictTuple(hv_DLPreprocessParam, "normalization_type", hv_NormalizationType);
    }
  }
  // catch (Exception) 
  catch (HException &HDevExpDefaultException)
  {
    HDevExpDefaultException.ToHTuple(&hv_Exception);
  }
  //
  //Map legacy parameter to new parameter and corresponding value.
  hv_Exception = 0;
  try
  {
    GetDictParam(hv_DLPreprocessParam, "key_exists", "contrast_normalization", &hv_LegacyNormalizationKeyExists);
    if (0 != hv_LegacyNormalizationKeyExists)
    {
      GetDictTuple(hv_DLPreprocessParam, "contrast_normalization", &hv_ContrastNormalization);
      //Replace 'contrast_normalization' by 'normalization_type'.
      if (0 != (int(hv_ContrastNormalization==HTuple("false"))))
      {
        SetDictTuple(hv_DLPreprocessParam, "normalization_type", "none");
      }
      else if (0 != (int(hv_ContrastNormalization==HTuple("true"))))
      {
        SetDictTuple(hv_DLPreprocessParam, "normalization_type", "first_channel");
      }
      RemoveDictKey(hv_DLPreprocessParam, "contrast_normalization");
    }
  }
  // catch (Exception) 
  catch (HException &HDevExpDefaultException)
  {
    HDevExpDefaultException.ToHTuple(&hv_Exception);
  }
  return;
}

// Chapter: Filters / Arithmetic
// Short Description: Scale the gray values of an image from the interval [Min,Max] to [0,255] 
void scale_image_range (HObject ho_Image, HObject *ho_ImageScaled, HTuple hv_Min, 
    HTuple hv_Max)
{

  // Local iconic variables
  HObject  ho_ImageSelected, ho_SelectedChannel;
  HObject  ho_LowerRegion, ho_UpperRegion, ho_ImageSelectedScaled;

  // Local control variables
  HTuple  hv_LowerLimit, hv_UpperLimit, hv_Mult;
  HTuple  hv_Add, hv_NumImages, hv_ImageIndex, hv_Channels;
  HTuple  hv_ChannelIndex, hv_MinGray, hv_MaxGray, hv_Range;

  //Convenience procedure to scale the gray values of the
  //input image Image from the interval [Min,Max]
  //to the interval [0,255] (default).
  //Gray values < 0 or > 255 (after scaling) are clipped.
  //
  //If the image shall be scaled to an interval different from [0,255],
  //this can be achieved by passing tuples with 2 values [From, To]
  //as Min and Max.
  //Example:
  //scale_image_range(Image:ImageScaled:[100,50],[200,250])
  //maps the gray values of Image from the interval [100,200] to [50,250].
  //All other gray values will be clipped.
  //
  //input parameters:
  //Image: the input image
  //Min: the minimum gray value which will be mapped to 0
  //     If a tuple with two values is given, the first value will
  //     be mapped to the second value.
  //Max: The maximum gray value which will be mapped to 255
  //     If a tuple with two values is given, the first value will
  //     be mapped to the second value.
  //
  //Output parameter:
  //ImageScale: the resulting scaled image.
  //
  if (0 != (int((hv_Min.TupleLength())==2)))
  {
    hv_LowerLimit = ((const HTuple&)hv_Min)[1];
    hv_Min = ((const HTuple&)hv_Min)[0];
  }
  else
  {
    hv_LowerLimit = 0.0;
  }
  if (0 != (int((hv_Max.TupleLength())==2)))
  {
    hv_UpperLimit = ((const HTuple&)hv_Max)[1];
    hv_Max = ((const HTuple&)hv_Max)[0];
  }
  else
  {
    hv_UpperLimit = 255.0;
  }
  //
  //Calculate scaling parameters.
  //Only scale if the scaling range is not zero.
  if (0 != (HTuple(int(((hv_Max-hv_Min).TupleAbs())<1.0E-6)).TupleNot()))
  {
    hv_Mult = ((hv_UpperLimit-hv_LowerLimit).TupleReal())/(hv_Max-hv_Min);
    hv_Add = ((-hv_Mult)*hv_Min)+hv_LowerLimit;
    //Scale image.
    ScaleImage(ho_Image, &ho_Image, hv_Mult, hv_Add);
  }
  //
  //Clip gray values if necessary.
  //This must be done for each image and channel separately.
  GenEmptyObj(&(*ho_ImageScaled));
  CountObj(ho_Image, &hv_NumImages);
  {
  HTuple end_val51 = hv_NumImages;
  HTuple step_val51 = 1;
  for (hv_ImageIndex=1; hv_ImageIndex.Continue(end_val51, step_val51); hv_ImageIndex += step_val51)
  {
    SelectObj(ho_Image, &ho_ImageSelected, hv_ImageIndex);
    CountChannels(ho_ImageSelected, &hv_Channels);
    {
    HTuple end_val54 = hv_Channels;
    HTuple step_val54 = 1;
    for (hv_ChannelIndex=1; hv_ChannelIndex.Continue(end_val54, step_val54); hv_ChannelIndex += step_val54)
    {
      AccessChannel(ho_ImageSelected, &ho_SelectedChannel, hv_ChannelIndex);
      MinMaxGray(ho_SelectedChannel, ho_SelectedChannel, 0, &hv_MinGray, &hv_MaxGray, 
          &hv_Range);
      Threshold(ho_SelectedChannel, &ho_LowerRegion, (hv_MinGray.TupleConcat(hv_LowerLimit)).TupleMin(), 
          hv_LowerLimit);
      Threshold(ho_SelectedChannel, &ho_UpperRegion, hv_UpperLimit, (hv_UpperLimit.TupleConcat(hv_MaxGray)).TupleMax());
      PaintRegion(ho_LowerRegion, ho_SelectedChannel, &ho_SelectedChannel, hv_LowerLimit, 
          "fill");
      PaintRegion(ho_UpperRegion, ho_SelectedChannel, &ho_SelectedChannel, hv_UpperLimit, 
          "fill");
      if (0 != (int(hv_ChannelIndex==1)))
      {
        CopyObj(ho_SelectedChannel, &ho_ImageSelectedScaled, 1, 1);
      }
      else
      {
        AppendChannel(ho_ImageSelectedScaled, ho_SelectedChannel, &ho_ImageSelectedScaled
            );
      }
    }
    }
    ConcatObj((*ho_ImageScaled), ho_ImageSelectedScaled, &(*ho_ImageScaled));
  }
  }
  return;
}

// Chapter: Graphics / Text
// Short Description: Set font independent of OS 
void set_display_font (HTuple hv_WindowHandle, HTuple hv_Size, HTuple hv_Font, HTuple hv_Bold, 
    HTuple hv_Slant)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_OS, hv_Fonts, hv_Style, hv_Exception;
  HTuple  hv_AvailableFonts, hv_Fdx, hv_Indices;

  //This procedure sets the text font of the current window with
  //the specified attributes.
  //
  //Input parameters:
  //WindowHandle: The graphics window for which the font will be set
  //Size: The font size. If Size=-1, the default of 16 is used.
  //Bold: If set to 'true', a bold font is used
  //Slant: If set to 'true', a slanted font is used
  //
  GetSystem("operating_system", &hv_OS);
  if (0 != (HTuple(int(hv_Size==HTuple())).TupleOr(int(hv_Size==-1))))
  {
    hv_Size = 16;
  }
  if (0 != (int((hv_OS.TupleSubstr(0,2))==HTuple("Win"))))
  {
    //Restore previous behavior
    hv_Size = (1.13677*hv_Size).TupleInt();
  }
  else
  {
    hv_Size = hv_Size.TupleInt();
  }
  if (0 != (int(hv_Font==HTuple("Courier"))))
  {
    hv_Fonts.Clear();
    hv_Fonts[0] = "Courier";
    hv_Fonts[1] = "Courier 10 Pitch";
    hv_Fonts[2] = "Courier New";
    hv_Fonts[3] = "CourierNew";
    hv_Fonts[4] = "Liberation Mono";
  }
  else if (0 != (int(hv_Font==HTuple("mono"))))
  {
    hv_Fonts.Clear();
    hv_Fonts[0] = "Consolas";
    hv_Fonts[1] = "Menlo";
    hv_Fonts[2] = "Courier";
    hv_Fonts[3] = "Courier 10 Pitch";
    hv_Fonts[4] = "FreeMono";
    hv_Fonts[5] = "Liberation Mono";
  }
  else if (0 != (int(hv_Font==HTuple("sans"))))
  {
    hv_Fonts.Clear();
    hv_Fonts[0] = "Luxi Sans";
    hv_Fonts[1] = "DejaVu Sans";
    hv_Fonts[2] = "FreeSans";
    hv_Fonts[3] = "Arial";
    hv_Fonts[4] = "Liberation Sans";
  }
  else if (0 != (int(hv_Font==HTuple("serif"))))
  {
    hv_Fonts.Clear();
    hv_Fonts[0] = "Times New Roman";
    hv_Fonts[1] = "Luxi Serif";
    hv_Fonts[2] = "DejaVu Serif";
    hv_Fonts[3] = "FreeSerif";
    hv_Fonts[4] = "Utopia";
    hv_Fonts[5] = "Liberation Serif";
  }
  else
  {
    hv_Fonts = hv_Font;
  }
  hv_Style = "";
  if (0 != (int(hv_Bold==HTuple("true"))))
  {
    hv_Style += HTuple("Bold");
  }
  else if (0 != (int(hv_Bold!=HTuple("false"))))
  {
    hv_Exception = "Wrong value of control parameter Bold";
    throw HException(hv_Exception);
  }
  if (0 != (int(hv_Slant==HTuple("true"))))
  {
    hv_Style += HTuple("Italic");
  }
  else if (0 != (int(hv_Slant!=HTuple("false"))))
  {
    hv_Exception = "Wrong value of control parameter Slant";
    throw HException(hv_Exception);
  }
  if (0 != (int(hv_Style==HTuple(""))))
  {
    hv_Style = "Normal";
  }
  QueryFont(hv_WindowHandle, &hv_AvailableFonts);
  hv_Font = "";
  {
  HTuple end_val48 = (hv_Fonts.TupleLength())-1;
  HTuple step_val48 = 1;
  for (hv_Fdx=0; hv_Fdx.Continue(end_val48, step_val48); hv_Fdx += step_val48)
  {
    hv_Indices = hv_AvailableFonts.TupleFind(HTuple(hv_Fonts[hv_Fdx]));
    if (0 != (int((hv_Indices.TupleLength())>0)))
    {
      if (0 != (int(HTuple(hv_Indices[0])>=0)))
      {
        hv_Font = HTuple(hv_Fonts[hv_Fdx]);
        break;
      }
    }
  }
  }
  if (0 != (int(hv_Font==HTuple(""))))
  {
    throw HException("Wrong value of control parameter Font");
  }
  hv_Font = (((hv_Font+"-")+hv_Style)+"-")+hv_Size;
  SetFont(hv_WindowHandle, hv_Font);
  return;
}

// Chapter: OCR / Deep OCR
// Short Description: Split rectangle2 into a number of rectangles. 
void split_rectangle2 (HTuple hv_Row, HTuple hv_Column, HTuple hv_Phi, HTuple hv_Length1, 
    HTuple hv_Length2, HTuple hv_NumSplits, HTuple *hv_SplitRow, HTuple *hv_SplitColumn, 
    HTuple *hv_SplitPhi, HTuple *hv_SplitLength1Out, HTuple *hv_SplitLength2Out)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_SplitLength, hv_TRow, hv_TCol, hv_HomMat2D;

  if (0 != (int(hv_NumSplits>0)))
  {
    hv_SplitLength = hv_Length1/(hv_NumSplits.TupleReal());
    //Assume center (0,0), transform afterwards.
    hv_TRow = HTuple(hv_NumSplits,0.0);
    hv_TCol = ((-hv_Length1)+hv_SplitLength)+((HTuple::TupleGenSequence(0,hv_NumSplits-1,1)*2)*hv_SplitLength);
    HomMat2dIdentity(&hv_HomMat2D);
    HomMat2dRotate(hv_HomMat2D, hv_Phi, 0, 0, &hv_HomMat2D);
    HomMat2dTranslate(hv_HomMat2D, hv_Row, hv_Column, &hv_HomMat2D);
    (*hv_SplitLength1Out) = HTuple(hv_NumSplits,hv_SplitLength);
    (*hv_SplitLength2Out) = HTuple(hv_NumSplits,hv_Length2);
    (*hv_SplitPhi) = HTuple(hv_NumSplits,hv_Phi);
    AffineTransPoint2d(hv_HomMat2D, hv_TRow, hv_TCol, &(*hv_SplitRow), &(*hv_SplitColumn));
  }
  else
  {
    throw HException("Number of splits must be greater than 0.");
  }
  return;
}

// Chapter: Graphics / Window
// Short Description: Set and return meta information to display images correctly. 
void update_window_meta_information (HTuple hv_WindowHandle, HTuple hv_WidthImage, 
    HTuple hv_HeightImage, HTuple hv_WindowRow1, HTuple hv_WindowColumn1, HTuple hv_MapColorBarWidth, 
    HTuple hv_MarginBottom, HTuple *hv_WindowImageRatioHeight, HTuple *hv_WindowImageRatioWidth, 
    HTuple *hv_SetPartRow2, HTuple *hv_SetPartColumn2, HTuple *hv_PrevWindowCoordinatesOut)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv__, hv_WindowWidth, hv_WindowHeight;
  HTuple  hv_WindowRow2, hv_WindowColumn2, hv_WindowRatio;
  HTuple  hv_ImageRow2, hv_ImageColumn2, hv_ImageRatio, hv_ImageWindowRatioHeight;
  HTuple  hv_ImageRow2InWindow, hv_ImageCol2InWindow;

  //
  //This procedure sets and returns meta information to display images correctly.
  //
  //Set part for the image to be displayed later and adapt window size (+ MarginBottom + MapColorBarWidth).
  GetWindowExtents(hv_WindowHandle, &hv__, &hv__, &hv_WindowWidth, &hv_WindowHeight);
  (*hv_WindowImageRatioHeight) = hv_WindowHeight/(hv_HeightImage*1.0);
  (*hv_WindowImageRatioWidth) = hv_WindowWidth/(hv_WidthImage*1.0);
  //
  //Set window part such that image is displayed undistorted.
  hv_WindowRow2 = hv_WindowHeight;
  hv_WindowColumn2 = hv_WindowWidth;
  hv_WindowRatio = hv_WindowColumn2/(hv_WindowRow2*1.0);
  //
  hv_ImageRow2 = hv_HeightImage+(hv_MarginBottom/(*hv_WindowImageRatioHeight));
  hv_ImageColumn2 = hv_WidthImage+(hv_MapColorBarWidth/(*hv_WindowImageRatioWidth));
  hv_ImageRatio = hv_ImageColumn2/(hv_ImageRow2*1.0);
  if (0 != (int(hv_ImageRatio>hv_WindowRatio)))
  {
    //
    //Extend image until right window border.
    (*hv_SetPartColumn2) = hv_ImageColumn2;
    hv_ImageWindowRatioHeight = hv_ImageColumn2/(hv_WindowColumn2*1.0);
    hv_ImageRow2InWindow = hv_ImageRow2/hv_ImageWindowRatioHeight;
    (*hv_SetPartRow2) = hv_ImageRow2+((hv_WindowRow2-hv_ImageRow2InWindow)/(*hv_WindowImageRatioWidth));
  }
  else
  {
    //
    //Extend image until bottom of window.
    (*hv_SetPartRow2) = hv_ImageRow2;
    hv_ImageWindowRatioHeight = hv_ImageRow2/(hv_WindowRow2*1.0);
    hv_ImageCol2InWindow = hv_ImageColumn2/hv_ImageWindowRatioHeight;
    (*hv_SetPartColumn2) = hv_ImageColumn2+((hv_WindowColumn2-hv_ImageCol2InWindow)/(*hv_WindowImageRatioHeight));
  }
  if (HDevWindowStack::IsOpen())
    SetPart(HDevWindowStack::GetActive(),0, 0, (*hv_SetPartRow2)-1, (*hv_SetPartColumn2)-1);
  //
  //Return the coordinates of the new window.
  (*hv_PrevWindowCoordinatesOut).Clear();
  (*hv_PrevWindowCoordinatesOut).Append(hv_WindowRow1);
  (*hv_PrevWindowCoordinatesOut).Append(hv_WindowColumn1);
  (*hv_PrevWindowCoordinatesOut).Append(hv_WindowWidth);
  (*hv_PrevWindowCoordinatesOut).Append(hv_WindowHeight);
  //
  return;
}

// Local procedures 
// Short Description: Local example procedure for cleaning up files written by example script. 
void clean_up_output (HTuple hv_OutputDir, HTuple hv_RemoveResults)
{

  // Local iconic variables

  // Local control variables
  HTuple  hv_WindowHandle, hv_WarningCleanup;

  //This local example procedure cleans up the output of the example.

  if (0 != (hv_RemoveResults.TupleNot()))
  {
    return;
  }
  //Display a warning.
  SetWindowAttr("background_color","black");
  OpenWindow(0,0,600,300,0,"visible","",&hv_WindowHandle);
  HDevWindowStack::Push(hv_WindowHandle);
  set_display_font(hv_WindowHandle, 16, "mono", "true", "false");
  hv_WarningCleanup.Clear();
  hv_WarningCleanup[0] = "Congratulations, you have finished the example.";
  hv_WarningCleanup[1] = "";
  hv_WarningCleanup[2] = "Unless you would like to use the output data / model,";
  hv_WarningCleanup[3] = "press F5 to clean up.";
  if (HDevWindowStack::IsOpen())
    DispText(HDevWindowStack::GetActive(),hv_WarningCleanup, "window", "center", 
        "center", ((((HTuple("black").Append("black")).Append("coral")).Append("coral")).Append("coral")), 
        HTuple(), HTuple());

  // stop(...); only in hdevelop
  if (HDevWindowStack::IsOpen())
    CloseWindow(HDevWindowStack::Pop());

  //Delete all outputs of the example.
  remove_dir_recursively(hv_OutputDir);
  DeleteFile("model_best.hdl");
  DeleteFile("model_best_info.hdict");
  return;
}
