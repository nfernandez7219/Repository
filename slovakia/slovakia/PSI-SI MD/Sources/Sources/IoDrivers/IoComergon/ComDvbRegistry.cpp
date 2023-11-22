
#include "tools2.hpp"
#include "BaseRegistry.hpp"
#include "loadRes.hpp"
#include "Card.h"
#include "ComDvb.hpp"

#define		DVBDRIVERKEY		"SYSTEM\\CurrentControlSet\\Services\\md_DVBDriver"
#define		DVBSTARTAUTOMATIC	2
#define		DVBSTARTDISABLED	4

#define	MAX_TEXT	255

static BOOL RegistryOpenDVBDriverKey( HKEY *hKey, REGSAM desiredAccess )
{
	char	KeyName[MAX_TEXT] = DVBDRIVERKEY;
	return regOpenKey( hKey, HKEY_LOCAL_MACHINE, KeyName, desiredAccess );
}

BOOL DVBDriverRegisteredByTheSystem( )
{
	HKEY	hKey;
	if( !RegistryOpenDVBDriverKey( &hKey, ReadOnly ) )
		return FALSE ;

	BOOL  ret=FALSE ;
	DWORD v ;
	if ( regQueryDWORD (hKey,"Start",&v) )
		ret = (v != DVBSTARTDISABLED) ;
	RegCloseKey (hKey);
	return ret ;
}

int	RegistryCreateDVBDriverEntry (PDVBDRIVERINFO pDVB)
{
	char	KeyName[MAX_TEXT] = DVBDRIVERKEY;
	HKEY	hKey;
	DWORD	dwordData;

	if ( !regCreateKey( &hKey, HKEY_LOCAL_MACHINE, KeyName ) )
		return REG_RESULT_CREATE_FAILED;

	if( !regWriteDWORD (hKey,"ErrorControl",0x00000001)  ||
		!regWriteDWORD (hKey,"Type"        ,0x00000001)  ||
		!regWriteDWORD (hKey,"Start"       ,DVBSTARTAUTOMATIC)  ||
		!regWriteDWORD (hKey,"Sender"      ,pDVB->Sender) )
	{
		RegCloseKey (hKey);
		return REG_RESULT_WRITE_FAILED;
	}

	// keep Base/Irq from previous installation (if any)
	if (  regQueryDWORD (hKey,"Base",&dwordData) )
		pDVB->Base = dwordData ;
	else
	if ( !regWriteDWORD (hKey,"Base",pDVB->Base) )
	{
		RegCloseKey (hKey);
		return REG_RESULT_WRITE_FAILED;
	}

	if (  regQueryDWORD (hKey,"Irq",&dwordData) )
		pDVB->Irq = dwordData ;
	else
	if ( !regWriteDWORD (hKey,"Irq",pDVB->Irq) )
	{
		RegCloseKey (hKey);
		return REG_RESULT_WRITE_FAILED;
	}
	
	RegCloseKey (hKey);
	return REG_RESULT_OK;
}


int	RegistryRetrieveDVBDriverEntry (PDVBDRIVERINFO pDVB)
{
	HKEY	hKey;
	DWORD	v;

	if( !RegistryOpenDVBDriverKey( &hKey, ReadOnly ) )
		return REG_RESULT_OPEN_FAILED;

	if ( !regQueryDWORD (hKey,"Base",&v) )
		{
		RegCloseKey (hKey);
		return REG_RESULT_QUERY_FAILED;
		}
	else
		pDVB->Base = v;

	if ( !regQueryDWORD (hKey,"Irq",&v) )
		{
		RegCloseKey (hKey);
		return REG_RESULT_QUERY_FAILED;
		}
	else
		pDVB->Irq = v;

	if ( !regQueryDWORD (hKey,"Sender",&v) )
		{
		RegCloseKey (hKey);
		return REG_RESULT_QUERY_FAILED;
		}
	else
		pDVB->Sender = v;

	RegCloseKey (hKey);
	return REG_RESULT_OK;
}


int	RegistryModifyDVBDriverEntry (PDVBDRIVERINFO pDVB)
{
	HKEY	hKey;
	if( !RegistryOpenDVBDriverKey( &hKey, WriteOnly ) )
		return REG_RESULT_OPEN_FAILED;

	if( !regWriteDWORD (hKey,"Base",pDVB->Base)  ||
		!regWriteDWORD (hKey,"Irq",pDVB->Irq) )
	{
		RegCloseKey (hKey);
		return REG_RESULT_QUERY_FAILED;
	}
	
	RegCloseKey (hKey);
	return REG_RESULT_OK;
}


static BOOL RegistryDisableDVBDriver( )
{
	HKEY	hKey;
	if( !RegistryOpenDVBDriverKey( &hKey, WriteOnly ) )
		return FALSE ;

	BOOL ret = regWriteDWORD( hKey, "Start", DVBSTARTDISABLED ) ;
	RegCloseKey (hKey);
	return TRUE ;
}

BOOL RegistryUninstallDVBDriver( BOOL removeDrvEntries )
{
	BOOL r0 = TRUE;
	if ( removeDrvEntries )
	{
		r0 = regDeleteKey( HKEY_LOCAL_MACHINE, DVBDRIVERKEY );
		if ( !r0 )
		{
			// If driver keys could not be removed, try at least to disable the driver.
			if( RegistryDisableDVBDriver() )
				r0 = TRUE ;
		}
	}
	return r0 ;
}
