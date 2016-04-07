#pragma once
#include <Ntsecapi.h> // DDK  
typedef const UNICODE_STRING* PCUNICODE_STRING;

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA {
    ULONG Flags;                    //Reserved.  
    PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.  
    PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.  
    PVOID DllBase;                  //A pointer to the base address for the DLL in memory.  
    ULONG SizeOfImage;              //The size of the DLL image, in bytes.  
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA {
    ULONG Flags;                    //Reserved.  
    PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.  
    PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.  
    PVOID DllBase;                  //A pointer to the base address for the DLL in memory.  
    ULONG SizeOfImage;              //The size of the DLL image, in bytes.  
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA {
    LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
    LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;

typedef const PLDR_DLL_NOTIFICATION_DATA PCLDR_DLL_NOTIFICATION_DATA;

typedef VOID(NTAPI *PLDR_DLL_NOTIFICATION_FUNCTION)(ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context);
typedef NTSTATUS(NTAPI *pfnLdrRegisterDllNotification)(ULONG Flags, PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction, void* Context, void **Cookie);
typedef NTSTATUS(NTAPI *pfnLdrUnregisterDllNotification)(void *Cookie);

#define LDR_DLL_NOTIFICATION_REASON_LOADED 1   
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 2  
