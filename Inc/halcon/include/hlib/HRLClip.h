/*****************************************************************************
 * HRLClip.h
 *****************************************************************************
 *
 * Project:     HALCON/libhalcon
 * Description: Rectangular clipping of regions (Hrlregion)
 *
 * (c) 1996-2023 by MVTec Software GmbH
 *                  www.mvtec.com
 *
 *****************************************************************************
 *
 *
 */


#ifndef HRLCLIP_H
#define HRLCLIP_H


#if defined(__cplusplus)
extern "C" {
#endif


extern HLibExport Herror HRLClipImsize(Hproc_handle proc_id,
                                       Hrlregion*   region);

extern HLibExport Herror HRLClipImsize2(Hproc_handle proc_id,
                                        Hrlregion* region, Hrlregion* clipped);

extern HLibExport Herror HRLClipp2(Hproc_handle proc_id, Hrlregion* in_out,
                                   int l1, int c1, int l2, int c2);


extern HLibExport Herror HRLRelClipp(Hproc_handle proc_id, Hrlregion* region,
                                     int top, int bottom, int left, int right,
                                     Hrlregion* clipped);

extern HLibExport Herror HRLClipp(Hproc_handle     proc_id,
                                  Hrlregion const* region_in, int l1, int c1,
                                  int l2, int c2, Hrlregion* clipped);

extern HLibExport Herror HRLHdiv(Hproc_handle     proc_id,
                                 Hrlregion const* Obj_ein, int horizontal,
                                 Hrlregion* tO_oben, Hrlregion* tO_unten);

extern HLibExport Herror HRLVdiv(Hproc_handle proc_id, Hrlregion* Obj_ein,
                                 int vertikale, Hrlregion* tO_links,
                                 Hrlregion* tO_rechts);

extern HLibExport Herror HRLDecomp(Hrlregion const* image_domain, int l1,
                                   int c1, int l2, int c2, Hrlregion* inner,
                                   Hrlregion* border);

extern HLibExport Herror HRCDecomp(Hvrlregion const* region, int l1, int c1,
                                   int l2, int c2, Hvrlregion* inner,
                                   Hvrlregion* outer);

extern HLibExport Herror HRLClipBottomRight(Hrlregion* H_RESTRICT region,
                                            HIMGCOOR max_l, HIMGCOOR max_c);

#if defined(__cplusplus)
}
#endif


#endif
