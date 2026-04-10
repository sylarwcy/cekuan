/*****************************************************************************
 * HError.h
 *****************************************************************************
 *
 * Project:     HALCON/libhalcon
 * Description: Error messages
 *
 * (c) 1996-2023 by MVTec Software GmbH
 *                  www.mvtec.com
 *
 *****************************************************************************
 *
 *
 */


#ifndef HERROR_H
#define HERROR_H


#if defined(__cplusplus)
extern "C" {
#endif

extern HLibExport Herror HSetExtendedErrorInfo(Hproc_handle ph,
                                               INT4_8       error_code,
                                               char const*  error_message);

#if defined(__GNUC__) || defined(__clang__)
/*
 * Tell gcc/clang that HSetExtendedErrorInfoF takes a printf-like format
 * string, allowing the compiler to check if the arguments match the format.
 */
__attribute__((format(printf, 3, 4)))
#endif
extern HLibExport Herror
HSetExtendedErrorInfoF(Hproc_handle ph, INT4_8 error_code,
                       char const* error_message, ...);

extern HLibExport Herror HClearExtendedErrorInfo(Hproc_handle ph);

extern HLibExport Herror HSetErrText(char* text);

extern HLibExport Herror HErrText(Herror message, char* err_text);

extern HLibExport Herror HErrorMessage(Herror message, char* msg_text);


#if defined(__cplusplus)
}
#endif


#endif
