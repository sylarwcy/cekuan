/*****************************************************************************
 * HalconAPI.h
 *****************************************************************************
 *
 * Project:     HALCON/C++
 * Description: Internal communication with HALCON library.
 *              Exposed for use by extension packages only.
 *
 * (c) 2010-2023 by MVTec Software GmbH
 *                  www.mvtec.com
 *
 *****************************************************************************
 *
 *
 *****************************************************************************/


#ifndef HCPP_HALCON_API_H
#define HCPP_HALCON_API_H


#include "hlib/DBNew.h"
#include "hlib/HInstance.h"
#include "HalconCDefs.h"


namespace HalconCpp
{
class LIntExport HalconAPI
{
public:
  // Create operator call
  static Hproc_handle PreCall(int proc_index);

  // Execute operator call
  static void CallProcedure(Hproc_handle proc);

  // Finalize operator call
  static void PostCall(Hproc_handle proc, Herror err = H_MSG_OK);


  // Store iconic input parameters

  static void Store(Hproc_handle proc, int par, const HObjectBase& obj);
  static void Store(Hproc_handle proc, int par, Hkey key);


  // Load iconic output parameters

  static void Load(Hproc_handle proc, int par, HObjectBase* obj);
  static void Load(Hproc_handle proc, int par, Hkey* key);


  // Store input control parameters

  static void Store(Hproc_handle proc, int par, Hlong value);
  static void Store(Hproc_handle proc, int par, double value);
  static void Store(Hproc_handle proc, int par, const char* value);
  static void StoreEnc(Hproc_handle proc, int par, const char* value);
#ifdef _WIN32
  static void Store(Hproc_handle proc, int par, const wchar_t* value);
#endif
  static void Store(Hproc_handle proc, int par, const HString& value);
  static void StoreEnc(Hproc_handle proc, int par, const HString& value);
  static void Store(Hproc_handle proc, int par, const HTuple& tuple);
  static void StoreEnc(Hproc_handle proc, int par, const HTuple& tuple);
  static void Store(Hproc_handle proc, int par, const HHandle& handle);
  static void Store(Hproc_handle proc, int par, void* pointer);

  static HTuple StoreKA(Hproc_handle proc, int par, const HDataBase& data);
  static HTuple StoreKA(Hproc_handle proc, int par,
                        const HHandleBaseArray& tools);
  static HTuple StoreKA(Hproc_handle proc, int par, const HDataArray& data);


  // Load output control parameters

  static void Load(Hproc_handle proc, int par, Hlong* value);
  static void Load(Hproc_handle proc, int par, double* value);
  static void LoadEnc(Hproc_handle proc, int par, HString* value);
  static void Load(Hproc_handle proc, int par, HTuple* tuple);
  static void LoadEnc(Hproc_handle proc, int par, HTuple* tuple);

  static void Load(Hproc_handle proc, int par, HHandle* tool);
  static void Load(Hproc_handle proc, int par, HDataBase* data);
  static void Load(Hproc_handle proc, int par, HHandleBaseArray* tools);
  static void Load(Hproc_handle proc, int par, HDataArray* data);
  static void Load(Hproc_handle proc, int par, void** pointer);

  // Initialize output control parameters
  static void InitOutp(Hproc_handle proc, int par);

  // Handle control parameters
  static Hphandle    CopyHandle(Hphandle key);
  static void        ClearHandle(Hphandle key);
  static Hlong       AcquireExternalOwnership(Hphandle handle);
  static void        ReleaseExternalOwnership(Hphandle handle);
  static Hlong       HandleToHlong(Hphandle handle);
  static bool        HandleIsValid(Hphandle handle);
  static const char* GetHandleSemType(Hphandle handle);
  static bool        TestEqualHandle(Hphandle handle1, Hphandle handle2);
  static bool        IsLegacyHandleMode();


  // Iconic object manipulation
  static Hkey CopyObject(Hkey key);
  static Hkey SelectObject(Hkey key, Hlong index);
  static bool TestEqualObj(Hkey key1, Hkey key2);
  static void ClearObject(Hkey key);
  static void AssertObjectClass(Hkey key, const char* assert_class);
  static void AssertHandleType(Hphandle handle, const char* assert_type);

  // Engine helpers
  static bool NeedsTranscodeToFromUtf8(const Htuple* value);
  static void TranscodeToUtf8FromInterfaceEncoding(Htuple* value);
  static void TranscodeFromUtf8ToInterfaceEncoding(Htuple* value);
};


}


#endif
