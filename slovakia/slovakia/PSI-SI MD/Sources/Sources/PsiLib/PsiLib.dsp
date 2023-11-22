# Microsoft Developer Studio Project File - Name="PsiLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=PsiLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PsiLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PsiLib.mak" CFG="PsiLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PsiLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "PsiLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PsiLib - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "PsiLib - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MDd /W3 /Gm /GX /Zi /Od /I "..\UsrLib\h" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /YX"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\PsiLib.lib"

!ENDIF 

# Begin Target

# Name "PsiLib - Win32 Release"
# Name "PsiLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "UsrLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\ComOut.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Src\AddPrgWizard.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\BroadcastMan.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CountryAvailDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CountryCodes.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\DescriptorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\DescriptorHolders.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\Descriptors.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\MultiLingNameDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\NetworkId.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\NetworkWizard.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ServiceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\TableHolders.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\Tables.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\TableTree.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\ValueList.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\WizardDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Src\AddPrgWizard.h
# End Source File
# Begin Source File

SOURCE=.\Src\BroadcastMan.h
# End Source File
# Begin Source File

SOURCE=.\Src\ComOut.h
# End Source File
# Begin Source File

SOURCE=.\Src\DescriptorDlg.h
# End Source File
# Begin Source File

SOURCE=.\Src\Descriptors.h
# End Source File
# Begin Source File

SOURCE=.\Src\DrvInterface.hpp
# End Source File
# Begin Source File

SOURCE=.\Src\ProgramPidDlg.h
# End Source File
# Begin Source File

SOURCE=.\Src\ServiceDlg.h
# End Source File
# Begin Source File

SOURCE=.\Src\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Src\TableHolders.h
# End Source File
# Begin Source File

SOURCE=.\Src\Tables.h
# End Source File
# Begin Source File

SOURCE=.\Src\TableTree.h
# End Source File
# Begin Source File

SOURCE=.\Src\ValueList.h
# End Source File
# Begin Source File

SOURCE=.\Src\WizardDlg.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ToDo.txt
# End Source File
# End Target
# End Project
