/*****************************************************************************
 * HInstance.h
 *****************************************************************************
 *
 * Project:      HALCON/HLib
 * Description:  handling of HALCON operator instances and HALCON threads;
 *               provides procedural access to global context variables;
 *
 * (c) 1996-2023 by MVTec Software GmbH
 *                  www.mvtec.com
 *
 *****************************************************************************/


#ifndef HINSTANCE_H
#define HINSTANCE_H


#if defined(__cplusplus)
extern "C" {
#endif


#include "hlib/HInstanceDefines.h"


/* HALCON library already initialized? */
extern HLibExport bool IsInitHlib(void);

/* HAccessGlVar - access to global HALCON context variables                  */
extern HLibExport Herror HAccessGlVar(Hproc_handle proc_handle, int element,
                                      uint8_t to_do, void* r_value,
                                      double w_value, char* w_string,
                                      Hkey w_key, INT4_8 index);

/* HWriteGlUtf8Var - write a string with encoding to the global HALCON
 * context and convert it if necessary from hlib encoding to utf-8 */
extern HLibExport Herror HWriteGlUtf8Var(Hproc_handle ph, int element,
                                         char const* str_hlib);

/* HReadGlUtf8Var - read a string with encoding from the global HALCON
 *  context and convert if necessary from utf-8 to hlib encoding
 *  when do_free is set to true by the called function, the result string was
 *  allocated in local memory and must be freed by the caller,
 *  if the caller passes NULL for do_free, it must always be freed */
extern HLibExport Herror HReadGlUtf8Var(Hproc_handle ph, int element,
                                        char** str_p, bool* do_free);

/* HCheckInpObjNum - check whether specified object number is in range       */
extern HLibExport bool HCheckInpObjNum(Hproc_handle proc_handle, int par_num,
                                       INT4_8 obj_num);
extern HLibExport bool HCheckObjNum(Hproc_handle proc_handle, int par_num,
                                    INT4_8 obj_num);
extern HLibExport bool HCheckOutpObjNum(Hproc_handle proc_handle, int par_num,
                                        INT4_8 obj_num);
extern HLibExport Hproc_handle HTSProcHandle(void);
extern HLibExport Hproc_handle HTSHProcIndepend(int op_id);

extern HLibExport void*  HCreateTSPSysInfoHandle(void);
extern HLibExport Herror HPushTSPSysInfoHandle(void* handle);
extern HLibExport void*  HPopTSPSysInfoHandle(void);
extern HLibExport Herror HClearTSPSysInfoHandle(void* handle);

/* HGetHProc - get a thread specific HALCON operator call instance (no full
 * use) */
extern HLibExport Herror HGetHProc(int32_t proc_index, Hproc_handle* ph);
/* HPutHProc - return a thread specific HALCON operator call instance to Core*/
extern HLibExport Herror HPutHProc(Hproc_handle proc_handle);
/* HGetRecHProc - get a thread specific HALCON operator call instance, able
 * to handle recursive calls  (no full use)  */
extern HLibExport Herror HGetRecHProc(int32_t proc_index, Hproc_handle* ph);
/* HPutRecHProc - return a thread specific (recursiveable) HALCON operator
 * call instance */
extern HLibExport Herror HPutRecHProc(Hproc_handle proc_handle);
/* HPutRecHProcErr - return a thread specific (recursiveable) HALCON operator
 * call instance, where the operator returned with the given error */
extern HLibExport Herror HPutRecHProcErr(Hproc_handle ph, Herror err);
/* HCreateHProc - create a new HALCON operator call instance                 */
extern HLibExport Herror HCreateHProc(int32_t       current_idx,
                                      Hproc_handle* proc_handle);
/* HDeleteHProc - delete a HALCON operator call instance                     */
extern HLibExport Herror HDeleteHProc(Hproc_handle proc_handle, bool last,
                                      Herror proc_result);

extern HLibExport bool HOpIdIsValid(int opid);

extern HLibExport int HGVGet_NumSysOp(void); /* number of system operators*/
extern HLibExport int HGVGet_NumOp(void);    /* total number of operators */
extern HLibExport char const* HGVGet_Language(void);
extern HLibExport int         HGVGet_locale_filename_encoding(void);
extern HLibExport uint32_t    HGVGet_parallel(void);
extern HLibExport bool        HGVGet_CPUFeature(int feature);
extern HLibExport void        HGVSet_CPUFeature(int feature, bool value);
extern HLibExport INT4_8      HGVGet_CPUCache(int level);

extern HLibExport char const* HOIGet_InpCtrlParType(int opid);
extern HLibExport char const* HOIGet_OutpCtrlParType(int opid);

extern HLibExport int         HOIGet_NumInpCtrlPar(int opid);
extern HLibExport int         HOIGet_NumOutpCtrlPar(int opid);
extern HLibExport int         HOIGet_NumInpObjPar(int opid);
extern HLibExport int         HOIGet_NumOutpObjPar(int opid);
extern HLibExport char const* HOIGet_LogicalName(int opid);

extern HLibExport int         HPHGet_NumInpCtrlPar(Hproc_handle ph);
extern HLibExport int         HPHGet_NumOutpCtrlPar(Hproc_handle ph);
extern HLibExport int         HPHGet_NumInpObjPar(Hproc_handle ph);
extern HLibExport int         HPHGet_NumOutpObjPar(Hproc_handle ph);
extern HLibExport int32_t     HPHGet_proc_index(Hproc_handle ph);
extern HLibExport const char* HPHGet_LogicalName(Hproc_handle ph);
extern HLibExport int32_t     HPHGet_check(Hproc_handle ph);
extern HLibExport INT4_8      HPHGet_PreviousMaxAlloc(Hproc_handle ph);

extern bool   HGetInstanceMemCacheMode(void);
extern Herror HSetInstanceMemCacheMode(Hproc_handle ph, bool mode);

/* HProcSetStop sets a stop-flag for all running operator instances
 * (asynchronously). Opertors supporting this flag will break execution and
 * return (without an error) */
extern HLibExport Herror HProcSetStop(int mode);

/* HProcBreak sets a beak flag for all running operator instances
 * (asynchronously) */
extern HLibExport Herror HProcBreak(void);

#if defined(__cplusplus)
}
#endif


#endif
