
#include "BaseRegistry.hpp"

const char	*RegistryErrorMessage( int code )
{
	switch( code )
	{
		case REG_RESULT_OK:
			return "Everything went Ok";
		case REG_RESULT_CREATE_FAILED:
			return "Failed to create registry entry";
		case REG_RESULT_OPEN_FAILED:
			return "Failed to open registry entry";
		case REG_RESULT_WRITE_FAILED:
			return "Failed to write to registry entry";
		case REG_RESULT_QUERY_FAILED:
			return "Failed to query registry entry";
		case REG_RESULT_INVALID_VALUE:
			return "Invalid value encountered in registry operation" ;
	}
	return "Unspecified error code" ;
}

BOOL regCreateKey( HKEY *hKey, HKEY baseKey, char *keyName )
{
	LONG	result;
	DWORD	disposition;

	result = RegCreateKeyEx( baseKey, keyName, 0, NULL, REG_OPTION_NON_VOLATILE, AllAccess, NULL, hKey, &disposition );
	
	return result == ERROR_SUCCESS;
}

BOOL regDeleteKey( HKEY baseKey, char *keyName )
{
	HKEY hKey;
	if ( !regOpenKey( &hKey, baseKey, keyName, AllAccess ) )
		return FALSE;

	for( int i=0 ; i < 100 ; ++i )
	{
		char subkeyName[200] ;
		int err = RegEnumKey( hKey, i, subkeyName, sizeof(subkeyName) ) ;
		if( err != ERROR_SUCCESS )
			break ;
		if( !regDeleteKey( hKey, subkeyName ) )
			break ;
	}

	RegCloseKey( hKey );
	long err = RegDeleteKey( baseKey, keyName );
	return err == ERROR_SUCCESS ;
}

BOOL regOpenKey( HKEY *hKey, HKEY baseKey, char *subKeyName, REGSAM desiredAccess )
{
	DWORD	reserved = 0;
	long result = RegOpenKeyEx (baseKey, subKeyName, reserved, desiredAccess, hKey );
	return  result == ERROR_SUCCESS ;
}

BOOL regWriteDWORD (HKEY hKey,char *ValueName,DWORD Value)
{
	DWORD	reserved = 0, ValueType = REG_DWORD, ValueSize = 4;
	
	long result = RegSetValueEx (hKey,ValueName,reserved,ValueType,(BYTE *)&Value,ValueSize);
	return (result == ERROR_SUCCESS);
}

BOOL reqWriteMultiSz (HKEY hKey,char *ValueName,BYTE *Value,DWORD ValueSize)
{
	DWORD	reserved = 0, ValueType = REG_MULTI_SZ;
	
	long result = RegSetValueEx (hKey,ValueName,reserved,ValueType,Value,ValueSize);
	return (result == ERROR_SUCCESS);
}

BOOL regWriteBinary (HKEY hKey,char *ValueName,BYTE *Value,DWORD ValueSize)
{
	DWORD	reserved = 0, ValueType = REG_BINARY;
	
	long result = RegSetValueEx (hKey,ValueName,reserved,ValueType,Value,ValueSize);
	return (result == ERROR_SUCCESS);
}

BOOL regWriteString (HKEY hKey,char *ValueName,char *Value)
{
	DWORD	reserved = 0, ValueType = REG_SZ, ValueSize = strlen (Value)+1;
	
	long result = RegSetValueEx (hKey,ValueName,reserved,ValueType,(BYTE *) Value,ValueSize);
	return (result == ERROR_SUCCESS);
}

BOOL regQueryDWORD (HKEY hKey,char *ValueName,DWORD *Value)
{
	DWORD	ValueSize = 4, ValueType = REG_DWORD;

	long result = RegQueryValueEx (hKey,ValueName,NULL,&ValueType,(BYTE *) Value,&ValueSize );
	return (result == ERROR_SUCCESS);
}

BOOL reqQueryMultiSz (HKEY hKey,char *ValueName,BYTE *Value,DWORD *ValueSize)
{
	DWORD	ValueType = REG_MULTI_SZ;

	long result = RegQueryValueEx (hKey,ValueName,NULL,&ValueType,Value,ValueSize );
	return (result == ERROR_SUCCESS);
}

BOOL regQueryBinary (HKEY hKey,char *ValueName,BYTE *Value,DWORD *ValueSize)
{
	DWORD	ValueType = REG_BINARY;

	long result = RegQueryValueEx (hKey,ValueName,NULL,&ValueType,Value,ValueSize );
	return (result == ERROR_SUCCESS);
}

BOOL regQueryString (HKEY hKey,char *ValueName,char *Value)
{
	DWORD	ValueType = REG_SZ, ValueSize = MAX_TEXT-1;

	long result = RegQueryValueEx (hKey,(LPTSTR)ValueName,NULL,&ValueType,(BYTE *)Value,&ValueSize );
	return (result == ERROR_SUCCESS);
}
