//
// Created by wcy on 25-7-28.
//

#ifndef HALCONINFER_H
#define HALCONINFER_H

#include "halconcpp/HalconCpp.h"
#include "halconcpp/HDevThread.h"

using namespace HalconCpp;

// Procedure declarations
// External procedures
// Chapter: Image / Channel
void add_colormap_to_image (HObject ho_GrayValueImage, HObject ho_Image, HObject *ho_ColoredImage,
    HTuple hv_HeatmapColorScheme);
// Chapter: Image / Channel
// Short Description: Create a lookup table and convert a gray scale image.
void apply_colorscheme_on_gray_value_image (HObject ho_InputImage, HObject *ho_ResultImage,
    HTuple hv_Schema);
// Chapter: Deep Learning / Model
// Short Description: Compute zoom factors to fit an image to a target size.
void calculate_dl_image_zoom_factors (HTuple hv_ImageWidth, HTuple hv_ImageHeight,
    HTuple hv_TargetWidth, HTuple hv_TargetHeight, HTuple hv_DLPreprocessParam, HTuple *hv_ZoomFactorWidth,
    HTuple *hv_ZoomFactorHeight);
// Chapter: Deep Learning / Model
// Short Description: Check the content of the parameter dictionary DLPreprocessParam.
void check_dl_preprocess_param (HTuple hv_DLPreprocessParam);
// Chapter: Deep Learning / Model
// Short Description: Compute 3D normals.
void compute_normals_xyz (HObject ho_x, HObject ho_y, HObject ho_z, HObject *ho_NXImage,
    HObject *ho_NYImage, HObject *ho_NZImage, HTuple hv_Smoothing);
// Chapter: OCR / Deep OCR
// Short Description: This procedure converts Deep OCR Detection results to an Object Detection results.
void convert_ocr_detection_result_to_object_detection (HTuple hv_OcrResults, HTuple *hv_DetectionResults);
// Chapter: Tools / Geometry
// Short Description: Convert the parameters of rectangles with format rectangle2 to the coordinates of its 4 corner-points.
void convert_rect2_5to8param (HTuple hv_Row, HTuple hv_Col, HTuple hv_Length1, HTuple hv_Length2,
    HTuple hv_Phi, HTuple *hv_Row1, HTuple *hv_Col1, HTuple *hv_Row2, HTuple *hv_Col2,
    HTuple *hv_Row3, HTuple *hv_Col3, HTuple *hv_Row4, HTuple *hv_Col4);
// Chapter: Tools / Geometry
// Short Description: Convert for four-sided figures the coordinates of the 4 corner-points to the parameters of format rectangle2.
void convert_rect2_8to5param (HTuple hv_Row1, HTuple hv_Col1, HTuple hv_Row2, HTuple hv_Col2,
    HTuple hv_Row3, HTuple hv_Col3, HTuple hv_Row4, HTuple hv_Col4, HTuple hv_ForceL1LargerL2,
    HTuple *hv_Row, HTuple *hv_Col, HTuple *hv_Length1, HTuple *hv_Length2, HTuple *hv_Phi);
// Chapter: Deep Learning / Model
// Short Description: Crops a given image object based on the given domain handling.
void crop_dl_sample_image (HObject ho_Domain, HTuple hv_DLSample, HTuple hv_Key,
    HTuple hv_DLPreprocessParam);
// Chapter: Graphics / Window
// Short Description: Close all window handles contained in a dictionary.
void dev_close_window_dict (HTuple hv_WindowHandleDict);
// Chapter: Graphics / Output
// Short Description: Display a map of the confidences.
void dev_display_confidence_regions (HObject ho_ImageConfidence, HTuple hv_DrawTransparency,
    HTuple *hv_Colors);
// Chapter: Deep Learning / Model
// Short Description: Visualize different images, annotations and inference results for a sample.
void dev_display_dl_data (HTuple hv_DLSample, HTuple hv_DLResult, HTuple hv_DLDatasetInfo,
    HTuple hv_KeysForDisplay, HTuple hv_GenParam, HTuple hv_WindowHandleDict);
// Chapter: Deep Learning / Model
// Short Description: Try to guess the maximum class id based on the given sample/result.
void dev_display_dl_data_get_max_class_id (HTuple hv_DLSample, HTuple *hv_MaxClassId,
    HTuple *hv_Empty);
// Chapter: Deep Learning / Anomaly Detection and Global Context Anomaly Detection
// Short Description: Display the ground truth anomaly regions of the given DLSample.
void dev_display_ground_truth_anomaly_regions (HTuple hv_SampleKeys, HTuple hv_DLSample,
    HTuple hv_CurrentWindowHandle, HTuple hv_LineWidth, HTuple hv_AnomalyRegionLabelColor,
    HTuple hv_AnomalyColorTransparency, HTuple *hv_AnomalyRegionExists);
// Chapter: Graphics / Output
// Short Description: Display the ground truth bounding boxes of DLSample.
void dev_display_ground_truth_detection (HTuple hv_DLSample, HTuple hv_SampleKeys,
    HTuple hv_LineWidthBbox, HTuple hv_ClassIDs, HTuple hv_BboxColors, HTuple hv_BboxLabelColor,
    HTuple hv_WindowImageRatio, HTuple hv_TextColor, HTuple hv_ShowLabels, HTuple hv_ShowDirection,
    HTuple hv_WindowHandle, HTuple *hv_BboxIDs);
// Chapter: Graphics / Output
// Short Description: Display a color bar next to an image.
void dev_display_map_color_bar (HTuple hv_ImageWidth, HTuple hv_ImageHeight, HTuple hv_MapColorBarWidth,
    HTuple hv_Colors, HTuple hv_MaxValue, HTuple hv_WindowImageRatio, HTuple hv_WindowHandle);
// Chapter: Deep Learning / Anomaly Detection and Global Context Anomaly Detection
// Short Description: Display the detected anomaly regions.
void dev_display_result_anomaly_regions (HObject ho_AnomalyRegion, HTuple hv_CurrentWindowHandle,
    HTuple hv_LineWidth, HTuple hv_AnomalyRegionResultColor);
// Chapter: Graphics / Output
// Short Description: Display result bounding boxes.
void dev_display_result_detection (HTuple hv_DLResult, HTuple hv_ResultKeys, HTuple hv_LineWidthBbox,
    HTuple hv_ClassIDs, HTuple hv_TextConf, HTuple hv_Colors, HTuple hv_BoxLabelColor,
    HTuple hv_WindowImageRatio, HTuple hv_TextPositionRow, HTuple hv_TextColor, HTuple hv_ShowLabels,
    HTuple hv_ShowDirection, HTuple hv_WindowHandle, HTuple *hv_BboxClassIndices);
// Chapter: Graphics / Output
// Short Description: Display the ground truth/result segmentation as regions.
void dev_display_segmentation_regions (HObject ho_SegmentationImage, HTuple hv_ClassIDs,
    HTuple hv_ColorsSegmentation, HTuple hv_ExcludeClassIDs, HTuple *hv_ImageClassIDs);
// Chapter: Graphics / Output
// Short Description: Display a map of weights.
void dev_display_weight_regions (HObject ho_ImageWeight, HTuple hv_DrawTransparency,
    HTuple hv_SegMaxWeight, HTuple *hv_Colors);
// Chapter: Develop
// Short Description: Open a new graphics window that preserves the aspect ratio of the given image size.
void dev_open_window_fit_size (HTuple hv_Row, HTuple hv_Column, HTuple hv_Width,
    HTuple hv_Height, HTuple hv_WidthLimit, HTuple hv_HeightLimit, HTuple *hv_WindowHandle);
// Chapter: Develop
// Short Description: Switch dev_update_pc, dev_update_var, and dev_update_window to 'off'.
void dev_update_off ();
// Chapter: Deep Learning / Object Detection and Instance Segmentation
// Short Description: Filter the instance segmentation masks of a DL sample based on a given selection.
void filter_dl_sample_instance_segmentation_masks (HTuple hv_DLSample, HTuple hv_BBoxSelectionMask);
// Chapter: XLD / Creation
// Short Description: Create an arrow shaped XLD contour.
void gen_arrow_contour_xld (HObject *ho_Arrow, HTuple hv_Row1, HTuple hv_Column1,
    HTuple hv_Row2, HTuple hv_Column2, HTuple hv_HeadLength, HTuple hv_HeadWidth);
// Chapter: OCR / Deep OCR
// Short Description: Generate ground truth characters if they don't exist and words to characters mapping.
void gen_dl_ocr_detection_gt_chars (HTuple hv_DLSampleTargets, HTuple hv_DLSample,
    HTuple hv_ScaleWidth, HTuple hv_ScaleHeight, HTupleVector/*{eTupleVector,Dim=1}*/ *hvec_WordsCharsMapping);
// Chapter: OCR / Deep OCR
// Short Description: Generate target link score map for ocr detection training.
void gen_dl_ocr_detection_gt_link_map (HObject *ho_GtLinkMap, HTuple hv_ImageWidth,
    HTuple hv_ImageHeight, HTuple hv_DLSampleTargets, HTupleVector/*{eTupleVector,Dim=1}*/ hvec_WordToCharVec,
    HTuple hv_Alpha);
// Chapter: OCR / Deep OCR
// Short Description: Generate target orientation score maps for ocr detection training.
void gen_dl_ocr_detection_gt_orientation_map (HObject *ho_GtOrientationMaps, HTuple hv_ImageWidth,
    HTuple hv_ImageHeight, HTuple hv_DLSample);
// Chapter: OCR / Deep OCR
// Short Description: Generate target text score map for ocr detection training.
void gen_dl_ocr_detection_gt_score_map (HObject *ho_TargetText, HTuple hv_DLSample,
    HTuple hv_BoxCutoff, HTuple hv_RenderCutoff, HTuple hv_ImageWidth, HTuple hv_ImageHeight);
// Chapter: OCR / Deep OCR
// Short Description: Preprocess dl samples and generate targets and weights for ocr detection training.
void gen_dl_ocr_detection_targets (HTuple hv_DLSampleOriginal, HTuple hv_DLPreprocessParam);
// Chapter: OCR / Deep OCR
// Short Description: Generate link score map weight for ocr detection training.
void gen_dl_ocr_detection_weight_link_map (HObject ho_LinkMap, HObject ho_TargetWeight,
    HObject *ho_TargetWeightLink, HTuple hv_LinkZeroWeightRadius);
// Chapter: OCR / Deep OCR
// Short Description: Generate orientation score map weight for ocr detection training.
void gen_dl_ocr_detection_weight_orientation_map (HObject ho_InitialWeight, HObject *ho_OrientationTargetWeight,
    HTuple hv_DLSample);
// Chapter: OCR / Deep OCR
// Short Description: Generate text score map weight for ocr detection training.
void gen_dl_ocr_detection_weight_score_map (HObject *ho_TargetWeightText, HTuple hv_ImageWidth,
    HTuple hv_ImageHeight, HTuple hv_DLSample, HTuple hv_BoxCutoff, HTuple hv_WSWeightRenderThreshold,
    HTuple hv_Confidence);
// Chapter: Deep Learning / Model
// Short Description: Store the given images in a tuple of dictionaries DLSamples.
void gen_dl_samples_from_images (HObject ho_Images, HTuple *hv_DLSampleBatch);
// Chapter: OCR / Deep OCR
// Short Description: Generate a word to characters mapping.
void gen_words_chars_mapping (HTuple hv_DLSample, HTupleVector/*{eTupleVector,Dim=1}*/ *hvec_WordsCharsMapping);
// Chapter: Deep Learning / Anomaly Detection and Global Context Anomaly Detection
// Short Description: Get the ground truth anomaly label and label ID.
void get_anomaly_ground_truth_label (HTuple hv_SampleKeys, HTuple hv_DLSample, HTuple *hv_AnomalyLabelGroundTruth,
    HTuple *hv_AnomalyLabelIDGroundTruth);
// Chapter: Deep Learning / Anomaly Detection and Global Context Anomaly Detection
// Short Description: Get the anomaly results out of DLResult and apply thresholds (if specified).
void get_anomaly_result (HObject *ho_AnomalyImage, HObject *ho_AnomalyRegion, HTuple hv_DLResult,
    HTuple hv_AnomalyClassThreshold, HTuple hv_AnomalyRegionThreshold, HTuple hv_AnomalyResultPostfix,
    HTuple *hv_AnomalyScore, HTuple *hv_AnomalyClassID, HTuple *hv_AnomalyClassThresholdDisplay,
    HTuple *hv_AnomalyRegionThresholdDisplay);
// Chapter: Graphics / Window
// Short Description: Get the next child window that can be used for visualization.
void get_child_window (HTuple hv_HeightImage, HTuple hv_Font, HTuple hv_FontSize,
    HTuple hv_Text, HTuple hv_PrevWindowCoordinates, HTuple hv_WindowHandleDict,
    HTuple hv_WindowHandleKey, HTuple *hv_WindowImageRatio, HTuple *hv_PrevWindowCoordinatesOut);
// Chapter: Deep Learning / Classification
// Short Description: Get the ground truth classification label id.
void get_classification_ground_truth (HTuple hv_SampleKeys, HTuple hv_DLSample, HTuple *hv_ClassificationLabelIDGroundTruth);
// Chapter: Deep Learning / Classification
// Short Description: Get the predicted classification class ID.
void get_classification_result (HTuple hv_ResultKeys, HTuple hv_DLResult, HTuple *hv_ClassificationClassID);
// Chapter: Deep Learning / Semantic Segmentation and Edge Extraction
// Short Description: Get the confidences of the segmentation result.
void get_confidence_image (HObject *ho_ImageConfidence, HTuple hv_ResultKeys, HTuple hv_DLResult);
// Chapter: Deep Learning / Model
// Short Description: Generate NumColors distinct colors
void get_distinct_colors (HTuple hv_NumColors, HTuple hv_Random, HTuple hv_StartColor,
    HTuple hv_EndColor, HTuple *hv_Colors);
// Chapter: Deep Learning / Model
// Short Description: Generate certain colors for different ClassNames
void get_dl_class_colors (HTuple hv_ClassNames, HTuple hv_AdditionalGreenClassNames,
    HTuple *hv_Colors);
// Chapter: Deep Learning / Model
// Short Description: Get an image of a sample with a certain key.
void get_dl_sample_image (HObject *ho_Image, HTuple hv_SampleKeys, HTuple hv_DLSample,
    HTuple hv_Key);
// Chapter: 3D Matching / 3D Gripping Point Detection
// Short Description: Extract gripping points from a dictionary.
void get_gripping_points_from_dict (HTuple hv_DLResult, HTuple *hv_Rows, HTuple *hv_Columns);
// Chapter: Graphics / Window
// Short Description: Get the next window that can be used for visualization.
void get_next_window (HTuple hv_Font, HTuple hv_FontSize, HTuple hv_ShowBottomDesc,
    HTuple hv_WidthImage, HTuple hv_HeightImage, HTuple hv_MapColorBarWidth, HTuple hv_ScaleWindows,
    HTuple hv_ThresholdWidth, HTuple hv_PrevWindowCoordinates, HTuple hv_WindowHandleDict,
    HTuple hv_WindowHandleKey, HTuple *hv_CurrentWindowHandle, HTuple *hv_WindowImageRatioHeight,
    HTuple *hv_PrevWindowCoordinatesOut);
// Chapter: Deep Learning / Semantic Segmentation and Edge Extraction
// Short Description: Get the ground truth segmentation image.
void get_segmentation_image_ground_truth (HObject *ho_SegmentationImagGroundTruth,
    HTuple hv_SampleKeys, HTuple hv_DLSample);
// Chapter: Deep Learning / Semantic Segmentation and Edge Extraction
// Short Description: Get the predicted segmentation result image.
void get_segmentation_image_result (HObject *ho_SegmentationImageResult, HTuple hv_ResultKeys,
    HTuple hv_DLResult);
// Chapter: Deep Learning / Semantic Segmentation and Edge Extraction
// Short Description: Get the weight image of a sample.
void get_weight_image (HObject *ho_ImageWeight, HTuple hv_SampleKeys, HTuple hv_DLSample);
// Chapter: File / Misc
// Short Description: Get all image files under the given path
void list_image_files (HTuple hv_ImageDirectory, HTuple hv_Extensions, HTuple hv_Options,
    HTuple *hv_ImageFiles);
// Chapter: Deep Learning / Model
// Short Description: Shuffle the input colors in a deterministic way
void make_neighboring_colors_distinguishable (HTuple hv_ColorsRainbow, HTuple *hv_Colors);
// Chapter: Graphics / Window
// Short Description: Open a window next to the given WindowHandleFather.
void open_child_window (HTuple hv_WindowHandleFather, HTuple hv_Font, HTuple hv_FontSize,
    HTuple hv_Text, HTuple hv_PrevWindowCoordinates, HTuple hv_WindowHandleDict,
    HTuple hv_WindowHandleKey, HTuple *hv_WindowHandleChild, HTuple *hv_PrevWindowCoordinatesOut);
// Chapter: Graphics / Window
// Short Description: Open a new window, either next to the last ones, or in a new row.
void open_next_window (HTuple hv_Font, HTuple hv_FontSize, HTuple hv_ShowBottomDesc,
    HTuple hv_WidthImage, HTuple hv_HeightImage, HTuple hv_MapColorBarWidth, HTuple hv_ScaleWindows,
    HTuple hv_ThresholdWidth, HTuple hv_PrevWindowCoordinates, HTuple hv_WindowHandleDict,
    HTuple hv_WindowHandleKey, HTuple *hv_WindowHandleNew, HTuple *hv_WindowImageRatioHeight,
    HTuple *hv_PrevWindowCoordinatesOut);
// Chapter: Deep Learning / Model
// Short Description: Preprocess 3D data for deep-learning-based training and inference.
void preprocess_dl_model_3d_data (HTuple hv_DLSample, HTuple hv_DLPreprocessParam);
// Chapter: Deep Learning / Model
// Short Description: Preprocess anomaly images for evaluation and visualization of deep-learning-based anomaly detection or Global Context Anomaly Detection.
void preprocess_dl_model_anomaly (HObject ho_AnomalyImages, HObject *ho_AnomalyImagesPreprocessed,
    HTuple hv_DLPreprocessParam);
// Chapter: Deep Learning / Model
// Short Description: Preprocess the provided DLSample image for augmentation purposes.
void preprocess_dl_model_augmentation_data (HTuple hv_DLSample, HTuple hv_DLPreprocessParam);
// Chapter: Deep Learning / Object Detection and Instance Segmentation
// Short Description: Preprocess the bounding boxes of type 'rectangle1' for a given sample.
void preprocess_dl_model_bbox_rect1 (HObject ho_ImageRaw, HTuple hv_DLSample, HTuple hv_DLPreprocessParam);
// Chapter: Deep Learning / Object Detection and Instance Segmentation
// Short Description: Preprocess the bounding boxes of type 'rectangle2' for a given sample.
void preprocess_dl_model_bbox_rect2 (HObject ho_ImageRaw, HTuple hv_DLSample, HTuple hv_DLPreprocessParam);
// Chapter: Deep Learning / Model
// Short Description: Preprocess images for deep-learning-based training and inference.
void preprocess_dl_model_images (HObject ho_Images, HObject *ho_ImagesPreprocessed,
    HTuple hv_DLPreprocessParam);
// Chapter: OCR / Deep OCR
// Short Description: Preprocess images for deep-learning-based training and inference of Deep OCR detection models.
void preprocess_dl_model_images_ocr_detection (HObject ho_Images, HObject *ho_ImagesPreprocessed,
    HTuple hv_DLPreprocessParam);
// Chapter: OCR / Deep OCR
// Short Description: Preprocess images for deep-learning-based training and inference of Deep OCR recognition models.
void preprocess_dl_model_images_ocr_recognition (HObject ho_Images, HObject *ho_ImagesPreprocessed,
    HTuple hv_DLPreprocessParam);
// Chapter: Deep Learning / Object Detection and Instance Segmentation
// Short Description: Preprocess the instance segmentation masks for a sample given by the dictionary DLSample.
void preprocess_dl_model_instance_masks (HObject ho_ImageRaw, HTuple hv_DLSample,
    HTuple hv_DLPreprocessParam);
// Chapter: Deep Learning / Semantic Segmentation and Edge Extraction
// Short Description: Preprocess segmentation and weight images for deep-learning-based segmentation training and inference.
void preprocess_dl_model_segmentations (HObject ho_ImagesRaw, HObject ho_Segmentations,
    HObject *ho_SegmentationsPreprocessed, HTuple hv_DLPreprocessParam);
// Chapter: Deep Learning / Model
// Short Description: Preprocess given DLSamples according to the preprocessing parameters given in DLPreprocessParam.
void preprocess_dl_samples (HTuple hv_DLSampleBatch, HTuple hv_DLPreprocessParam);
// Chapter: Image / Manipulation
// Short Description: Change value of ValuesToChange in Image to NewValue.
void reassign_pixel_values (HObject ho_Image, HObject *ho_ImageOut, HTuple hv_ValuesToChange,
    HTuple hv_NewValue);
// Chapter: File / Misc
// Short Description: Remove a directory recursively.
void remove_dir_recursively (HTuple hv_DirName);
// Chapter: Deep Learning / Model
// Short Description: Remove invalid 3D pixels from a given domain.
void remove_invalid_3d_pixels (HObject ho_ImageX, HObject ho_ImageY, HObject ho_ImageZ,
    HObject ho_Domain, HObject *ho_DomainOut, HTuple hv_InvalidPixelValue);
// Chapter: Deep Learning / Model
// Short Description: Replace legacy preprocessing parameters or values.
void replace_legacy_preprocessing_parameters (HTuple hv_DLPreprocessParam);
// Chapter: Filters / Arithmetic
// Short Description: Scale the gray values of an image from the interval [Min,Max] to [0,255]
void scale_image_range (HObject ho_Image, HObject *ho_ImageScaled, HTuple hv_Min,
    HTuple hv_Max);
// Chapter: Graphics / Text
// Short Description: Set font independent of OS
void set_display_font (HTuple hv_WindowHandle, HTuple hv_Size, HTuple hv_Font, HTuple hv_Bold,
    HTuple hv_Slant);
// Chapter: OCR / Deep OCR
// Short Description: Split rectangle2 into a number of rectangles.
void split_rectangle2 (HTuple hv_Row, HTuple hv_Column, HTuple hv_Phi, HTuple hv_Length1,
    HTuple hv_Length2, HTuple hv_NumSplits, HTuple *hv_SplitRow, HTuple *hv_SplitColumn,
    HTuple *hv_SplitPhi, HTuple *hv_SplitLength1Out, HTuple *hv_SplitLength2Out);
// Chapter: Graphics / Window
// Short Description: Set and return meta information to display images correctly.
void update_window_meta_information (HTuple hv_WindowHandle, HTuple hv_WidthImage,
    HTuple hv_HeightImage, HTuple hv_WindowRow1, HTuple hv_WindowColumn1, HTuple hv_MapColorBarWidth,
    HTuple hv_MarginBottom, HTuple *hv_WindowImageRatioHeight, HTuple *hv_WindowImageRatioWidth,
    HTuple *hv_SetPartRow2, HTuple *hv_SetPartColumn2, HTuple *hv_PrevWindowCoordinatesOut);
// Local procedures
// Short Description: Local example procedure for cleaning up files written by example script.
void clean_up_output (HTuple hv_OutputDir, HTuple hv_RemoveResults);

#endif //HALCONINFER_H
