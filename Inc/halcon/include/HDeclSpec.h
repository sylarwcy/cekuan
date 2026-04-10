/*****************************************************************************
 * HDeclSpec.h
 *****************************************************************************
 *
 * Project:     HALCON/libhalcon
 * Description: Windows DLL specifications
 *
 * (c) 1996-2023 by MVTec Software GmbH
 *                  www.mvtec.com
 *
 *****************************************************************************/

#ifndef HALCON_DECL_SPEC_H
#define HALCON_DECL_SPEC_H

/* HALCON library. If _HLibStatic is defined, HALCON is a static library. */
#if defined(_HLibStatic)
#  define HLibExport
#elif defined(_WIN32) && !defined(_NODLL)
#  if defined(_HLibDLL)
#    define HLibExport __declspec(dllexport)
#  else
#    define HLibExport __declspec(dllimport)
#  endif
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#  define HLibExport __attribute__((visibility("default")))
#else
#  define HLibExport
#endif

/*
 * HALCON language interfaces. If _LIntExport is defined, the language
 * interfaces are static libraries.
 */
#if defined(_LIntStatic)
#  define LIntExport
#elif defined(_WIN32) && !defined(_NODLL)
#  if defined(_LIntDLL)
#    define LIntExport __declspec(dllexport)
#  else
#    define LIntExport __declspec(dllimport)
#  endif
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#  define LIntExport __attribute__((visibility("default")))
#else
#  define LIntExport
#endif

/* Extension packages. These are always shared libraries / DLLs */
#if defined(_WIN32)
#  define HUserExport __declspec(dllexport)
#  define HUserImport __declspec(dllimport)
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#  define HUserExport __attribute__((visibility("default")))
#  define HUserImport
#else
#  define HUserExport
#  define HUserImport
#endif

#endif
