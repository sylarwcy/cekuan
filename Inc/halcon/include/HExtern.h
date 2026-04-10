/*****************************************************************************
 * HExtern.h
 *****************************************************************************
 *
 * Project:     HALCON/libhalcon
 * Description: External variables and some global procedures for
 *              language interfaces
 *
 * (c) 1996-2023 by MVTec Software GmbH
 *                  www.mvtec.com
 *
 *****************************************************************************/

#ifndef HEXTERN_H
#define HEXTERN_H

#if defined(__cplusplus)
extern "C" {
#endif

/* global variables for image processing                                     */
/* procedures description    */
extern HLibExport int     HDoLowError;        /* error messages of HALCON  */
extern HLibExport int32_t HLowErrorAction[4]; /* action for HALCON low  */
                                              /* level error messages   */
#ifdef _WIN32
extern HLibExport int32_t
    HDoMessageBoxOnError; /* allow error popup on windows */
#endif
extern HLibExport int HDoLicenseError;     /* license msgs of HALCON    */
extern HLibExport int HUseSpinLock;        /* sync initialization of
                                            * Parallel HALCON           */
extern HLibExport int HStartUpThreadPool;  /* start thread pool
                                            * initially? */
extern HLibExport int HShutdownThreadPool; /* shutdown thread pool
                                            * on termination? */
extern HLibExport int HShuttingDown;       /* currently terminating and
                                            * unloading HALCON Library? */

/* Convenience function accessors as some development environments */
/* do not directly support importing variables from C DLLs.        */

#ifdef _WIN32
extern HLibExport void __stdcall HSetDoMessageBoxOnError(int32_t value);
#endif
extern HLibExport void __stdcall HSetDoLicenseError(int value);
extern HLibExport void __stdcall HSetUseSpinLock(int value);
extern HLibExport void __stdcall HSetStartUpThreadPool(int value);
extern HLibExport void __stdcall HSetShutdownThreadPool(int value);

#ifdef _WIN32
extern HLibExport int32_t __stdcall HGetDoMessageBoxOnError(void);
#endif
extern HLibExport int __stdcall HGetDoLicenseError(void);
extern HLibExport int __stdcall HGetUseSpinLock(void);
extern HLibExport int __stdcall HGetStartUpThreadPool(void);
extern HLibExport int __stdcall HGetShutdownThreadPool(void);
extern HLibExport int __stdcall HGetShuttingDown(void);

/*
 * Memory allocator to use. eHMemoryAllocatorSystem is the system's default
 * memory allocator.
 */
typedef enum
{
  eHMemoryAllocatorTypeInvalid  = -1,
  eHMemoryAllocatorTypeSystem   = 0,
  eHMemoryAllocatorTypeMiMalloc = 1
} eHMemoryAllocatorType;

/*
 * Set and get the currently used memory allocator. These are intended to be
 * used to be able to set the memory allocator to use before HALCON is properyl
 * initialized - once a HALCON operator has been called, use the appropriate
 * get_system/set_system operator calls instead.
 */
extern HLibExport
    eHMemoryAllocatorType __stdcall HGetMemoryAllocatorType(void);
extern HLibExport
    Herror __stdcall HSetMemoryAllocatorType(eHMemoryAllocatorType allocator);

/* remark: the structure above contains global HALCON system variables       */
/* which are (in the normal case) exclusively accessible via a valid         */
/* procedure handle and by using specific functions of the HALCON-Lib        */
/* source code file HInstance.c; those functions contain synchronization     */
/* mechanisms to guarantee a correct access to the variables even in case of */
/* a parallel/multithreaded/reentrant HALCON version.                        */
/* Nevertheless the structure is propagated here so that it also can be used */
/* directly, but this should be done VERY CAREFULLY!! At the moment it is    */
/* only used directly within HDevelop, which means no problem, because       */
/* HDevelop works strictly sequentially, i.e. it calls all HALCON            */
/* operators one after another;                                              */

/* Functions to tell HALCON library the interface from which it gets accessed.
 * This enables the library to adapt its default behaviour.                  */
extern HLibExport void __stdcall HSetHDevelopInt(int AccessByHDevelop);
extern HLibExport int __stdcall HGetHDevelopInt(void);

/* Function to tell HALCON library about the console */
#ifdef WIN32
extern HLibExport void __stdcall HSetConsole(int ProcessId);
#endif

/* Function to register a license recheck failure callback. */
extern HLibExport void __stdcall HSetLicenseRecheckFailedCallback(
    HLicenseRecheckFailedCallback callback, void* context);

/* FinalizeHALCONLibrary prepares HALCON for finalization. It should be called
 * right before unloading the HALCON Library.
 * Return Value: H_ERR_FINI_USR_THREAD if there are still user threads running
 * that were using HALCON functionality. No resources are freed in this case.
 */
extern HLibExport int __stdcall FinalizeHALCONLibrary(void);

/* Callback interface for operators to signal their progress and current
 * status.  External applications can register to receive such updates using
 * set_system('add_progress_callback', CallbackFunction).
 *
 * The registered callback is called by selected operators to signal their
 * progress and supply an update in text form. The parameters of the callback
 * are:
 * - id: Operating system specific thread identifier. Note that with Automatic
 *       Operator Parallelization (AOP), the callback can be called from a
 *       different thread than the one used to call the operator. The id is
 *       thread independent and unique for the current operator call.
 *       It is the result of GetCurrentThreadId() (Windows) or pthread_self
 *       (UNIX) of the main thread used to call the operator.
 * - operator_name: Name of the operator that is executing
 * - progress: >=0: the current progress of the operator between 0 and 1.
 *                  Note that the progress is only an approximation.
 *             -1: No progress available (only a message)
 *             -2: the operator has finished execution. The application should
 *                 clean up any resources allocated for the operator.
 *                 Note that this call is done only for operators that
 *                 previously sent a progress update.
 * - message: If not NULL, a message about the operator's progress in english.
 */
typedef void(__stdcall* HProgressBarCallback)(Hlong       id,
                                              const char* operator_name,
                                              double      progress,
                                              const char* message);

/*
 * Verify XML file signed with hsigntool
 * path is a pointer to a UTF-8 encoded path to the XML file to sign.
 * pk is a pointer to a buffer of 32 byte length containing the public key
 * corresponding to the private key used to sign the file,
 */
extern HLibExport Herror HVerifyXMLFile(const char* path, void const* pk);

#if defined(__cplusplus)
}
#endif

#endif
