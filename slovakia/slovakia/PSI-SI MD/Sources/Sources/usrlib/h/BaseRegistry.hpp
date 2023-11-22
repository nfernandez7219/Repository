
#ifndef __BASEREGISTRY_HPP__
#define __BASEREGISTRY_HPP__

#include <windows.h>

#define	MAX_TEXT							255

#define		REG_RESULT_OK					0
#define		REG_RESULT_CREATE_FAILED		1
#define		REG_RESULT_OPEN_FAILED			2
#define		REG_RESULT_WRITE_FAILED			3
#define		REG_RESULT_QUERY_FAILED			4
#define		REG_RESULT_INVALID_VALUE		5
#define		REG_RESULT_MAX					6

//registry accessing rights
#define ReadOnly		(KEY_READ)
#define WriteOnly		(KEY_WRITE)
#define ReadWrite		(KEY_READ|KEY_WRITE)
#define AllAccess		(KEY_ALL_ACCESS)

const char	*RegistryErrorMessage( int code );

extern "C"
{
// Base registry manipulation functions
BOOL regCreateKey( HKEY *hKey, HKEY baseKey, char *keyName );
BOOL regDeleteKey( HKEY baseKey, char *keyName );
BOOL regOpenKey( HKEY *hKey, HKEY baseKey, char *keyName, REGSAM desiredAccess );

BOOL regWriteDWORD (HKEY hKey,char *ValueName,DWORD Value);
BOOL regQueryDWORD (HKEY hKey,char *ValueName,DWORD *Value);

BOOL reqWriteMultiSz (HKEY hKey,char *ValueName,BYTE *Value,DWORD ValueSize);
BOOL reqQueryMultiSz (HKEY hKey,char *ValueName,BYTE *Value,DWORD *ValueSize);

BOOL regWriteBinary (HKEY hKey,char *ValueName,BYTE *Value,DWORD ValueSize);
BOOL regQueryBinary (HKEY hKey,char *ValueName,BYTE *Value,DWORD *ValueSize);

BOOL regWriteString (HKEY hKey,char *ValueName,char *Value);
BOOL regQueryString (HKEY hKey,char *ValueName,char *Value);
}

#endif