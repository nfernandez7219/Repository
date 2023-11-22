# Microsoft Developer Studio Project File - Name="IoDvbAsi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=IoDvbAsi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IoDvbAsi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IoDvbAsi.mak" CFG="IoDvbAsi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IoDvbAsi - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IoDvbAsi - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/dev/dvb/IoDrivers/IoDvbAsi", AJDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IoDvbAsi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IODVBASI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IODVBASI_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41b /d "NDEBUG"
# ADD RSC /l 0x41b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "IoDvbAsi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Bin"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IODVBASI_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MDd /W3 /Gm /Gi /GX /Zi /Od /Gy /I "..\..\Dvblib\h" /I "..\..\usrlib\h" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "__IODVBASIDLL" /D "_WINDLL" /D "_AFXDLL" /YX"tools2.hpp" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41b /d "_DEBUG"
# ADD RSC /l 0x41b /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 version.lib DvbD.lib usrlibD.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\..\bin"

!ENDIF 

# Begin Target

# Name "IoDvbAsi - Win32 Release"
# Name "IoDvbAsi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ComAsi.def
# End Source File
# Begin Source File

SOURCE=.\ComDvbAsi.cpp
# End Source File
# Begin Source File

SOURCE=.\drv_ComAsi.cpp
# End Source File
# Begin Source File

SOURCE=.\DvbAsiSetupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\IoDvbAsi.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ComDvbAsi.hpp
# End Source File
# Begin Source File

SOURCE=.\dvbIoctl.h
# End Source File
# End Group
# Begin Group "Doc"

# PROP Default_Filter ""
# Begin Group "Firmware"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\DVB Master Manual 2.0.doc"
# End Source File
# Begin Source File

SOURCE=.\technote.doc
# End Source File
# End Group
# Begin Source File

SOURCE=.\DvbAsiProgramerReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\IoDvbAsi.lst
# End Source File
# Begin Source File

SOURCE=.\ReadmeDvbAsiRcv.txt
# End Source File
# Begin Source File

SOURCE=.\ReadmeDvbAsiSrv.txt
# End Source File
# End Group
# End Target
# End Project
