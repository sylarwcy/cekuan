/*****************************************************************************
 * IOSpy.h
 *****************************************************************************
 *
 * Project:     HALCON/libhalcon
 * Description: Spy procedures
 *
 * (c) 1996-2023 by MVTec Software GmbH
 *                  www.mvtec.com
 *
 *****************************************************************************
 *
 *
 */


#ifndef IOSPY_H
#define IOSPY_H


#if defined(__cplusplus)
extern "C" {
#endif


extern HLibExport Herror IOSetSpyByEnv(Hproc_handle proc_id, char* env);

extern HLibExport Herror IOSetSpy(Hproc_handle proc_id, char const* task,
                                  Hpar const* par, int type);

extern HLibExport Herror IOSetSpyTime(Hproc_handle proc_id, bool mode);

extern HLibExport Herror IOGetSpyTime(Hproc_handle proc_id, bool* mode);

extern HLibExport Herror IOSpyDB(Hproc_handle proc_handle);

extern HLibExport Herror IOSetSpyNumPar(Hproc_handle proc_id, int num);

extern HLibExport Herror IOGetSpyNumPar(Hproc_handle proc_id, int* num);

extern HLibExport Herror IOSetSpyPredecessor(Hproc_handle proc_id, bool state);

extern HLibExport Herror IOGetSpyPredecessor(Hproc_handle proc_id,
                                             bool*        state);

extern HLibExport Herror IOSpyPrintPredecessor(Hproc_handle proc_id,
                                               const char* kind, int pred,
                                               int curr);

extern HLibExport Herror IOSetSpyProcCall(Hproc_handle proc_id, bool state);

extern HLibExport Herror IOGetSpyProcCall(Hproc_handle proc_id, bool* state);
extern HLibExport bool   IOGetSpyProcCallSwitch(void);

extern HLibExport Herror IOSetSpyFile(Hproc_handle proc_handle,
                                      Hphandle     file_id);

extern HLibExport Herror IOGetSpyFile(Hproc_handle proc_handle,
                                      Hphandle*    file_id);

extern HLibExport Herror IOGetSpyProc(Hproc_handle proc_id, bool* proc,
                                      bool* input_ctrl, bool* output_ctrl);

extern HLibExport Herror IOSetSpyProc(Hproc_handle proc_id, bool proc,
                                      bool input_ctrl, bool output_ctrl);

extern HLibExport Herror IOSpyProcEnd(void);

extern HLibExport Herror IOSpyProc(Hproc_handle proc_id, const char* name);

extern HLibExport Herror IOSpyPar(Hproc_handle proc_handle, int par, int type,
                                  Hpar const* val, INT4_8 num, bool input);

extern HLibExport Herror IOSpyCPar(Hproc_handle proc_handle, int par,
                                   Hcpar const* val, INT4_8 num, bool input);

extern HLibExport Herror IOSpyElem(Hproc_handle proc_handle, int par,
                                   void const* val, INT4_8 num, int type,
                                   bool input);

extern HLibExport Herror IOSpyCTuple(Hproc_handle proc_handle, int par,
                                     Hctuple const* val, bool input);

extern HLibExport Herror IOSetSpyError(Hproc_handle proc_id, bool state);

extern HLibExport Herror IOGetSpyError(Hproc_handle proc_id, bool* state);

extern HLibExport Herror HProcessErr(const char* proc, Herror err, INT4_8 line,
                                     const char* file);

extern HLibExport void IOSpyProcCall(const char* proc, INT4_8 line,
                                     const char* file);


#if defined(__cplusplus)
}
#endif


#endif
